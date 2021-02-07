/* StreamWriter.h */

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

#ifndef STREAM_WRITER_H
#define STREAM_WRITER_H

#include "StreamHttpParser.h"
#include "Interfaces/Engine/AudioDataReceiver.h"
#include "Utils/Pimpl.h"

#include <QObject>

class MetaData;
class PlayManager;
class QTcpSocket;
class RawAudioDataProvider;

/**
 * @brief The StreamWriter class. This class is the interface between StreamDataSender and StreamServer.
 * It watches the client socket and spreads data to its client.
 * @ingroup Broadcasting
 */
class StreamWriter :
	public QObject,
	public Engine::RawAudioDataReceiver
{
	Q_OBJECT
	PIMPL(StreamWriter)

	signals:
		void sigNewConnection(const QString& ip);
		void sigDisconnected(StreamWriter* sw);

	public:
		enum class Type : uint8_t
		{
				Undefined,
				Standard,
				Invalid,
				Streaming
		};

		/**
		 * @brief StreamWriter
		 * @param socket
		 * @param ip
		 * @param md
		 */
		StreamWriter(PlayManager* playManager, RawAudioDataProvider* audioDataProvider, QTcpSocket* socket, const QString& ip);
		~StreamWriter() override;

		/**
		 * @brief get client ip address
		 * @return
		 */
		QString ip() const;

		/**
		 * @brief Send a m3u playlist (see StreamDataSender)
		 * @return
		 */
		bool sendPlaylist();

		/**
		 * @brief Send the http favicon (see StreamDataSender)
		 * @return
		 */
		bool sendFavicon();

		/**
		 * @brief Send track information (see StreamDataSender)
		 * @return
		 */
		bool sendMetadata();

		/**
		 * @brief Send website background (see StreamDataSender)
		 * @return
		 */
		bool sendBackground();

		/**
		 * @brief send a html5 website (see StreamDataSender)
		 * @return
		 */
		bool sendHtml5();

		/**
		 * @brief send a appropriate header based on the type of request  (see StreamDataSender)
		 * @param reject if true, a reject header is sent.
		 * @return
		 */
		bool sendHeader(bool reject);

		StreamHttpParser::HttpAnswer parseMessage();

		/**
		 * @brief disconnect a client socket
		 */
		void disconnect();

		/**
		 * @brief stop sending sound over the client socket
		 */
		void dismiss();

		/**
		 * @brief new audio data has arrived and has to be forwarded to the socket
		 * @param data
		 * @param size
		 */
		void writeAudioData(const QByteArray& data) override;

	private:
		void reset();

	private slots:
		void socketDisconnected();
		void dataAvailble();
		void clearSocket();
};

using StreamWriterPtr = std::shared_ptr<StreamWriter>;
#endif
