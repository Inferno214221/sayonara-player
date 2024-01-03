/* AsyncWebAccessTest.cpp
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

#include "test/Common/SayonaraTest.h"

#include "Utils/Utils.h"
#include "Utils/WebAccess/WebClientImpl.h"

#include <QTcpSocket>
#include <QTcpServer>
#include <QByteArray>
#include <QRegExp>
#include <QSignalSpy>

// access working directory with Test::Base::tempPath("somefile.txt");

namespace
{
	constexpr const auto Port = 22444U;

	constexpr const auto EndpointSuccess = "success";
	constexpr const auto EndpointRedirect = "redirect";
	constexpr const auto EndpointStream = "stream";
	constexpr const auto EndpointTimeout = "timeout";
	constexpr const auto EndpointNoData = "nodata";
	constexpr const auto EndpointNoHttpData = "nohttp";

	QString createUrl(const QString& endpoint)
	{
		return QString("http://127.0.0.1:%1/%2").arg(Port).arg(endpoint);
	}

	QByteArray createResponse(int httpStatusCode, const QString& httpStatusString, const QStringList& headerData,
	                          const QString& data = QString())
	{
		const auto header = QString("HTTP/1.1 %1 %2")
			.arg(httpStatusCode)
			.arg(httpStatusString);

		auto response = QStringList();
		response += header;
		response += QString("Content-Length: %1").arg(data.size());
		response += headerData;
		response += QString();
		response += data;

		return response.join("\r\n").toLocal8Bit();
	}

	void sendResponse(QTcpSocket* socket, int httpStatusCode, const QString& httpStatusString,
	                  const QStringList& headerData,
	                  const QString& data = QString())
	{
		const auto response = createResponse(httpStatusCode, httpStatusString, headerData, data);

		// NOLINTNEXTLINE(readability-magic-numbers)
		Util::sleepMs(Util::randomNumber(200, 1000));
		socket->write(response);
		socket->flush();
	}

	QByteArray parseRequest(const QByteArray& request)
	{
		QRegExp re("GET /?([a-zA-Z0-9]+).*");
		return (re.indexIn(request) >= 0) ? re.cap(1).toLocal8Bit() : QByteArray();
	}
}

class WebServer :
	public QObject
{
	Q_OBJECT
	public:
		explicit WebServer(QObject* parent) :
			mServer {std::make_shared<QTcpServer>(parent)}
		{
			connect(mServer.get(), &QTcpServer::newConnection,
			        this, &WebServer::newConnection);
			mServer->listen(QHostAddress::AnyIPv4, Port);
		}

		~WebServer() override
		{
			mServer->close();
			mServer->deleteLater();
		}

	private slots:

		void socketDataAvailable()
		{
			constexpr const auto ResponseOk = 200;
			constexpr const auto ResponseRedirect = 301;
			constexpr const auto ResponseNotFound = 404;

			auto* socket = dynamic_cast<QTcpSocket*>(sender());

			const auto request = socket->readAll();
			const auto data = parseRequest(request);

			if(data == EndpointSuccess)
			{
				const auto header = QStringList()
					<< "Content-Type: text/html"
					<< "Connection: Closed";

				sendResponse(socket, ResponseOk, "OK", header, "success");
			}

			else if(data == EndpointRedirect)
			{
				const auto header = QStringList()
					<< "Location: /success";
				sendResponse(socket, ResponseRedirect, "Moved Permanently", header);
			}

			else if(data == EndpointStream)
			{
				const auto header = QStringList()
					<< "Content-Type: audio/mpeg"
					<< "Connection: keep-alive";

				constexpr const auto StringLength = 1000;
				sendResponse(socket, ResponseOk, "OK", header, Util::randomString(StringLength));
			}

			else if(data == EndpointNoData)
			{
				sendResponse(socket, ResponseOk, "OK", QStringList());
			}

			else if(data == EndpointTimeout)
			{
				return;
			}

			else if(data == EndpointNoHttpData)
			{
				auto icyData = QString("ICY/1.1 200 OK");

				socket->write(icyData.toLocal8Bit());
				socket->flush();
			}

			else
			{
				const auto header = QStringList()
					<< "Content-Type: text/html"
					<< "Connection: close";

				sendResponse(socket, ResponseNotFound, "Not Found", header);
			}
		}

		void newConnection()
		{
			if(auto* socket = mServer->nextPendingConnection(); socket)
			{
				connect(socket, &QTcpSocket::readyRead, this, &WebServer::socketDataAvailable);
			}
		}

	private: // NOLINT(readability-redundant-access-specifiers)
		std::shared_ptr<QTcpServer> mServer;
};

class AsyncWebAccessTest :
	public Test::Base
{
	Q_OBJECT

	public:
		AsyncWebAccessTest() :
			Test::Base("AsyncWebAccessTest") {}

	private slots:
		[[maybe_unused]] void testSuccess();
		[[maybe_unused]] void testNotFound();
		[[maybe_unused]] void testRedirect();
		[[maybe_unused]] void testStream();
		[[maybe_unused]] void testNoData();
		[[maybe_unused]] void testNoHttpData();
		[[maybe_unused]] void testTimeout();
};

[[maybe_unused]] void AsyncWebAccessTest::testSuccess()
{
	WebServer webserver(this);
	auto* webAccess = new WebClientImpl(this);

	QSignalSpy spy(webAccess, &WebClient::sigFinished);
	webAccess->run(createUrl(EndpointSuccess));
	QVERIFY(spy.wait(5000));

	QVERIFY(webAccess->data() == "success");
	QVERIFY(webAccess->hasData());
	QVERIFY(!webAccess->hasError());
	QVERIFY(webAccess->status() == WebClient::Status::GotData);
}

[[maybe_unused]] void AsyncWebAccessTest::testNotFound()
{
	WebServer webserver(this);
	auto webAccess = std::make_shared<WebClientImpl>(this);

	QSignalSpy spy(webAccess.get(), &WebClient::sigFinished);
	webAccess->run(createUrl("foobar"));
	QVERIFY(spy.wait(5000));

	QVERIFY(webAccess->data().isEmpty());
	QVERIFY(!webAccess->hasData());
	QVERIFY(webAccess->hasError());
	QVERIFY(webAccess->status() == WebClient::Status::NotFound);
}

[[maybe_unused]] void AsyncWebAccessTest::testRedirect()
{
	WebServer webserver(this);
	auto webAccess = std::make_shared<WebClientImpl>(this);

	QSignalSpy spy(webAccess.get(), &WebClient::sigFinished);
	webAccess->run(createUrl(EndpointRedirect));
	QVERIFY(spy.wait(5000));

	QVERIFY(webAccess->data() == "success");
	QVERIFY(webAccess->hasData());
	QVERIFY(!webAccess->hasError());
	QVERIFY(webAccess->status() == WebClient::Status::GotData);
}

[[maybe_unused]] void AsyncWebAccessTest::testStream()
{
	WebServer webserver(this);
	auto webAccess = std::make_shared<WebClientImpl>(this);

	QSignalSpy spy(webAccess.get(), &WebClient::sigFinished);
	webAccess->run(createUrl(EndpointStream));
	QVERIFY(spy.wait(5000));

	QVERIFY(webAccess->data().isEmpty());
	QVERIFY(!webAccess->hasData());
	QVERIFY(!webAccess->hasError());
	QVERIFY(webAccess->status() == WebClient::Status::AudioStream);
}

[[maybe_unused]] void AsyncWebAccessTest::testNoData()
{
	WebServer webserver(this);
	auto webAccess = std::make_shared<WebClientImpl>(this);

	QSignalSpy spy(webAccess.get(), &WebClient::sigFinished);
	webAccess->run(createUrl(EndpointNoData));
	QVERIFY(spy.wait(5000));

	QVERIFY(webAccess->data().isEmpty());
	QVERIFY(!webAccess->hasData());
	QVERIFY(!webAccess->hasError());
	QVERIFY(webAccess->status() == WebClient::Status::NoData);
}

[[maybe_unused]] void AsyncWebAccessTest::testNoHttpData()
{
	WebServer webserver(this);
	auto webAccess = std::make_shared<WebClientImpl>(this);

	QSignalSpy spy(webAccess.get(), &WebClient::sigFinished);
	webAccess->run(createUrl(EndpointNoHttpData));
	QVERIFY(spy.wait(5000));

	QVERIFY(webAccess->data().isEmpty());
	QVERIFY(!webAccess->hasData());
	QVERIFY(!webAccess->hasError());
	QVERIFY(webAccess->status() == WebClient::Status::NoHttp);
}

[[maybe_unused]] void AsyncWebAccessTest::testTimeout()
{
	WebServer webserver(this);
	auto* webAccess = new WebClientImpl(this);

	QSignalSpy spy(webAccess, &WebClient::sigFinished);
	webAccess->run(createUrl(EndpointTimeout), 2500); // NOLINT(readability-magic-numbers)
	QVERIFY(spy.wait(5000));

	QVERIFY(webAccess->data().isEmpty());
	QVERIFY(!webAccess->hasData());
	QVERIFY(webAccess->hasError());
	QVERIFY(webAccess->status() == WebClient::Status::Timeout);
}

QTEST_GUILESS_MAIN(AsyncWebAccessTest)

#include "AsyncWebAccessTest.moc"
