/* StreamWriter.h */

/* Copyright (C) 2011-2020  Lucio Carreras
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
#include "Interfaces/Engine/AudioDataReceiverInterface.h"
#include "Utils/Pimpl.h"

#include <QObject>

class StreamDataSender;
class MetaData;
class QTcpSocket;

/**
 * @brief The StreamWriter class. This class is the interface between StreamDataSender and StreamServer.
 * It watches the client socket and spreads data to its client.
 * @ingroup Broadcasting
 */
class StreamWriter :
		public QObject,
		public Engine::RawSoundReceiverInterface

{
	Q_OBJECT
	PIMPL(StreamWriter)

	signals:
		void sig_new_connection(const QString& ip);
		void sig_disconnected(StreamWriter* sw);


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
		StreamWriter(QTcpSocket* socket, const QString& ip, const MetaData& md);
		~StreamWriter() override;

		/**
		 * @brief get client ip address
		 * @return
		 */
		QString get_ip() const;

		/**
		 * @brief send new icy data to clients, and send new metadata to remote controls.
		 * @param md Track structure
		 */
		void change_track(const MetaData& md);


		/**
		 * @brief Send a m3u playlist (see StreamDataSender)
		 * @return
		 */
		bool send_playlist();

		/**
		 * @brief Send the http favicon (see StreamDataSender)
		 * @return
		 */
		bool send_favicon();

		/**
		 * @brief Send track information (see StreamDataSender)
		 * @return
		 */
		bool send_metadata();

		/**
		 * @brief Send website background (see StreamDataSender)
		 * @return
		 */
		bool send_bg();

		/**
		 * @brief send a html5 website (see StreamDataSender)
		 * @return
		 */
		bool send_html5();

		/**
		 * @brief send a appropriate header based on the type of request  (see StreamDataSender)
		 * @param reject if true, a reject header is sent.
		 * @return
		 */
		bool send_header(bool reject);

		StreamHttpParser::HttpAnswer parse_message();

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
		void new_audio_data(const QByteArray& data) override;

	private:
		void reset();

	private slots:
		void socket_disconnected();
		void data_available();
		void clear_socket();
};

using StreamWriterPtr=std::shared_ptr<StreamWriter>;
#endif
