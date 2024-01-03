/* WebClientImpl.cpp */
/*
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
#include "WebClientImpl.h"

#include "Utils/Logger/Logger.h"
#include "Utils/Utils.h"
#include "Utils/FileUtils.h"
#include "Utils/Macros.h"
#include "WebClientFactory.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QImage>
#include <QTimer>

namespace
{

	bool isStream(QNetworkReply* reply)
	{
		const auto contentType = reply->header(QNetworkRequest::ContentTypeHeader).toString().toLower();
		return contentType.contains("audio/") ||
		       contentType.contains("video/");
	}

	bool isPlaylistFile(const QUrl& url)
	{
		return Util::File::isPlaylistFile(url.fileName());
	}

	WebClient::Status convertNetworkError(QNetworkReply::NetworkError networkError)
	{
		switch(networkError)
		{
			case QNetworkReply::ContentNotFoundError:
				return WebClient::Status::NotFound;
			case QNetworkReply::OperationCanceledError:
				return WebClient::Status::Timeout;
			default:
				return WebClient::Status::Error;
		}
	}

	QString checkGetUrl(QString url)
	{
		url.replace(QStringLiteral("itpc://"), QStringLiteral("http://"));
		url.replace(QStringLiteral("feed://"), QStringLiteral("http://"));

		return url;
	}

	void setRequestHeader(QNetworkRequest& request, const QMap<QByteArray, QByteArray>& headerData)
	{
		for(auto it = headerData.cbegin(); it != headerData.cend(); it++)
		{
			request.setRawHeader(it.key(), it.value());
		}
	}

	std::pair<QByteArray, WebClient::Status> checkReplyData(QNetworkReply* reply, WebClient* webAccess)
	{
		if((reply->bytesAvailable() > 0) && reply->isReadable())
		{
			spLog(Log::Develop, webAccess) << "Got " << reply->bytesAvailable() << " bytes";
			return std::make_pair(reply->readAll(), WebClient::Status::GotData);
		}

		spLog(Log::Develop, webAccess) << "Answer contains no data";
		return std::make_pair(QByteArray(), WebClient::Status::NoData);
	}

	std::pair<QByteArray, WebClient::Status> checkReplyError(QNetworkReply* reply, WebClient* webAccess)
	{
		const auto networkError = reply->error();

		spLog(Log::Warning, webAccess) << QString("Cannot open %1: %2 (%3)")
			.arg(reply->request().url().toString())
			.arg(reply->errorString())
			.arg(static_cast<int>(networkError));

		const auto status = (reply->rawHeaderList().isEmpty())
		                    ? WebClient::Status::NoHttp
		                    : convertNetworkError(networkError);

		return std::make_pair(QByteArray(), status);
	}

	QString getUserAgent(const WebClient::Mode mode)
	{
		switch(mode)
		{
			case WebClient::Mode::AsSayonara:
				return QString("Sayonara/%1").arg(SAYONARA_VERSION);
			case WebClient::Mode::AsBrowser:
				return QStringLiteral("Mozilla/5.0 (Linux; rv:35.0) Gecko/20100101 Firefox/35.0");
			case WebClient::Mode::Random:
			{
				constexpr const auto MinimumStringLength = 8;
				constexpr const auto MaximumStringLength = 16;
				return Util::randomString(Util::randomNumber(MinimumStringLength, MaximumStringLength));
			}

			case WebClient::Mode::None:
			default:
				return {};
		}
	}
} //namespace

struct WebClientImpl::Private
{
	AbstractWebClientStopper* stopper;
	QNetworkAccessManager* networkAccessManager;
	QString url;
	QByteArray data;
	QMap<QByteArray, QByteArray> header;
	WebClientImpl::Mode mode {WebClient::Mode::AsBrowser};
	WebClientImpl::Status status {WebClientImpl::Status::NoError};

	explicit Private(WebClientImpl* parent) :
		stopper {new AbstractWebClientStopper(parent)},
		networkAccessManager {new QNetworkAccessManager(parent)} {}
};

WebClientImpl::WebClientImpl(QObject* parent) :
	WebClient(parent),
	m {Pimpl::make<Private>(this)}
{
	connect(parent, &QObject::destroyed, this, &WebClientImpl::stop);
}

WebClientImpl::~WebClientImpl() = default;

void WebClientImpl::run(const QString& url, int timeout)
{
	reset();
	m->url = checkGetUrl(url);

	auto request = QNetworkRequest(m->url);
	request.setMaximumRedirectsAllowed(2);
	setRequestHeader(request, m->header);
	request.setHeader(QNetworkRequest::UserAgentHeader, getUserAgent(m->mode));
	request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
	                     QNetworkRequest::NoLessSafeRedirectPolicy);

	spLog(Log::Debug, this) << "Call " << request.url().toString();

	auto* reply = m->networkAccessManager->get(request);
	connect(reply, &QNetworkReply::readyRead, this, &WebClientImpl::dataAvailable);
	connect(reply, &QNetworkReply::finished, this, &WebClientImpl::finished);

	connect(m->stopper, &AbstractWebClientStopper::sigStopped, reply, &QNetworkReply::abort);
	connect(m->stopper, &AbstractWebClientStopper::sigTimeout, this, &WebClientImpl::timeout);
	connect(m->stopper, &AbstractWebClientStopper::sigTimeout, reply, &QNetworkReply::abort);

	m->stopper->startTimer(timeout);
}

void WebClientImpl::runPost(const QString& url, const QByteArray& postData, int timeout)
{
	reset();
	m->url = url;

	auto request = QNetworkRequest(m->url);
	setRequestHeader(request, m->header);
	request.setHeader(QNetworkRequest::ContentTypeHeader,
	                  QStringLiteral("application/x-www-form-urlencoded"));

	auto* reply = m->networkAccessManager->post(request, postData);
	connect(reply, &QNetworkReply::finished, this, &WebClientImpl::finished);

	connect(m->stopper, &AbstractWebClientStopper::sigStopped, reply, &QNetworkReply::abort);
	connect(m->stopper, &AbstractWebClientStopper::sigTimeout, reply, &QNetworkReply::abort);

	m->stopper->startTimer(timeout);
}

QByteArray WebClientImpl::data() const { return m->data; }

QString WebClientImpl::url() const { return m->url; }

void WebClientImpl::setMode(WebClientImpl::Mode mode) { m->mode = mode; }

void WebClientImpl::setRawHeader(const QMap<QByteArray, QByteArray>& header) { m->header = header; }

WebClientImpl::Status WebClientImpl::status() const { return m->status; }

bool WebClientImpl::hasData() const { return (m->status == WebClientImpl::Status::GotData); }

bool WebClientImpl::hasError() const
{
	return (m->status == WebClientImpl::Status::Error) ||
	       (m->status == WebClientImpl::Status::Timeout) ||
	       (m->status == WebClientImpl::Status::NotFound);
}

void WebClientImpl::stop()
{
	m->stopper->stop();
}

void WebClientImpl::dataAvailable()
{
	auto* reply = dynamic_cast<QNetworkReply*>(sender());
	if(isStream(reply) && !isPlaylistFile(QUrl(m->url)))
	{
		m->status = WebClientImpl::Status::AudioStream;
		m->data.clear();
		m->stopper->stop();
	}
}

void WebClientImpl::finished()
{
	m->stopper->stopTimer();

	auto* reply = dynamic_cast<QNetworkReply*>(sender());
	if(reply->error() == QNetworkReply::NoError)
	{
		std::tie(m->data, m->status) = checkReplyData(reply, this);
	}

	else if(m->status == WebClientImpl::Status::NoError)
	{
		std::tie(m->data, m->status) = checkReplyError(reply, this);
	}

	emit sigFinished(); // NOLINT(readability-misleading-indentation)
	reply->deleteLater();
}

void WebClientImpl::timeout()
{
	spLog(Log::Debug, this) << "Timeout reached " << m->url;

	m->status = WebClientImpl::Status::Timeout;
	m->data.clear();
}

void WebClientImpl::reset()
{
	m->status = WebClientImpl::Status::NoError;
	m->data.clear();
	m->networkAccessManager->clearAccessCache();
}

struct AbstractWebClientStopper::Private
{
	QTimer* timer;

	explicit Private(QObject* parent) :
		timer {new QTimer(parent)} {}
};

AbstractWebClientStopper::AbstractWebClientStopper(QObject* parent) :
	QObject {parent},
	m {Pimpl::make<Private>(this)}
{
	m->timer->setSingleShot(true);
	connect(m->timer, &QTimer::timeout, this, &AbstractWebClientStopper::timeout);
}

AbstractWebClientStopper::~AbstractWebClientStopper() noexcept = default;

void AbstractWebClientStopper::startTimer(int timeout)
{
	if(timeout > 0)
	{
		m->timer->start(timeout);
	}
}

void AbstractWebClientStopper::stopTimer()
{
	m->timer->stop();
}

void AbstractWebClientStopper::stop()
{
	stopTimer();
	emit sigStopped();
}

void AbstractWebClientStopper::timeout()
{
	stopTimer();
	emit sigTimeout();
}
