/* StreamDataSender.cpp */

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

/* This module is responsible for sending data
 * (stream data, http data) to the client
 * Most of the functions are called by Streamwriter
 */

#include "StreamDataSender.h"
#include "Utils/Utils.h"
#include "Utils/FileUtils.h"
#include "Utils/Logger/Logger.h"
#include "Utils/StandardPaths.h"

#include <QByteArray>
#include <QTcpSocket>
#include <QPair>

static char padding[256];

using HttpHeaderPair = QPair<QString, QString>;

struct StreamDataSender::Private
{
	QTcpSocket* socket = nullptr;
	qint64 bytesWritten;
	QByteArray header;
	QByteArray icyHeader;
	QByteArray rejectHeader;
	QString trackPath;

	Private(QTcpSocket* outSocket)
	{
		bytesWritten = 0;
		socket = outSocket;

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

		icyHeader = QByteArray
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

		rejectHeader = QByteArray("HTTP/1.1 501 Not Implemented\r\nConnection: close\r\n");

		header.append("\r\n");
		icyHeader.append("\r\n");
		rejectHeader.append("\r\n");

		trackPath = Util::randomString(16) + ".mp3";
	}

	QByteArray createHttpHeader(const QList<HttpHeaderPair>& lst)
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

	qint64 sendAnswer(const QString& content_type, const QByteArray& content, const QString& connection_state)
	{
		QList<HttpHeaderPair> header_info
			{
				HttpHeaderPair("content-type", content_type),
				HttpHeaderPair("content-length", QString::number(content.size())),
				HttpHeaderPair("Connection", connection_state)
			};

		QByteArray data = createHttpHeader(header_info) + content;

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

bool StreamDataSender::sendTrash()
{
	char singleByte = 0x00;
	auto bytes = m->socket->write(&singleByte, 1);

	m->socket->disconnectFromHost();
	m->socket->close();

	return (bytes > 0);
}

bool StreamDataSender::sendData(const QByteArray& data)
{
	m->bytesWritten = 0;

	auto bytes = m->socket->write(data);
	return (bytes > 0);
}

// [.............................................................] = buffer
// [  bytes_before        | icy_data | bytes_to_write ][remainder]

bool StreamDataSender::sendIcyData(const QByteArray& data, const QString& stream_title)
{
	qint64 bytesWritten = 0;
	const int IcySize = 8192;

	if(data.isEmpty())
	{
		return true;
	}

	if(data.size() < (IcySize - m->bytesWritten))
	{
		bytesWritten = m->socket->write(data);
		if(bytesWritten < 0)
		{
			spLog(Log::Debug, this) << "Something is wrong";
			return false;
		}

		m->bytesWritten += bytesWritten;
		return true;
	}
	else
	{
		qint64 bytes_before = IcySize - m->bytesWritten;

		QByteArray data_before = data.left(int(bytes_before));
		QByteArray data_after = data.mid(int(bytes_before));

		if(!data_before.isEmpty())
		{
			bytesWritten = m->socket->write(data_before);
		}

		sendIcyMetadata(stream_title);

		// this happens if size > 8192
		if(data_after.size() > IcySize)
		{
			bytesWritten = m->socket->write(data_after, IcySize);
			m->bytesWritten = 0;

			return sendIcyData(data_after.mid(IcySize), stream_title);
		}

			// there's a some data
		else if(data_after.size() > 0)
		{
			bytesWritten = m->socket->write(data_after);
		}

			// zero bytes left, so we start at zero again
		else
		{
			bytesWritten = 0;
		}

		m->bytesWritten = std::max<qint64>(0, bytesWritten) % IcySize;

		return (bytesWritten >= 0);
	}
}

bool StreamDataSender::sendIcyMetadata(const QString& stream_title)
{
	QByteArray metadata;
	{
		metadata.append("StreamTitle='");
		metadata.append(stream_title.toLocal8Bit());
		metadata.append("';");
		metadata.append("StreamUrl='http://sayonara-player.com';");
	}

	int sz = metadata.size(); // size of icy metadata

	// number of padding bytes
	int paddingCount = 16 * ((sz + 15) / 16) - sz;

	metadata.append(padding, paddingCount);
	metadata.prepend(char((sz + 15) / 16));

	auto bytes = m->socket->write(metadata);
	return (bytes > 0);
}

bool StreamDataSender::sendHeader(bool reject, bool icy)
{
	int64_t bytes = 0;

	if(reject)
	{
		bytes = m->socket->write(m->rejectHeader);
	}
	else if(icy)
	{
		bytes = m->socket->write(m->icyHeader);
	}
	else
	{
		bytes = m->socket->write(m->header);
	}

	if(bytes <= 0)
	{
		return false;
	}

	if(reject)
	{
		return false;
	}

	return true;
}

bool StreamDataSender::sendHtml5(const QString& stream_title)
{
	QString htmlString;
	bool success = false;

	const auto broadcastHtml = Util::xdgCachePath("broadcast.html");
	if(Util::File::exists(broadcastHtml))
	{
		success = Util::File::readFileIntoString(broadcastHtml, htmlString);
	}

	if(!success)
	{
		success = Util::File::readFileIntoString(":/Broadcasting/broadcast.html", htmlString);
	}

	if(!success)
	{
		return false;
	}

	htmlString.replace("$AUDIOSOURCE", m->trackPath.toLocal8Bit());
	htmlString.replace("$STREAMTITLE", stream_title.toLocal8Bit());

	return (m->sendAnswer("text", htmlString.toLocal8Bit(), "keep-alive") > 0);
}

bool StreamDataSender::sendBackground()
{
	QByteArray html;

	bool success = Util::File::readFileIntoByteArray(":/Broadcasting/background.png", html);
	if(!success)
	{
		return false;
	}

	return (m->sendAnswer("image/png", html, "close") > 0);
}

bool StreamDataSender::sendMetadata(const QString& stream_title)
{
	QByteArray html = stream_title.toLocal8Bit();

	return (m->sendAnswer("text/plain", html, "close") > 0);
}

bool StreamDataSender::sendPlaylist(const QString& host, int port)
{
	const QByteArray playlist =
		"#EXTM3U\n\n"
		"#EXTINF:-1, Michael Lugmair (Lucio Carreras) - Sayonara Player Radio\n" +
		QString("http://%1:%2/%3\n\n")
			.arg(host)
			.arg(port)
			.arg(m->trackPath)
			.toLocal8Bit();

	auto bytes = m->sendAnswer("audio/x-mpegurl", playlist, "close");
	return (bytes > 0);
}

bool StreamDataSender::sendFavicon()
{
	QByteArray arr;

	bool success = Util::File::readFileIntoByteArray(":/Broadcasting/favicon.ico", arr);
	if(!success)
	{
		return false;
	}

	return (m->sendAnswer("image/x-icon", arr, "close") > 0);
}

void StreamDataSender::flush()
{
	return m->flush();
}
