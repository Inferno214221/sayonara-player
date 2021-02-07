/* StreamServer.h */

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

#ifndef STREAM_SERVER_H
#define STREAM_SERVER_H

#include "StreamWriter.h"
#include "Utils/Pimpl.h"

// also needed for AcceptError
#include <QTcpSocket>

class PlayManager;
class RawAudioDataProvider;

/**
 * @brief The StreamServer class. This class is listening for new connections and holds and administrates current connections.
 * @ingroup Broadcasting
 */
class StreamServer :
	public QObject
{
	Q_OBJECT
	PIMPL(StreamServer)

	signals:
		void sigNewConnection(const QString& ip);
		void sigConnectionClosed(const QString& ip);
		void sigListening(bool);

	public:
		explicit StreamServer(PlayManager* playManager, RawAudioDataProvider* audioDataProvider, QObject* parent = nullptr);
		~StreamServer();

		QStringList connectedClients() const;

	public slots:
		void dismiss(int idx);

		void disconnect(StreamWriterPtr sw);
		void disconnectAll();

		bool listen();
		void close();
		void restart();

	private slots:
		void acceptClient(QTcpSocket* socket, const QString& ip);
		void rejectClient(QTcpSocket* socket, const QString& ip);

		void serverDestroyed();

		void newClientRequest();
		void newClientRequestError(QAbstractSocket::SocketError socketError);

		void disconnected(StreamWriter* sw);
		void newConnection(const QString& ip);

		void activeChanged();
		void portChanged();
};

#endif
