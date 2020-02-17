/* StreamWriter.cpp */

/* Copyright (C) 2011-2020 Michael Lugmair (Lucio Carreras)
 *
 * This file is part of sayonara player
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/* This module is the interface between the parser and the data sender
 */

#include "StreamWriter.h"
#include "StreamDataSender.h"
#include "StreamHttpParser.h"
#include "Components/PlayManager/PlayManager.h"
#include "Components/Engine/EngineHandler.h"
#include "Utils/MetaData/MetaData.h"
#include "Utils/Logger/Logger.h"

#include <QTcpSocket>

struct StreamWriter::Private
{
	Engine::Handler*	engine=nullptr;
	StreamHttpParser*	parser=nullptr;
	StreamDataSender*	sender=nullptr;
	QTcpSocket*			socket=nullptr;

	QString				streamTitle;
	QString				ip;

	StreamWriter::Type	type;

	bool				dismissed; // after that, only trash will be sent
	bool				sendData; // after that, no data at all will be sent

	Private(QTcpSocket* socket, const QString& ip) :
		socket(socket),
		ip(ip),
		type(StreamWriter::Type::Undefined),
		dismissed(false),
		sendData(false)
	{
		engine = Engine::Handler::instance();
		parser = new StreamHttpParser();
		sender = new StreamDataSender(socket);
	}
};

// socket is the client socket
StreamWriter::StreamWriter(QTcpSocket* socket, const QString& ip, const MetaData& md) :
	Engine::RawSoundReceiverInterface()
{
	m = Pimpl::make<Private>(socket, ip);
	m->streamTitle = md.artist() + " - " + md.title();

	if(m->socket->bytesAvailable()){
		dataAvailble();
	}

	connect(socket, &QTcpSocket::disconnected, this, &StreamWriter::socketDisconnected);
	connect(socket, &QTcpSocket::readyRead, this, &StreamWriter::dataAvailble);
	connect(Engine::Handler::instance(), &Engine::Handler::destroyed, this, [=](){
		m->engine = nullptr;
	});

	connect(PlayManager::instance(), &PlayManager::sigCurrentTrackChanged, this, [=](const MetaData&){
		this->clearSocket();
	});

	connect(PlayManager::instance(), &PlayManager::sigSeekedRelative, this, [=](double){
		this->clearSocket();
	});

	connect(PlayManager::instance(), &PlayManager::sigSeekedAbsoluteMs, this, [=](MilliSeconds){
		this->clearSocket();
	});

	connect(PlayManager::instance(), &PlayManager::sigSeekedRelativeMs, this, [=](MilliSeconds){
		this->clearSocket();
	});

	m->engine->registerRawSoundReceiver(this);
}

StreamWriter::~StreamWriter()
{
	if(m->engine){
		m->engine->unregisterRawSoundReceiver(this);
	}

	if(m->parser){
		delete m->parser; m->parser = nullptr;
	}

	if(m->sender){
		delete m->sender; m->sender = nullptr;
	}
}

QString StreamWriter::ip() const
{
	return m->ip;
}


StreamHttpParser::HttpAnswer StreamWriter::parseMessage()
{
	StreamHttpParser::HttpAnswer status;
	status = m->parser->parse(m->socket->readAll());

	spLog(Log::Debug, this) << "Parse message " << StreamHttpParser::answerString(status);

	return status;
}

void StreamWriter::writeAudioData(const QByteArray& data)
{
	if(!m->sendData) {
		return;
	}

	if(m->dismissed){
		m->sender->sendTrash();
		return;
	}

	if(m->parser->isIcyStream()){
		m->sender->sendIcyData(data, m->streamTitle);
	}

	else{
		m->sender->sendData(data);
	}
}

bool StreamWriter::sendPlaylist()
{
	return m->sender->sendPlaylist(m->parser->host(), m->socket->localPort());
}

bool StreamWriter::sendFavicon()
{
	return m->sender->sendFavicon();
}

bool StreamWriter::sendMetadata()
{
	return m->sender->sendMetadata(m->streamTitle);
}

bool StreamWriter::sendBackground()
{
	return m->sender->sendBackground();
}

bool StreamWriter::sendHtml5()
{
	return m->sender->sendHtml5(m->streamTitle);
}

bool StreamWriter::sendHeader(bool reject)
{
	return m->sender->sendHeader(reject, m->parser->isIcyStream());
}

void StreamWriter::changeTrack(const MetaData& md)
{
	m->streamTitle =  md.artist() + " - " + md.title();
}

void StreamWriter::dismiss()
{
	if(m->engine){
		m->engine->unregisterRawSoundReceiver(this);
	}

	m->dismissed = true;
}


void StreamWriter::disconnect()
{
	dismiss();

	m->socket->disconnectFromHost();
}


void StreamWriter::socketDisconnected()
{
	if(m->engine){
		m->engine->unregisterRawSoundReceiver(this);
	}

	emit sigDisconnected(this);
}


void StreamWriter::dataAvailble()
{
	StreamHttpParser::HttpAnswer answer = parseMessage();

	bool success;
	bool close_connection = true;
	m->type = StreamWriter::Type::Standard;

	switch(answer){
		case StreamHttpParser::HttpAnswer::Fail:
		case StreamHttpParser::HttpAnswer::Reject:

			m->type = StreamWriter::Type::Invalid;
			//sp_log(Log::Debug, this) << "Rejected: " << _parser->get_user_agent() << ": " << get_ip();
			sendHeader(true);
			break;

		case StreamHttpParser::HttpAnswer::Ignore:
			//sp_log(Log::Debug, this) << "ignore...";
			break;

		case StreamHttpParser::HttpAnswer::Playlist:
			//sp_log(Log::Debug, this) << "Asked for playlist";
			sendPlaylist();
			break;

		case StreamHttpParser::HttpAnswer::HTML5:
			//sp_log(Log::Debug, this) << "Asked for html5";
			sendHtml5();

			break;

		case StreamHttpParser::HttpAnswer::BG:
			//sp_log(Log::Debug, this) << "Asked for background";
			sendBackground();
			break;

		case StreamHttpParser::HttpAnswer::Favicon:
			//sp_log(Log::Debug, this) << "Asked for favicon";
			sendFavicon();
			break;

		case StreamHttpParser::HttpAnswer::MetaData:
			//sp_log(Log::Debug, this) << "Asked for metadata";
			sendMetadata();
			break;

		default:
			m->type = StreamWriter::Type::Streaming;
			close_connection = false;
			//sp_log(Log::Debug, this) << "Accepted: " << _parser->get_user_agent() << ": " << ip;
			success = sendHeader(false);

			if(success){
				m->sendData = true;
				Engine::Handler::instance()->registerRawSoundReceiver(this);
			}

			emit sigNewConnection(ip());
			break;
	}

	if(close_connection){
		m->socket->close();
	}
}

void StreamWriter::clearSocket()
{
	auto bytes = m->socket->bytesToWrite();
	spLog(Log::Debug, this) << "There are still " << bytes << " bytes";
	m->socket->flush();
}

