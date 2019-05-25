/* StreamServer.cpp */

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

#include "StreamServer.h"

#include "Utils/Utils.h"
#include "Utils/MetaData/MetaData.h"
#include "Utils/WebAccess/AsyncWebAccess.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Settings/SettingNotifier.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Message/Message.h"

#include "Components/Engine/EngineHandler.h"
#include "Components/PlayManager/PlayManager.h"

#include <QHostAddress>
#include <QTcpServer>
#include <QTcpSocket>
#include <QNetworkProxy>

struct StreamServer::Private
{
	QTcpServer*							server=nullptr;		// the server

	MetaData							cur_track;				// cur played track

	QList<QPair<QTcpSocket*, QString>>	pending;				// pending requests queue
	bool								asking;				// set if currently any requests are being processed

	QList<StreamWriter*>				lst_sw;				// all open streams
	QStringList							allowed_ips;			// IPs without prompt
	QStringList							dismissed_ips;		// dismissed IPs

	int									current_port;

	Private()
	{
		asking = false;
		current_port = GetSetting(Set::Broadcast_Port);
	}
};


StreamServer::StreamServer(QObject* parent) :
	QObject(parent)
{
	m = Pimpl::make<StreamServer::Private>();

	PlayManagerPtr play_manager = PlayManager::instance();
	Engine::Handler* engine = Engine::Handler::instance();

	connect(play_manager, &PlayManager::sig_track_changed, this, &StreamServer::track_changed);
	connect(engine, &Engine::Handler::destroyed, this, &StreamServer::close);

	ListenSetting(Set::Broadcast_Active, StreamServer::active_changed);
	ListenSetting(SetNoDB::MP3enc_found, StreamServer::active_changed);
	ListenSetting(Set::Broadcast_Port, StreamServer::port_changed);
}

StreamServer::~StreamServer()
{
	// don't call close here because of the signal it's shooting
	disconnect_all();

	if(m->server)
	{
		m->server->close();
		m->server->deleteLater();
		m->server = nullptr;

		sp_log(Log::Info, this) << "Server closed.";
	}


	Util::sleep_ms(500);
}

bool StreamServer::listen()
{
	int port = GetSetting(Set::Broadcast_Port);
	bool already_there = (m->server != nullptr);

	bool mp3_available = GetSetting(SetNoDB::MP3enc_found);
	bool active = GetSetting(Set::Broadcast_Active);

	if(!mp3_available || !active)
	{
		close();
		return false;
	}

	if(!already_there)
	{
		m->server = new QTcpServer();
		m->server->setProxy(QNetworkProxy());

		connect(m->server, &QTcpServer::newConnection, this, &StreamServer::new_client_request);
		connect(m->server, &QTcpServer::destroyed, this, &StreamServer::server_destroyed);
	}

	bool success = m->server->isListening();
	if(!success)
	{
		success = m->server->listen(QHostAddress::AnyIPv4, port);

		if(!success)
		{
			sp_log(Log::Warning, this) << "Cannot listen on port " << port;
			sp_log(Log::Warning, this) << m->server->errorString();

			close();

			return false;
		}

		m->server->setMaxPendingConnections(10);
	}

	if(!already_there){
		sp_log(Log::Info, this) << "Listening on port " << port;
	}

	emit sig_listening(true);

	return true;
}

void StreamServer::close()
{
	disconnect_all();

	if(m->server)
	{
		m->server->close();
		m->server->deleteLater();
		m->server = nullptr;

		sp_log(Log::Info, this) << "Server closed.";
	}

	emit sig_listening(false);
}


// this happens when the user tries to look for the codec again
void StreamServer::restart()
{
	close();
	bool success = listen();
	emit sig_listening(success);
}


void StreamServer::server_destroyed()
{
	sp_log(Log::Info, this) << "Server destroyed.";
}


