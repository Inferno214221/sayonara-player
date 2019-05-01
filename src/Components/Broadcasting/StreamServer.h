/* StreamServer.h */

/* Copyright (C) 2011-2017  Lucio Carreras
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

class QTcpSocket;
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
		void sig_new_connection(const QString& ip);
		void sig_connection_closed(const QString& ip);
		void sig_listening(bool);

	public:
		explicit StreamServer(QObject* parent=nullptr);
		~StreamServer();

		void active_changed();
		void port_changed();

	public slots:
		void dismiss(int idx);

		void disconnect(StreamWriterPtr sw);
		void disconnect_all();

		bool listen();
		void close();
		void restart();

	private slots:
		void accept_client(QTcpSocket* socket, const QString& ip);
		void reject_client(QTcpSocket* socket, const QString& ip);

		void track_changed(const MetaData&);
		void server_destroyed();

		void new_client_request();
		void disconnected(StreamWriter* sw);
		void new_connection(const QString& ip);
};

#endif
