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

#include "Interfaces/PlayManager.h"
#include "Interfaces/AudioDataProvider.h"

#include "Utils/MetaData/MetaData.h"
#include "Utils/Logger/Logger.h"

#include <QTcpSocket>

namespace
{
	QString getCurrentTitle(PlayManager* playManager)
	{
		return QString("%1 - %2")
			.arg(playManager->currentTrack().albumArtist())
			.arg(playManager->currentTrack().title());
	}
}

struct StreamWriter::Private
{
	PlayManager* playManager;
	QTcpSocket* socket;
	RawAudioDataProvider* audioDataProvider;
	StreamHttpParser* parser;
	StreamDataSender* sender;

	QString ip;
	StreamWriter::Type type;

	bool dismissed; // after that, only trash will be sent
	bool sendData; // after that, no data at all will be sent

	Private(PlayManager* playManager, RawAudioDataProvider* audioDataProvider, QTcpSocket* socket, const QString& ip) :
		playManager(playManager),
		socket(socket),
		audioDataProvider(audioDataProvider),
		parser{new StreamHttpParser()},
		sender{new StreamDataSender(socket)},
		ip(ip),
		type(StreamWriter::Type::Undefined),
		dismissed(false),
		sendData(false)
	{}
};

// socket is the client socket
StreamWriter::StreamWriter(PlayManager* playManager, RawAudioDataProvider* audioDataProvider, QTcpSocket* socket, const QString& ip) :
	Engine::RawAudioDataReceiver()
{
	m = Pimpl::make<Private>(playManager, audioDataProvider, socket, ip);

	if(m->socket->bytesAvailable())
	{
		dataAvailble();
	}

	connect(socket, &QTcpSocket::disconnected, this, &StreamWriter::socketDisconnected);
	connect(socket, &QTcpSocket::readyRead, this, &StreamWriter::dataAvailble);

	connect(m->playManager, &PlayManager::sigCurrentTrackChanged, this, [=](const MetaData&) {
		this->clearSocket();
	});

	connect(m->playManager, &PlayManager::sigSeekedRelative, this, [=](double) {
		this->clearSocket();
	});

	connect(m->playManager, &PlayManager::sigSeekedAbsoluteMs, this, [=](MilliSeconds) {
		this->clearSocket();
	});

	connect(m->playManager, &PlayManager::sigSeekedRelativeMs, this, [=](MilliSeconds) {
		this->clearSocket();
	});

	m->audioDataProvider->registerAudioDataReceiver(this);
}

StreamWriter::~StreamWriter()
{
	m->audioDataProvider->unregisterAudioDataReceiver(this);

	if(m->parser)
	{
		delete m->parser;
		m->parser = nullptr;
	}

	if(m->sender)
	{
		delete m->sender;
		m->sender = nullptr;
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
	if(!m->sendData)
	{
		return;
	}

	if(m->dismissed)
	{
		m->sender->sendTrash();
		return;
	}

	if(m->parser->isIcyStream())
	{
		m->sender->sendIcyData(data, getCurrentTitle(m->playManager));
	} 

	else
	{
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
	return m->sender->sendMetadata(getCurrentTitle(m->playManager));
}

bool StreamWriter::sendBackground()
{
	return m->sender->sendBackground();
}

bool StreamWriter::sendHtml5()
{
	return m->sender->sendHtml5(getCurrentTitle(m->playManager));
}

bool StreamWriter::sendHeader(bool reject)
{
	return m->sender->sendHeader(reject, m->parser->isIcyStream());
}

void StreamWriter::dismiss()
{
	if(m->audioDataProvider)
	{
		m->audioDataProvider->unregisterAudioDataReceiver(this);
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
	if(m->audioDataProvider)
	{
		m->audioDataProvider->unregisterAudioDataReceiver(this);
	}

	emit sigDisconnected(this);
}

void StreamWriter::dataAvailble()
{
	StreamHttpParser::HttpAnswer answer = parseMessage();

	bool success;
	bool close_connection = true;
	m->type = StreamWriter::Type::Standard;

	switch(answer)
	{
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

			if(success)
			{
				m->sendData = true;
				m->audioDataProvider->registerAudioDataReceiver(this);
			}

			emit sigNewConnection(ip());
			break;
	}

	if(close_connection)
	{
		m->socket->close();
	}
}

void StreamWriter::clearSocket()
{
	auto bytes = m->socket->bytesToWrite();
	spLog(Log::Debug, this) << "There are still " << bytes << " bytes";
	m->socket->flush();
}

