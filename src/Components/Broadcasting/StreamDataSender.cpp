/* StreamDataSender.cpp */

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

/* This module is responsible for sending data
 * (stream data, http data) to the client
 * Most of the functions are called by Streamwriter
 */

#include "StreamDataSender.h"
#include "Utils/Utils.h"
#include "Utils/FileUtils.h"
#include "Utils/Logger/Logger.h"

#include <QByteArray>
#include <QTcpSocket>
#include <QPair>

static char padding[256];

using HttpHeaderPair=QPair<QString, QString>;

struct StreamDataSender::Private
{
	QTcpSocket*		socket=nullptr;
	qint64			bytes_written;

	QByteArray		header;
	QByteArray		icy_header;
	QByteArray		reject_header;

	QString			track_path;

	Private(QTcpSocket* out_socket)
	{
		bytes_written = 0;
		socket = out_socket;

		header = QByteArray
		(
			"ICY 200 Ok\r\n"
			"icy-notice1:Bliblablupp\r\n"
			"icy-notice2:asdfasd\r\n"
			"icy-name:Sayonara Player Radio\r\n"
			"icy-genre:\r\n"
			"icy-url:http://sayonara-player.com\r\n"
			"icy-pub:1\r\n"
			"icy-br:192\r\n"
			"Accept-Ranges:none\r\n"
			"content-type:audio/mpeg\r\n"
			"connection:keep-alive\r\n"
		);

		icy_header = QByteArray
		(
			"ICY 200 Ok\r\n"
			"icy-notice1:Bliblablupp\r\n"
			"icy-notice2:asdfasd\r\n"
			"icy-name:Sayonara Player Radio\r\n"
			"icy-genre:\r\n"
			"icy-url:http://sayonara-player.com\r\n"
			"icy-pub:1\r\n"
			"icy-br:192\r\n"
			"icy-metaint:8192\r\n"
			"Accept-Ranges:none\r\n"
			"content-type:audio/mpeg\r\n"
			"connection:keep-alive\r\n"
		);

		reject_header = QByteArray("HTTP/1.1 501 Not Implemented\r\nConnection: close\r\n");

		header.append("\r\n");
		icy_header.append("\r\n");
		reject_header.append("\r\n");

		track_path = Util::random_string(16) + ".mp3";
	}

	QByteArray create_http_header(const QList<HttpHeaderPair>& lst)
	{
		QByteArray arr;
		arr.push_back("HTTP/1.1 200 OK\r\n");

		for(const HttpHeaderPair& p : lst)
		{
			arr.push_back(p.first.toLocal8Bit());
			arr.push_back(": ");
			arr.push_back(p.second.toLocal8Bit());
			arr.push_back("\r\n");
		}

		arr.push_back("\r\n");

		return arr;
	}

	qint64 send_answer(const QString& content_type, const QByteArray& content, const QString& connection_state)
	{
		QList<HttpHeaderPair> header_info
		{
			HttpHeaderPair("content-type", content_type),
			HttpHeaderPair("content-length", QString::number(content.size())),
			HttpHeaderPair("Connection", connection_state)
		};

		QByteArray data = create_http_header(header_info) + content;

		return socket->write(data);
	}

	void flush()
	{
		socket->reset();
	}
};

StreamDataSender::StreamDataSender(QTcpSocket* socket)
{
	memset(padding, 0, 256);

	m = Pimpl::make<Private>(socket);
}

StreamDataSender::~StreamDataSender() = default;

bool StreamDataSender::send_trash()
{
	char single_byte = 0x00;
	int64_t n_bytes;

	n_bytes = m->socket->write(&single_byte, 1);

	m->socket->disconnectFromHost();
	m->socket->close();

	return (n_bytes > 0);
}

bool StreamDataSender::send_data(const QByteArray& data)
{
	m->bytes_written = 0;

	auto bytes = m->socket->write(data);

	return (bytes > 0);
}

// [.............................................................] = buffer
// [  bytes_before        | icy_data | bytes_to_write ][remainder]

