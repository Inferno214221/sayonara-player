#include "SayonaraTest.h"
// access working directory with Test::Base::tempPath("somefile.txt");

#include "Utils/Utils.h"
#include "Utils/WebAccess/AsyncWebAccess.h"

#include <QTcpSocket>
#include <QTcpServer>
#include <QByteArray>
#include <QRegExp>
#include <QSignalSpy>

namespace
{
	constexpr const auto Port = 22444u;

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
}

class WebServer :
	public QObject
{
	Q_OBJECT
	public:
		WebServer(QObject* parent) :
			mServer {std::make_shared<QTcpServer>(parent)}
		{
			connect(mServer.get(), &QTcpServer::newConnection,
			        this, &WebServer::newConnection);
			mServer->listen(QHostAddress::AnyIPv4, Port);
		}

		~WebServer()
		{
			mServer->close();
			mServer->deleteLater();
		}

	private slots:

		void socketDataAvailable()
		{
			auto* socket = static_cast<QTcpSocket*>(sender());

			const auto request = socket->readAll();
			const auto data = parseRequest(request);

			if(data == EndpointSuccess)
			{
				const auto header = QStringList()
					<< "Content-Type: text/html"
					<< "Connection: Closed";

				sendResponse(socket, 200, "OK", header, "success");
			}

			else if(data == EndpointRedirect)
			{
				const auto header = QStringList()
					<< "Location: /success";
				sendResponse(socket, 301, "Moved Permanently", header);
			}

			else if(data == EndpointStream)
			{
				const auto header = QStringList()
					<< "Content-Type: audio/mpeg"
					<< "Connection: keep-alive";

				sendResponse(socket, 200, "OK", header, Util::randomString(1'000));
			}

			else if(data == EndpointNoData)
			{
				sendResponse(socket, 200, "OK", QStringList());
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

				sendResponse(socket, 404, "Not Found", header);
			}
		}

		void newConnection()
		{
			if(auto socket = mServer->nextPendingConnection(); socket)
			{
				connect(socket, &QTcpSocket::readyRead, this, &WebServer::socketDataAvailable);
			}
		}

	private:
		void sendResponse(QTcpSocket* socket, int httpStatusCode, const QString& httpStatusString,
		                  const QStringList& headerData,
		                  const QString& data = QString())
		{
			const auto response = createResponse(httpStatusCode, httpStatusString, headerData, data);
			Util::sleepMs(Util::randomNumber(200, 1000));
			socket->write(response);
			socket->flush();
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

		QByteArray parseRequest(const QByteArray& request)
		{
			QRegExp re("GET /?([a-zA-Z0-9]+).*");
			return (re.indexIn(request) >= 0) ? re.cap(1).toLocal8Bit() : QByteArray();
		}

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
		void testSuccess();
		void testNotFound();
		void testRedirect();
		void testStream();
		void testNoData();
		void testNoHttpData();
		void testTimeout();
};

void AsyncWebAccessTest::testSuccess()
{
	WebServer webserver(this);
	auto* webAccess = new AsyncWebAccess(this);

	QSignalSpy spy(webAccess, &AsyncWebAccess::sigFinished);
	webAccess->run(createUrl(EndpointSuccess));
	QVERIFY(spy.wait(5000));

	QVERIFY(webAccess->data() == "success");
	QVERIFY(webAccess->hasData());
	QVERIFY(!webAccess->hasError());
	QVERIFY(webAccess->status() == AsyncWebAccess::Status::GotData);
}

void AsyncWebAccessTest::testNotFound()
{
	WebServer webserver(this);
	auto webAccess = std::make_shared<AsyncWebAccess>(this);

	QSignalSpy spy(webAccess.get(), &AsyncWebAccess::sigFinished);
	webAccess->run(createUrl("foobar"));
	QVERIFY(spy.wait(5000));

	QVERIFY(webAccess->data().isEmpty());
	QVERIFY(!webAccess->hasData());
	QVERIFY(webAccess->hasError());
	QVERIFY(webAccess->status() == AsyncWebAccess::Status::NotFound);
}

void AsyncWebAccessTest::testRedirect()
{
	WebServer webserver(this);
	auto webAccess = std::make_shared<AsyncWebAccess>(this);

	QSignalSpy spy(webAccess.get(), &AsyncWebAccess::sigFinished);
	webAccess->run(createUrl(EndpointRedirect));
	QVERIFY(spy.wait(5000));

	QVERIFY(webAccess->data() == "success");
	QVERIFY(webAccess->hasData());
	QVERIFY(!webAccess->hasError());
	QVERIFY(webAccess->status() == AsyncWebAccess::Status::GotData);
}

void AsyncWebAccessTest::testStream()
{
	WebServer webserver(this);
	auto webAccess = std::make_shared<AsyncWebAccess>(this);

	QSignalSpy spy(webAccess.get(), &AsyncWebAccess::sigFinished);
	webAccess->run(createUrl(EndpointStream));
	QVERIFY(spy.wait(5000));

	QVERIFY(webAccess->data().isEmpty());
	QVERIFY(!webAccess->hasData());
	QVERIFY(!webAccess->hasError());
	QVERIFY(webAccess->status() == AsyncWebAccess::Status::AudioStream);
}

void AsyncWebAccessTest::testNoData()
{
	WebServer webserver(this);
	auto webAccess = std::make_shared<AsyncWebAccess>(this);

	QSignalSpy spy(webAccess.get(), &AsyncWebAccess::sigFinished);
	webAccess->run(createUrl(EndpointNoData));
	QVERIFY(spy.wait(5000));

	QVERIFY(webAccess->data().isEmpty());
	QVERIFY(!webAccess->hasData());
	QVERIFY(!webAccess->hasError());
	QVERIFY(webAccess->status() == AsyncWebAccess::Status::NoData);
}

void AsyncWebAccessTest::testNoHttpData()
{
	WebServer webserver(this);
	auto webAccess = std::make_shared<AsyncWebAccess>(this);

	QSignalSpy spy(webAccess.get(), &AsyncWebAccess::sigFinished);
	webAccess->run(createUrl(EndpointNoHttpData));
	QVERIFY(spy.wait(5000));

	QVERIFY(webAccess->data().isEmpty());
	QVERIFY(!webAccess->hasData());
	QVERIFY(!webAccess->hasError());
	QVERIFY(webAccess->status() == AsyncWebAccess::Status::NoHttp);
}

void AsyncWebAccessTest::testTimeout()
{
	WebServer webserver(this);
	auto* webAccess = new AsyncWebAccess(this);

	QSignalSpy spy(webAccess, &AsyncWebAccess::sigFinished);
	webAccess->run(createUrl(EndpointTimeout), 2500);
	QVERIFY(spy.wait(5000));

	QVERIFY(webAccess->data().isEmpty());
	QVERIFY(!webAccess->hasData());
	QVERIFY(webAccess->hasError());
	QVERIFY(webAccess->status() == AsyncWebAccess::Status::Timeout);
}

QTEST_GUILESS_MAIN(AsyncWebAccessTest)

#include "AsyncWebAccessTest.moc"