// either show a popup dialog or accept directly
void StreamServer::new_client_request()
{
	QTcpSocket* pending_socket = m->server->nextPendingConnection();
	if(!pending_socket) {
		return;
	}

	QString pending_ip = pending_socket->peerAddress().toString();

	if(m->dismissed_ips.contains(pending_ip))
	{
		reject_client(pending_socket, pending_ip);
		m->dismissed_ips.removeOne(pending_ip);
		return;
	}

	m->pending << QPair<QTcpSocket*, QString>(pending_socket, pending_ip);

	if(m->asking){
		return;
	}

	m->asking = true;

	do
	{
		pending_socket = m->pending[0].first;
		pending_ip = m->pending[0].second;

		if( GetSetting(Set::Broadcast_Prompt) )
		{
			if(!m->allowed_ips.contains(pending_ip))
			{
				QString question = tr("%1 wants to listen to your music.").arg(pending_ip).append("\nOk?");

				Message::Answer answer = Message::question_yn(question, "Stream Server");
				if(answer==Message::Answer::Yes)
				{
					accept_client(pending_socket, pending_ip);
				}
				else
				{
					reject_client(pending_socket, pending_ip);
				}
			}

			else{
				accept_client(pending_socket, pending_ip);
			}
		}

		else{
			accept_client(pending_socket, pending_ip);
		}

		m->pending.pop_front();

	} while(m->pending.size() > 0);

	m->asking = false;
}

// every kind of request will land here or in reject client.
// so one client will be accepted multiple times until he will be able
// to listen to music
void StreamServer::accept_client(QTcpSocket* socket, const QString& ip)
{
	if(!m->allowed_ips.contains(ip)){
		m->allowed_ips << ip;
	}

	sp_log(Log::Info, this) << "New client request from " << ip << " (" << m->lst_sw.size() << ")";

	StreamWriter* sw = new StreamWriter(socket, ip, m->cur_track);

	connect(sw, &StreamWriter::sig_disconnected, this, &StreamServer::disconnected);
	connect(sw, &StreamWriter::sig_new_connection, this, &StreamServer::new_connection);

	m->lst_sw << sw;

	emit sig_new_connection(ip);
}

void StreamServer::reject_client(QTcpSocket* socket, const QString& ip)
{
	Q_UNUSED(socket);
	Q_UNUSED(ip);
}


// this finally is the new connection when asking for sound
void StreamServer::new_connection(const QString& ip)
{
	Q_UNUSED(ip)
}


void StreamServer::track_changed(const MetaData& md)
{
	m->cur_track = md;
	for(StreamWriter* sw : ::Util::AsConst(m->lst_sw))
	{
		sw->change_track(md);
	}
}

// when user forbids further streaming
void StreamServer::dismiss(int idx)
{
	if( idx >= m->lst_sw.size() ) {
		return;
	}

	StreamWriter* sw = m->lst_sw[idx];
	m->dismissed_ips << sw->get_ip();
	m->allowed_ips.removeOne(sw->get_ip());

	sw->dismiss();
}

// real socket disconnect (if no further sending is possible)
void StreamServer::disconnect(StreamWriterPtr sw)
{
	sw->disconnect();
}

void StreamServer::disconnect_all()
{
	for(StreamWriter* sw : ::Util::AsConst(m->lst_sw))
	{
		QObject::disconnect(sw, &StreamWriter::sig_disconnected, this, &StreamServer::disconnected);
		QObject::disconnect(sw, &StreamWriter::sig_new_connection, this, &StreamServer::new_connection);

		sw->disconnect();
		sw->deleteLater();
	}

	m->lst_sw.clear();
}

// the client disconnected itself
void StreamServer::disconnected(StreamWriter* sw)
{
	if(!sw) {
		return;
	}

	QString ip = sw->get_ip();
	emit sig_connection_closed(ip);

	// remove the item, garbage collector deletes that item
	for(auto it=m->lst_sw.begin(); it != m->lst_sw.end(); it++)
	{
		if(sw == *it)
		{
			m->lst_sw.erase(it);
			break;
		}
	}
}


void StreamServer::active_changed()
{
	if( GetSetting(Set::Broadcast_Active) &&
		GetSetting(SetNoDB::MP3enc_found))
	{
		listen();
	}

	else{
		close();
	}
}

void StreamServer::port_changed()
{
	int port = GetSetting(Set::Broadcast_Port);

	if(port != m->current_port){
		restart();
	}

	m->current_port = port;
}
