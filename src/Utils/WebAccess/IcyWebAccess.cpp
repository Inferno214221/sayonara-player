/* IcyWebAccess.cpp */

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

#include "IcyWebAccess.h"
#include "Utils/FileUtils.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Macros.h"

#include <QTcpSocket>
#include <QUrl>

namespace
{
	constexpr const auto StandardPort = 80;

	QString concatDirAndFilename(const QString& directory, const QString& filename)
	{
		auto ret = "/" + directory + "/" + filename;
		return Util::File::cleanFilename(ret);
	}

	void closeTcpSocket(QTcpSocket* socket)
	{
		if(socket->isOpen())
		{
			socket->close();
		}

		socket->deleteLater();
	}
}

struct IcyWebAccess::Private
{
	IcyWebAccess::Status status {IcyWebAccess::Status::Success};
	QTcpSocket* tcp {nullptr};
	QString hostname;
	QString directory;
	QString filename;
	int port {StandardPort};
};

IcyWebAccess::IcyWebAccess(QObject* parent) :
	QObject(parent)
{
	m = Pimpl::make<Private>();
}

IcyWebAccess::~IcyWebAccess() = default;

void IcyWebAccess::check(const QUrl& url)
{
	m->tcp = new QTcpSocket(nullptr);
	m->hostname = url.host(QUrl::PrettyDecoded);
	m->port = url.port(StandardPort);
	m->directory = url.path();
	m->filename = url.fileName();
	m->status = IcyWebAccess::Status::NotExecuted;

	connect(m->tcp, &QTcpSocket::connected, this, &IcyWebAccess::connected);
	connect(m->tcp, &QTcpSocket::disconnected, this, &IcyWebAccess::disconnected);
	connect(m->tcp, &QTcpSocket::readyRead, this, &IcyWebAccess::dataAvailable);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
	connect(m->tcp, &QAbstractSocket::errorOccurred, this, &IcyWebAccess::errorReceived);
#else
	const auto signal = static_cast<void (QAbstractSocket::*) (QAbstractSocket::SocketError)>(&QAbstractSocket::error);
	connect(m->tcp, signal, this, &IcyWebAccess::errorReceived);
#endif

	m->tcp->connectToHost(m->hostname, m->port, QTcpSocket::ReadWrite, QAbstractSocket::AnyIPProtocol);

	spLog(Log::Develop, this) << "Start ICY Request";
}

void IcyWebAccess::stop()
{
	if(m->tcp && m->tcp->isOpen() && m->tcp->isValid())
	{
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
	const auto userAgent = QString("Sayonara/%1").arg(SAYONARA_VERSION);
	const auto targetPath = concatDirAndFilename(m->directory, m->filename);
	const auto header = QStringList {
		QString("GET %1 HTTP/1.1").arg(targetPath),
		QString("User-Agent: %1").arg(userAgent),
		"Connection: Keep-Alive",
		"Accept-Encoding: gzip, deflate",
		"Accept-Language: en-US,*",
		QString("Host: %1:%2").arg(m->hostname).arg(m->port),
		"\r\n"
	};

	const auto data = header.join("\r\n").toLocal8Bit();

	spLog(Log::Develop, this) << data;

	const auto bytesWritten = m->tcp->write(data.data(), data.size());
	if(bytesWritten != data.size())
	{
		spLog(Log::Warning, this) << "Could only write " << bytesWritten << " bytes";
		m->status = IcyWebAccess::Status::WriteError;
		emit sigFinished();
		closeTcpSocket(m->tcp);
	}
}

void IcyWebAccess::disconnected()
{
	spLog(Log::Develop, this) << "Disconnected";
	if(m->status == IcyWebAccess::Status::NotExecuted)
	{
		m->status = IcyWebAccess::Status::OtherError;
		emit sigFinished();
	}

	closeTcpSocket(m->tcp);

	sender()->deleteLater();
}

void IcyWebAccess::errorReceived(QAbstractSocket::SocketError /*socketState*/)
{
	spLog(Log::Warning, this) << "Icy Webaccess Error: " << m->tcp->errorString();

	m->status = IcyWebAccess::Status::OtherError;

	closeTcpSocket(m->tcp);
	emit sigFinished();
}

void IcyWebAccess::dataAvailable()
{
	constexpr const auto BufferSize = 20;

	const auto arr = m->tcp->read(BufferSize);

	m->status = arr.contains("ICY 200 OK")
	            ? IcyWebAccess::Status::Success
	            : IcyWebAccess::Status::WrongAnswer;

	if(m->status == IcyWebAccess::Status::WrongAnswer)
	{
		spLog(Log::Warning, this) << "Icy Answer Error: " << arr;
	}

	closeTcpSocket(m->tcp);
	emit sigFinished();
}
