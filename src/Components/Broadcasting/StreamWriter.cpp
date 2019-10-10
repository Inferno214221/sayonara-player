/* StreamWriter.cpp */

/* Copyright (C) 2011-2019  Lucio Carreras
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

	QString				stream_title;
	QString				ip;

	StreamWriter::Type	type;

	bool				dismissed; // after that, only trash will be sent
	bool				send_data; // after that, no data at all will be sent

	Private(QTcpSocket* socket, const QString& ip) :
		socket(socket),
		ip(ip),
		type(StreamWriter::Type::Undefined),
		dismissed(false),
		send_data(false)
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
	m->stream_title = md.artist() + " - " + md.title();

	if(m->socket->bytesAvailable()){
		data_available();
	}

	connect(socket, &QTcpSocket::disconnected, this, &StreamWriter::socket_disconnected);
	connect(socket, &QTcpSocket::readyRead, this, &StreamWriter::data_available);
	connect(Engine::Handler::instance(), &Engine::Handler::destroyed, this, [=](){
		m->engine = nullptr;
	});

	m->engine->register_raw_sound_receiver(this);
}

StreamWriter::~StreamWriter()
{
	if(m->engine){
		m->engine->unregister_raw_sound_receiver(this);
	}

	if(m->parser){
		delete m->parser; m->parser = nullptr;
	}

	if(m->sender){
		delete m->sender; m->sender = nullptr;
	}
}

QString StreamWriter::get_ip() const
{
	return m->ip;
}


StreamHttpParser::HttpAnswer StreamWriter::parse_message()
{
	StreamHttpParser::HttpAnswer status;
	status = m->parser->parse(m->socket->readAll());

	sp_log(Log::Debug, this) << "Parse message " << StreamHttpParser::answer_string(status);

	return status;
}

void StreamWriter::new_audio_data(const QByteArray& data)
{
	if(!m->send_data) {
		return;
	}

	if(m->dismissed){
		m->sender->send_trash();
		return;
	}

	if(m->parser->is_icy()){
		m->sender->send_icy_data(data, m->stream_title);
	}

	else{
		m->sender->send_data(data);
	}
}

bool StreamWriter::send_playlist()
{
	return m->sender->send_playlist(m->parser->get_host(), m->socket->localPort());
}

bool StreamWriter::send_favicon()
{
	return m->sender->send_favicon();
}

bool StreamWriter::send_metadata()
{
	return m->sender->send_metadata(m->stream_title);
}

bool StreamWriter::send_bg()
{
	return m->sender->send_bg();
}

bool StreamWriter::send_html5()
{
	return m->sender->send_html5(m->stream_title);
}

bool StreamWriter::send_header(bool reject)
{
	return m->sender->send_header(reject, m->parser->is_icy());
}

void StreamWriter::change_track(const MetaData& md)
{
	m->stream_title =  md.artist() + " - " + md.title();
}

void StreamWriter::dismiss()
{
	if(m->engine){
		m->engine->unregister_raw_sound_receiver(this);
	}

	m->dismissed = true;
}


void StreamWriter::disconnect()
{
	dismiss();

	m->socket->disconnectFromHost();
}


void StreamWriter::socket_disconnected()
{
	if(m->engine){
		m->engine->unregister_raw_sound_receiver(this);
	}

	emit sig_disconnected(this);
}


void StreamWriter::data_available()
{
	StreamHttpParser::HttpAnswer answer = parse_message();
	QString ip = get_ip();
	bool success;
	bool close_connection = true;
	m->type = StreamWriter::Type::Standard;

	switch(answer){
		case StreamHttpParser::HttpAnswer::Fail:
		case StreamHttpParser::HttpAnswer::Reject:

			m->type = StreamWriter::Type::Invalid;
			//sp_log(Log::Debug, this) << "Rejected: " << _parser->get_user_agent() << ": " << get_ip();
			send_header(true);
			break;

		case StreamHttpParser::HttpAnswer::Ignore:
			//sp_log(Log::Debug, this) << "ignore...";
			break;

		case StreamHttpParser::HttpAnswer::Playlist:
			//sp_log(Log::Debug, this) << "Asked for playlist";
			send_playlist();
			break;

		case StreamHttpParser::HttpAnswer::HTML5:
			//sp_log(Log::Debug, this) << "Asked for html5";
			send_html5();

			break;

		case StreamHttpParser::HttpAnswer::BG:
			//sp_log(Log::Debug, this) << "Asked for background";
			send_bg();
			break;

		case StreamHttpParser::HttpAnswer::Favicon:
			//sp_log(Log::Debug, this) << "Asked for favicon";
			send_favicon();
			break;

		case StreamHttpParser::HttpAnswer::MetaData:
			//sp_log(Log::Debug, this) << "Asked for metadata";
			send_metadata();
			break;

		default:
			m->type = StreamWriter::Type::Streaming;
			close_connection = false;
			//sp_log(Log::Debug, this) << "Accepted: " << _parser->get_user_agent() << ": " << ip;
			success = send_header(false);

			if(success){
				m->send_data = true;
				Engine::Handler::instance()->register_raw_sound_receiver(this);
			}

			emit sig_new_connection(ip);
			break;
	}

	if(close_connection){
		m->socket->close();
	}
}

