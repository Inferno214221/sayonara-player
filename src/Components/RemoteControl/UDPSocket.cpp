#include "UDPSocket.h"

#include "Utils/Utils.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Logger/Logger.h"

#include <QUdpSocket>
#include <QNetworkDatagram>

struct RemoteUDPSocket::Private
{
	QUdpSocket* socket=nullptr;
};

RemoteUDPSocket::RemoteUDPSocket(QObject* parent) :
	QObject(parent)
{
	m = Pimpl::make<Private>();

	ListenSetting(Set::Remote_Active, RemoteUDPSocket::remote_settings_changed);
	ListenSetting(Set::Remote_Discoverable, RemoteUDPSocket::remote_settings_changed);
}

RemoteUDPSocket::~RemoteUDPSocket() = default;

#include "Utils/Algorithm.h"

static QString answer_string(const QString& answer_id, const QString& sayonara_id, const QString& text)
{
	return QString("sayrc01%1/%2/%3/%4")
		.arg(answer_id)
		.arg(sayonara_id)
		.arg(text.size())
		.arg(text);
}

void RemoteUDPSocket::data_received()
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
			QStringList ips = Util::ip_addresses();
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

			QString ip_string = ips.join(",") + QString("@%1").arg(GetSetting(Set::Remote_Port)) ;
			QString str = answer_string("ips", GetSetting(Set::Player_PublicId), ip_string);

			socket->writeDatagram
			(
				str.toLocal8Bit(),
				address,
				port
			);
		}

		else
		{
			sp_log(Log::Warning, this) << "Illegal remote control request " << data.size() << ": " << data << " raw data: " << raw_data;
		}

		sp_log(Log::Info, this) << data << " from " << address.toString() << ":" << port;
	}
}

void RemoteUDPSocket::remote_settings_changed()
{
	set_active(GetSetting(Set::Remote_Active) && GetSetting(Set::Remote_Discoverable));
}

void RemoteUDPSocket::set_active(bool b)
{
	if(b)
	{
		if(m->socket) {
			return;
		}

		m->socket = new QUdpSocket(this);
		m->socket->bind(QHostAddress::Any, 54056);

		connect(m->socket, &QUdpSocket::readyRead, this, &RemoteUDPSocket::data_received);
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