bool StreamDataSender::send_icy_data(const QByteArray& data, const QString& stream_title)
{
	qint64 bytes_written = 0;
	const int IcySize = 8192;

	if(data.isEmpty())
	{
		return true;
	}

	if(data.size() < (IcySize - m->bytes_written))
	{
		bytes_written = m->socket->write(data);
		if(bytes_written < 0)
		{
			sp_log(Log::Debug, this) << "Something is wrong";
			return false;
		}

		m->bytes_written += bytes_written;
		return true;
	}

	else
	{
		qint64 bytes_before = IcySize - m->bytes_written;

		QByteArray data_before = data.left(int(bytes_before));
		QByteArray data_after = data.mid(int(bytes_before));

		if(!data_before.isEmpty())
		{
			bytes_written = m->socket->write(data_before);
		}

		send_icy_metadata(stream_title);

		// this happens if size > 8192
		if(data_after.size() > IcySize)
		{
			bytes_written = m->socket->write(data_after, IcySize);
			m->bytes_written = 0;

			return send_icy_data(data_after.mid(IcySize), stream_title);
		}

		// there's a some data
		else if(data_after.size() > 0)
		{
			bytes_written = m->socket->write(data_after);
		}

		// zero bytes left, so we start at zero again
		else
		{
			bytes_written = 0;
		}

		m->bytes_written = std::max<qint64>(0, bytes_written) % IcySize;

		return (bytes_written >= 0);
	}
}


bool StreamDataSender::send_icy_metadata(const QString& stream_title)
{
	int64_t n_bytes=0;
	QByteArray metadata;
	{
		metadata.append("StreamTitle='");
		metadata.append( stream_title.toLocal8Bit() );
		metadata.append("';");
		metadata.append("StreamUrl='http://sayonara-player.com';");
	}

	int sz = metadata.size(); // size of icy metadata

	// number of padding bytes
	int n_padding = 16 * ((sz + 15) / 16) - sz;

	metadata.append(padding, n_padding);
	metadata.prepend(char((sz + 15) / 16));

	n_bytes = m->socket->write( metadata );

	return (n_bytes > 0);
}


bool StreamDataSender::send_header(bool reject, bool icy)
{
	int64_t n_bytes=0;

	if(reject){
		n_bytes = m->socket->write( m->reject_header );
	}

	else if(icy)
	{
		n_bytes = m->socket->write( m->icy_header );
	}

	else
	{
		n_bytes = m->socket->write( m->header );
	}

	if(n_bytes <= 0){
		return false;
	}

	if(reject){
		return false;
	}

	return true;
}


bool StreamDataSender::send_html5(const QString& stream_title)
{
	QString html_string;
	bool success = false;

	if(Util::File::exists(Util::sayonara_path("broadcast.html"))) {
		success = Util::File::read_file_into_str(Util::sayonara_path("broadcast.html"), html_string);
	}

	if(!success) {
		success = Util::File::read_file_into_str(":/Broadcasting/broadcast.html", html_string);
	}

	if(!success) {
		return false;
	}

	html_string.replace("$AUDIOSOURCE", m->track_path.toLocal8Bit());
	html_string.replace("$STREAMTITLE", stream_title.toLocal8Bit());

	return (m->send_answer("text", html_string.toLocal8Bit(), "keep-alive") > 0);
}


bool StreamDataSender::send_bg()
{
	QByteArray html;

	bool success = Util::File::read_file_into_byte_arr(":/Broadcasting/background.png", html);
	if(!success) {
		return false;
	}

	return (m->send_answer("image/png", html, "close") > 0);
}


bool StreamDataSender::send_metadata(const QString& stream_title)
{
	QByteArray html = stream_title.toLocal8Bit();

	return (m->send_answer("text/plain", html, "close") > 0);
}


bool StreamDataSender::send_playlist(const QString& host, int port)
{
	QByteArray playlist =
		"#EXTM3U\n\n"
		"#EXTINF:-1, Lucio Carreras - Sayonara Player Radio\n" +
		QString("http://%1:%2/%3\n\n")
			.arg(host)
			.arg(port)
			.arg(m->track_path)
			.toLocal8Bit();

	auto n_bytes = m->send_answer("audio/x-mpegurl", playlist, "close");
	return (n_bytes > 0);
}


bool StreamDataSender::send_favicon()
{
	QByteArray arr;

	bool success = Util::File::read_file_into_byte_arr(":/Broadcasting/favicon.ico", arr);
	if(!success) {
		return false;
	}

	return (m->send_answer("image/x-icon", arr, "close") > 0);
}

void StreamDataSender::flush()
{
	return m->flush();
}
