/* UDPSocket.cpp
 *
 * Copyright (C) 2011-2024 Michael Lugmair
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

#include "UDPSocket.h"

#include "Utils/Utils.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Algorithm.h"

#include <QUdpSocket>
#include <QNetworkDatagram>
#include <QHostInfo>

struct RemoteUDPSocket::Private
{
	QUdpSocket* socket=nullptr;
};

RemoteUDPSocket::RemoteUDPSocket(QObject* parent) :
	QObject(parent)
{
	m = Pimpl::make<Private>();

	ListenSetting(Set::Remote_Active, RemoteUDPSocket::remoteSettingsChanged);
	ListenSetting(Set::Remote_Discoverable, RemoteUDPSocket::remoteSettingsChanged);
}

RemoteUDPSocket::~RemoteUDPSocket() = default;

void RemoteUDPSocket::dataReceived()
{
	auto* socket = static_cast<QUdpSocket*>(sender());

	while(socket->hasPendingDatagrams())
	{
		const QNetworkDatagram datagram = socket->receiveDatagram(32);

		const QHostAddress address = datagram.senderAddress();
		int port = datagram.senderPort();

		const QByteArray raw_data =	datagram.data();
		const QString data = QString::fromLocal8Bit(raw_data);
		if(data.size() >= 10 && data.startsWith("sayrc01req"))
		{
			QStringList ips = Util::ipAddresses();
			Util::Algorithm::sort(ips, [&address](const QString& ip1, const QString& ip2)
			{
				auto ip4addr = address.toIPv4Address();
				auto ip4addr1 = QHostAddress(ip1).toIPv4Address();
				auto ip4addr2 = QHostAddress(ip2).toIPv4Address();

				decltype(ip4addr) diff1, diff2;
				if(ip4addr < ip4addr1){
					diff1 = ip4addr1 - ip4addr;
				}

				else {
					diff1 = ip4addr - ip4addr1;
				}

				if(ip4addr < ip4addr2){
					diff2 = ip4addr2 - ip4addr;
				}

				else {
					diff2 = ip4addr - ip4addr2;
				}

				return (diff1 < diff2);
			});

			const QString ip_string = ips.join(",") + QString("@%1").arg(GetSetting(Set::Remote_Port)) ;
			const QString public_id = GetSetting(Set::Player_PublicId);

			const QString str = QString("sayrc01%1/%2/%3/%4")
				.arg("ips")
				.arg(public_id)
				.arg(QHostInfo::localHostName())
				.arg(ip_string);

			socket->writeDatagram
			(
				str.toLocal8Bit(),
				address,
				quint16(port)
			);
		}

		else
		{
			spLog(Log::Warning, this) << "Illegal remote control request " << data.size() << ": " << data << " raw data: " << raw_data;
		}

		spLog(Log::Info, this) << data << " from " << address.toString() << ":" << port;
	}
}

void RemoteUDPSocket::remoteSettingsChanged()
{
	setActive(GetSetting(Set::Remote_Active) && GetSetting(Set::Remote_Discoverable));
}

void RemoteUDPSocket::setActive(bool b)
{
	if(b)
	{
		if(m->socket) {
			return;
		}

		m->socket = new QUdpSocket(this);
		m->socket->bind(QHostAddress::Any, 54056);

		connect(m->socket, &QUdpSocket::readyRead, this, &RemoteUDPSocket::dataReceived);
	}

	else
	{
		if(m->socket)
		{
			m->socket->close();
			m->socket->deleteLater();
			m->socket = nullptr;
		}
	}
}
