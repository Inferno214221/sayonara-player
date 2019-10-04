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
	uint64_t		sent_data_bytes;

	QByteArray		header;
	QByteArray		icy_header;
	QByteArray		reject_header;

	QString			track_path;

	Private(QTcpSocket* out_socket)
	{
		sent_data_bytes = 0;
		socket = out_socket;
		header = QByteArray(
					"ICY 200 Ok\r\n"
					"icy-notice1:Bliblablupp\r\n"
					"icy-notice2:asdfasd\r\n"
					"icy-name:Sayonara Player Radio\r\n"
					"icy-genre:Angry songs\r\n"
					"icy-url:http://sayonara-player.com\r\n"
					"icy-pub:1\r\n"
					"icy-br:192\r\n"
					"Accept-Ranges:none\r\n"
					"content-type:audio/mpeg\r\n"
					"connection:keep-alive\r\n"
		);

		icy_header = QByteArray(
					"ICY 200 Ok\r\n"
					"icy-notice1:Bliblablupp\r\n"
					"icy-notice2:asdfasd\r\n"
					"icy-name:Sayonara Player Radio\r\n"
					"icy-genre:Angry songs\r\n"
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
};

StreamDataSender::StreamDataSender(QTcpSocket* socket)
{
	memset(padding, 0, 256);

	m = Pimpl::make<Private>(socket);
}

StreamDataSender::~StreamDataSender() {}

bool StreamDataSender::send_trash()
{
	char single_byte = 0x00;
	int64_t n_bytes;

	n_bytes = m->socket->write(&single_byte, 1);

	m->socket->disconnectFromHost();
	m->socket->close();

	return (n_bytes > 0);
}

bool StreamDataSender::send_data(const uchar* data, uint64_t size)
{
	m->sent_data_bytes = 0;

	int64_t n_bytes = m->socket->write( (const char*) data, size);

	return (n_bytes > 0);
}


// [..........................................................] = buffer
// [  bytes_before        | icy_data | bytes_to_write ]

bool StreamDataSender::send_icy_data(const uchar* data, uint64_t size, const QString& stream_title)
{
	int64_t n_bytes=0;
	int64_t bytes_to_write = 0;
	const int IcySize = 8192;

	if(m->sent_data_bytes + size > IcySize){
		uint64_t bytes_before = IcySize - m->sent_data_bytes;

		if(bytes_before > 0){
			n_bytes = m->socket->write( (const char*) data, bytes_before);
		}

		send_icy_metadata(stream_title);
		bytes_to_write = size - bytes_before;

		// this happens if size > 8192
		if(bytes_to_write > IcySize){
			n_bytes = m->socket->write( (const char*) (data + bytes_before), IcySize);
			bytes_to_write = bytes_to_write - n_bytes;
		}

		else if(bytes_to_write > 0){
			n_bytes = m->socket->write( (const char*) (data + bytes_before), bytes_to_write);
			bytes_to_write = 0;
		}

		// zero bytes left, so we start at zero again
		else{
			n_bytes = 0;
			bytes_to_write = 0;
		}

		m->sent_data_bytes = n_bytes;
	}

	else{
		n_bytes = m->socket->write( (const char*) data, size);
		m->sent_data_bytes += n_bytes;
		if(m->sent_data_bytes > IcySize){
			sp_log(Log::Debug, this) << "Something is wrong";
		}
	}

	if(bytes_to_write > 0){
		send_icy_data(data + IcySize, bytes_to_write, stream_title);
	}

	return (n_bytes > 0);
}


bool StreamDataSender::send_icy_metadata(const QString& stream_title)
{
	bool success;
	int64_t n_bytes=0;
	QByteArray metadata = QByteArray("StreamTitle='");
	int sz; // size of icy metadata
	int n_padding; // number of padding bytes

	metadata.append( stream_title.toLocal8Bit() );
	metadata.append("';");
	metadata.append("StreamUrl='http://sayonara-player.com';");

	sz = metadata.size();
	n_padding = ( (int)((sz + 15) / 16) * 16 - sz );

	metadata.append(padding, n_padding);
	metadata.prepend((char) (int)((sz + 15) / 16));

	n_bytes = m->socket->write( metadata );

	success = (n_bytes > 0);

	return success;
}


bool StreamDataSender::send_header(bool reject, bool icy)
{
	int64_t n_bytes=0;

	if(reject){
		n_bytes = m->socket->write( m->reject_header );
	}

	else if(icy){
		n_bytes = m->socket->write( m->icy_header );
	}

	else{
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
	int n_bytes;
	QByteArray html;
	QByteArray data;

	html = QByteArray() +
			"<!DOCTYPE html>"
			"<html>"
			"<head>"
			"<link rel=\"icon\" href=\"favicon.ico\" type=\"image/x-icon\" />"
			"<title>Sayonara Player Radio</title>"

			"<script>\n"

			"function loadXMLDoc()\n"
			"{\n"
			"  var xmlhttp;\n"
			"  if (window.XMLHttpRequest)\n"
			"  {// code for IE7+, Firefox, Chrome, Opera, Safari\n"
			"    xmlhttp=new XMLHttpRequest();\n"
			"  }\n"
			"  else\n"
			"  {// code for IE6, IE5\n"
			"    xmlhttp=new ActiveXObject(\"Microsoft.XMLHTTP\");\n"
			"  }\n"
			"  xmlhttp.onreadystatechange=function()\n"
			"  {\n"
			"    if (xmlhttp.readyState==4 && xmlhttp.status==200)\n"
			"    {\n"
			"      document.getElementById(\"metadata\").innerHTML=xmlhttp.responseText;\n"
			"    }\n"
			"  }\n"
			"  xmlhttp.open(\"GET\",\"metadata\",true);\n"
			"  xmlhttp.send();\n"
			"}\n"

			"function start(){\n"
			"}"
			"</script>"

			"</head>"
			"<body background=\"bg-checker.png\" onload=\"start()\" style=\"width: 400px; margin: 0 auto; text-align: center;\" >"

			"<h1 style=\"color: #f3841a; font-family: Fredoka One, lucida grande, tahoma, sans-serif; font-weight: 400;\">Sayonara Player Radio</h1>"
			"<audio id=\"player\" autoplay controls>"
			"<source src=\"" + m->track_path.toLocal8Bit() + "\" type=\"audio/mpeg\">"
			"Your browser does not support the audio element."
			"</audio><br /><br />"

			"<div id=\"metadata\" style=\"color: white;\">"
			+ stream_title.toLocal8Bit() +
			"</div><br />"

			"<div style=\"color: white;\">"
			"<button type=\"button\" onclick=\"loadXMLDoc()\" style=\"border: 1px solid #282828; background: #505050; padding: 4px; color: white;\">Refresh</button><br /><br />"
			"</div><br /><br />"
			"<div style=\"color: white;\">"
			"Stream by Lucio Carreras"
			"</div>"

			"</body>"
			"</html>";

	QList<HttpHeaderPair> header_info;
	header_info << HttpHeaderPair("content-type", "text");
	header_info << HttpHeaderPair("content-length", QString::number(html.size()));
	header_info << HttpHeaderPair("Connection", "Keep-alive");

	data = m->create_http_header(header_info) + html;

	n_bytes = m->socket->write(data);

	return (n_bytes > 0);
}


bool StreamDataSender::send_bg()
{
	QByteArray html;

	bool success = Util::File::read_file_into_byte_arr(
		":/Icons/bg-checker.png", html
	);

	if(!success){
		return false;
	}

	QList<HttpHeaderPair> header_info;
	header_info << HttpHeaderPair("content-type", "image/png");
	header_info << HttpHeaderPair("content-length", QString::number(html.size()));
	header_info << HttpHeaderPair("Connection", "close");

	QByteArray data = m->create_http_header(header_info) + html;

	int n_bytes = m->socket->write(data);

	return (n_bytes > 0);
}


bool StreamDataSender::send_metadata(const QString& stream_title)
{
	QByteArray html = stream_title.toLocal8Bit();

	QList<HttpHeaderPair> header_info;
	header_info << HttpHeaderPair("content-type", "text/plain");
	header_info << HttpHeaderPair("content-length", QString::number(html.size()));
	header_info << HttpHeaderPair("Connection", "close");

	QByteArray data = m->create_http_header(header_info) + html;

	int n_bytes = m->socket->write(data);

	return (n_bytes > 0);
}


bool StreamDataSender::send_playlist(const QString& host, int port)
{
	QByteArray playlist = QByteArray("#EXTM3U\n\n"
						  "#EXTINF:-1, Lucio Carreras - Sayonara Player Radio\n" +
						  QString("http://%1:%2/%3\n\n")
							.arg(host)
							.arg(port)
							.arg(m->track_path)
							.toLocal8Bit()
	);

	QList<HttpHeaderPair> header_info;
	header_info << HttpHeaderPair("content-type", "audio/x-mpegurl");
	header_info << HttpHeaderPair("content-length", QString::number(playlist.size()));
	header_info << HttpHeaderPair("Connection", "close");

	QByteArray data = m->create_http_header(header_info) + playlist;

	int n_bytes = m->socket->write(data);

	return (n_bytes > 0);
}


bool StreamDataSender::send_favicon()
{
	QByteArray arr;
	bool success = Util::File::read_file_into_byte_arr(":/Icons/favicon.ico", arr);

	if(!success){
		return false;
	}

	QList<HttpHeaderPair> header_info;
	header_info << HttpHeaderPair("content-type", "image/x-icon");
	header_info << HttpHeaderPair("content-length", QString::number(arr.size()));
	header_info << HttpHeaderPair("Connection", "close");

	QByteArray data = m->create_http_header(header_info) + arr;

	int n_bytes = m->socket->write(data);

	return (n_bytes > 0);
}
