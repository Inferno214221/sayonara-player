/* IcyWebAccess.cpp */

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

#include "IcyWebAccess.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Macros.h"

#include <QTcpSocket>
#include <QUrl>

struct IcyWebAccess::Private
{
		IcyWebAccess::Status status;
		QTcpSocket* tcp=nullptr;
		QString hostname;
		QString directory;
		QString filename;
		int port;

		Private()
		{
			status = IcyWebAccess::Status::Success;
			port = 80;
		}

		void close_tcp()
		{
			if(tcp->isOpen()){
				tcp->close();
			}

			tcp->deleteLater();
		}

		QString concat_dir_and_filename() const
		{
			QString ret = directory + "/" + filename;
			while(ret.contains("//")){
				ret.replace("//", "/");
			}

			if(!ret.startsWith("/")){
				ret.prepend("/");
			}

			return ret;
		}
};

IcyWebAccess::IcyWebAccess(QObject *parent) :
	QObject(parent)
{
	m = Pimpl::make<Private>();

}

IcyWebAccess::~IcyWebAccess() {}

void IcyWebAccess::check(const QUrl& url)
{
	m->tcp = new QTcpSocket(nullptr);
	m->hostname = url.host(QUrl::PrettyDecoded);
	m->port = url.port(80);
	m->directory = url.path();
	m->filename = url.fileName();
	m->status = IcyWebAccess::Status::NotExecuted;

	connect(m->tcp, &QTcpSocket::connected, this, &IcyWebAccess::connected);
	connect(m->tcp, &QTcpSocket::disconnected, this, &IcyWebAccess::disconnected);
	connect(m->tcp, &QTcpSocket::readyRead, this, &IcyWebAccess::data_available);

	connect(m->tcp, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(error_received(QAbstractSocket::SocketError)));

	m->tcp->connectToHost(m->hostname,
						   m->port,
						   QTcpSocket::ReadWrite,
						   QAbstractSocket::AnyIPProtocol
	);

	sp_log(Log::Develop, this) << "Start ICY Request";
}

void IcyWebAccess::stop()
{
	if(m->tcp && m->tcp->isOpen() && m->tcp->isValid()){
		m->tcp->abort();
		m->tcp->close();
	}
}

IcyWebAccess::Status IcyWebAccess::status() const
{
	return m->status;
}


void IcyWebAccess::connected()
{
	QString user_agent = QString("Sayonara/") + SAYONARA_VERSION;
	QByteArray data(
				"GET " + m->concat_dir_and_filename().toLocal8Bit() + " HTTP/1.1\r\n"
				"User-Agent: " + user_agent.toLocal8Bit() + "\r\n"
				"Connection: Keep-Alive\r\n"
				"Accept-Encoding: gzip, deflate\r\n"
				"Accept-Language: en-US,*\r\n"
				"Host: " +
				m->hostname.toLocal8Bit() + ":" +
				QString::number(m->port).toLocal8Bit() + "\r\n\r\n"
	);

	sp_log(Log::Develop, this) << data;

	int64_t bytes_written = m->tcp->write(data.data(), data.size());
	if(bytes_written != data.size())
	{
		sp_log(Log::Warning, this) << "Could only write " << bytes_written << " bytes";
		m->status = IcyWebAccess::Status::WriteError;
		emit sig_finished();
		m->close_tcp();
	}
}

void IcyWebAccess::disconnected()
{
	sp_log(Log::Develop, this) << "Disconnected";
	if(m->status == IcyWebAccess::Status::NotExecuted) {
		m->status = IcyWebAccess::Status::OtherError;
		emit sig_finished();
	}

	m->close_tcp();

	sender()->deleteLater();
}

void IcyWebAccess::error_received(QAbstractSocket::SocketError socket_state)
{
	Q_UNUSED(socket_state)

	sp_log(Log::Warning, this) << "Icy Webaccess Error: " << m->tcp->errorString();

	m->status = IcyWebAccess::Status::OtherError;
	m->close_tcp();

	emit sig_finished();
}

void IcyWebAccess::data_available()
{
	QByteArray arr = m->tcp->read(20);
	if(arr.contains("ICY 200 OK")){
		m->status = IcyWebAccess::Status::Success;
	}

	else {
		sp_log(Log::Warning, this) << "Icy Answer Error: " << arr;
		m->status = IcyWebAccess::Status::WrongAnswer;
	}

	m->close_tcp();

	emit sig_finished();
}
