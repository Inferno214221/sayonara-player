/* StreamServer.cpp */

/* Copyright (C) 2011-2024 Michael Lugmair (Lucio Carreras)
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

#include "Components/PlayManager/PlayManager.h"
#include "Interfaces/AudioDataProvider.h"
#include "Utils/Algorithm.h"
#include "Utils/Language/Language.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Message/Message.h"
#include "Utils/MetaData/MetaData.h"
#include "Utils/Settings/SettingNotifier.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Utils.h"

#include <QHostAddress>
#include <QTcpServer>
#include <QTcpSocket>
#include <QNetworkProxy>

namespace Algorithm = Util::Algorithm;

struct StreamServer::Private
{
	PlayManager* playManager;
	RawAudioDataProvider* audioDataProvider;
	QTcpServer* server = nullptr;        // the server

	QList<QPair<QTcpSocket*, QString>> pending;                // pending requests queue

	QList<StreamWriter*> clients;                // all open streams
	QStringList allowedIpAddresses;            // IPs without prompt
	QStringList dismissedIpAddresses;        // dismissed IPs

	int currentPort;
	bool asking;                // set if currently any requests are being processed

	Private(PlayManager* playManager, RawAudioDataProvider* audioDataProvider) :
		playManager(playManager),
		audioDataProvider(audioDataProvider),
		currentPort(GetSetting(Set::Broadcast_Port)),
		asking(false) {}
};

StreamServer::StreamServer(PlayManager* playManager, RawAudioDataProvider* audioDataProvider, QObject* parent) :
	QObject(parent)
{
	m = Pimpl::make<StreamServer::Private>(playManager, audioDataProvider);

	ListenSetting(Set::Broadcast_Active, StreamServer::activeChanged);
	ListenSetting(SetNoDB::MP3enc_found, StreamServer::activeChanged);
	ListenSetting(Set::Broadcast_Port, StreamServer::portChanged);
}

StreamServer::~StreamServer()
{
	// don't call close here because of the signal it's shooting
	disconnectAll();

	if(m->server)
	{
		m->server->close();
		m->server->deleteLater();
		m->server = nullptr;

		spLog(Log::Info, this) << "Server closed.";
	}

	Util::sleepMs(500);
}

QStringList StreamServer::connectedClients() const
{
	QStringList clients;
	Util::Algorithm::transform(m->clients, clients, [](StreamWriter* client) {
		return client->ip();
	});

	return clients;
}

bool StreamServer::listen()
{
	bool mp3Available = GetSetting(SetNoDB::MP3enc_found);
	bool active = GetSetting(Set::Broadcast_Active);

	if(!mp3Available || !active)
	{
		close();
		return false;
	}

	bool alreadyThere = (m->server != nullptr);
	if(!alreadyThere)
	{
		m->server = new QTcpServer();
		m->server->setProxy(QNetworkProxy());

		connect(m->server, &QTcpServer::newConnection, this, &StreamServer::newClientRequest);
		connect(m->server, &QTcpServer::acceptError, this, &StreamServer::newClientRequestError);
		connect(m->server, &QTcpServer::destroyed, this, &StreamServer::serverDestroyed);
	}

	int port = GetSetting(Set::Broadcast_Port);
	bool success = m->server->isListening();
	if(!success)
	{
		success = m->server->listen(QHostAddress::AnyIPv4, quint16(port));
		if(!success)
		{
			success = m->server->listen(QHostAddress::LocalHost, quint16(port));
		}

		if(!success)
		{
			spLog(Log::Warning, this) << "Cannot listen on port " << port;
			spLog(Log::Warning, this) << m->server->errorString();

			close();

			return false;
		}

		m->server->setMaxPendingConnections(10);
	}

	if(!alreadyThere)
	{
		spLog(Log::Info, this) << "Listening on port " << port;
	}

	emit sigListening(true);

	return true;
}

void StreamServer::close()
{
	disconnectAll();

	if(m->server)
	{
		m->server->close();
		m->server->deleteLater();
		m->server = nullptr;

		spLog(Log::Info, this) << "Server closed.";
	}

	emit sigListening(false);
}

// this happens when the user tries to look for the codec again
void StreamServer::restart()
{
	close();
	bool success = listen();
	emit sigListening(success);
}

void StreamServer::serverDestroyed()
{
	spLog(Log::Info, this) << "Server destroyed.";
}

// either show a popup dialog or accept directly
void StreamServer::newClientRequest()
{
	QTcpSocket* pendingSocket = m->server->nextPendingConnection();
	if(!pendingSocket)
	{
		return;
	}

	QString pendingIp = pendingSocket->peerAddress().toString();
	if(m->dismissedIpAddresses.contains(pendingIp))
	{
		rejectClient(pendingSocket, pendingIp);
		m->dismissedIpAddresses.removeOne(pendingIp);
		return;
	}

	m->pending << QPair<QTcpSocket*, QString>(pendingSocket, pendingIp);

	if(m->asking)
	{
		return;
	}

	m->asking = true;

	do
	{
		pendingSocket = m->pending[0].first;
		pendingIp = m->pending[0].second;

		if(GetSetting(Set::Broadcast_Prompt))
		{
			if(!m->allowedIpAddresses.contains(pendingIp))
			{
				QString question = tr("%1 wants to listen to your music.")
					.arg(pendingIp);

				question += "\n" + Lang::get(Lang::OK) + "?";

				Message::Answer answer = Message::question_yn(question, "Stream Server");
				if(answer == Message::Answer::Yes)
				{
					acceptClient(pendingSocket, pendingIp);
				}
				else
				{
					rejectClient(pendingSocket, pendingIp);
				}
			}
			else
			{
				acceptClient(pendingSocket, pendingIp);
			}
		}
		else
		{
			acceptClient(pendingSocket, pendingIp);
		}

		m->pending.pop_front();

	} while(!m->pending.empty());

	m->asking = false;
}

void StreamServer::newClientRequestError(QAbstractSocket::SocketError socket_error)
{
	auto* server = static_cast<QTcpServer*>(sender());
	spLog(Log::Error, this) << "Error connecting: " << server->errorString() << ": " << int(socket_error);
}

// every kind of request will land here or in reject client.
// so one client will be accepted multiple times until he will be able
// to listen to music
void StreamServer::acceptClient(QTcpSocket* socket, const QString& ip)
{
	if(!m->allowedIpAddresses.contains(ip))
	{
		m->allowedIpAddresses << ip;
	}

	spLog(Log::Info, this) << "New client request from " << ip << " (" << m->clients.size() << ")";

	auto* sw = new StreamWriter(m->playManager, m->audioDataProvider, socket, ip);
	connect(sw, &StreamWriter::sigDisconnected, this, &StreamServer::disconnected);
	connect(sw, &StreamWriter::sigNewConnection, this, &StreamServer::newConnection);

	m->clients << sw;

	emit sigNewConnection(ip);
}

void StreamServer::rejectClient(QTcpSocket* socket, const QString& ip)
{
	Q_UNUSED(socket);
	Q_UNUSED(ip);
}

// this finally is the new connection when asking for sound
void StreamServer::newConnection(const QString& ip)
{
	Q_UNUSED(ip)
}

// when user forbids further streaming
void StreamServer::dismiss(int idx)
{
	if(idx >= m->clients.size())
	{
		return;
	}

	StreamWriter* sw = m->clients[idx];
	m->dismissedIpAddresses << sw->ip();
	m->allowedIpAddresses.removeOne(sw->ip());

	sw->dismiss();
}

// real socket disconnect (if no further sending is possible)
void StreamServer::disconnect(StreamWriterPtr sw)
{
	sw->disconnect();
}

void StreamServer::disconnectAll()
{
	for(StreamWriter* sw: Algorithm::AsConst(m->clients))
	{
		QObject::disconnect(sw, &StreamWriter::sigDisconnected, this, &StreamServer::disconnected);
		QObject::disconnect(sw, &StreamWriter::sigNewConnection, this, &StreamServer::newConnection);

		sw->disconnect();
		sw->deleteLater();
	}

	m->clients.clear();
}

// the client disconnected itself
void StreamServer::disconnected(StreamWriter* sw)
{
	if(!sw)
	{
		return;
	}

	QString ip = sw->ip();
	emit sigConnectionClosed(ip);

	// remove the item, garbage collector deletes that item
	for(auto it = m->clients.begin(); it != m->clients.end(); it++)
	{
		if(sw == *it)
		{
			m->clients.erase(it);
			break;
		}
	}
}

void StreamServer::activeChanged()
{
	if(GetSetting(Set::Broadcast_Active) &&
	   GetSetting(SetNoDB::MP3enc_found))
	{
		listen();
	}
	else
	{
		close();
	}
}

void StreamServer::portChanged()
{
	int port = GetSetting(Set::Broadcast_Port);

	if(port != m->currentPort)
	{
		restart();
	}

	m->currentPort = port;
}
