/* StreamDataSender.h */

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

#ifndef STREAMDATASENDER_H
#define STREAMDATASENDER_H

#include "Utils/Pimpl.h"

class QTcpSocket;

/**
 * @brief The StreamDataSender class. This class is used for sending the raw bytes.
 * @ingroup Broadcasting
 */
class StreamDataSender
{
	PIMPL(StreamDataSender)

	private:
		bool sendIcyMetadata(const QString& streamTitle);

	public:
		explicit StreamDataSender(QTcpSocket* socket);
		~StreamDataSender();

		bool sendTrash();
		bool sendData(const QByteArray& data);
		bool sendIcyData(const QByteArray& data, const QString& streamTitle);
		bool sendHeader(bool reject, bool icy);
		bool sendHtml5(const QString& streamTitle);
		bool sendBackground();
		bool sendMetadata(const QString& streamTitle);
		bool sendPlaylist(const QString& host, int port);
		bool sendFavicon();

		void flush();

};

#endif // STREAMDATASENDER_H
