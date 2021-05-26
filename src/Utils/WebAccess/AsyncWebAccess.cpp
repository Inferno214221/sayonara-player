/* AsyncWebAccess.cpp */

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

#include "Utils/WebAccess/AsyncWebAccess.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Utils.h"
#include "Utils/FileUtils.h"
#include "Utils/Macros.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QImage>
#include <QTimer>

namespace
{
	bool isStream(QNetworkReply* reply)
	{
		const auto contentType = reply->header(QNetworkRequest::ContentTypeHeader).toString();
		return contentType.toLower().contains("audio/") ||
		       contentType.toLower().contains("video/");
	}

	bool isPlaylistFile(const QUrl& url)
	{
		return Util::File::isPlaylistFile(url.fileName());
	}

	AsyncWebAccess::Status convertNetworkError(QNetworkReply::NetworkError networkError)
	{
		switch(networkError)
		{
			case QNetworkReply::ContentNotFoundError:
				return AsyncWebAccess::Status::NotFound;
			case QNetworkReply::OperationCanceledError:
				return AsyncWebAccess::Status::Timeout;
			default:
				return AsyncWebAccess::Status::Error;
		}
	}

	QString checkGetUrl(const QString& url)
	{
		auto result = url;

		result.replace(QStringLiteral("itpc://"), QStringLiteral("http://"));
		result.replace(QStringLiteral("feed://"), QStringLiteral("http://"));

		return result;
	}

	void setRequestHeader(QNetworkRequest& request, const QMap<QByteArray, QByteArray>& headerData)
	{
		for(auto it = headerData.cbegin(); it != headerData.cend(); it++)
		{
			request.setRawHeader(it.key(), it.value());
		}
	}

	std::pair<QByteArray, AsyncWebAccess::Status> checkReplyData(QNetworkReply* reply, AsyncWebAccess* webAccess)
	{
		if((reply->bytesAvailable() > 0) && reply->isReadable())
		{
			spLog(Log::Develop, webAccess) << "Got " << reply->bytesAvailable() << " bytes";
			return std::make_pair(reply->readAll(), AsyncWebAccess::Status::GotData);
		}

		spLog(Log::Develop, webAccess) << "Answer contains no data";
		return std::make_pair(QByteArray(), AsyncWebAccess::Status::NoData);
	}

	std::pair<QByteArray, AsyncWebAccess::Status> checkReplyError(QNetworkReply* reply, AsyncWebAccess* webAccess)
	{
		const auto networkError = reply->error();

		spLog(Log::Warning, webAccess) << QString("Cannot open %1: %2 (%3)")
			.arg(reply->request().url().toString())
			.arg(reply->errorString())
			.arg(static_cast<int>(networkError));

		const auto status = (reply->rawHeaderList().isEmpty())
		                    ? AsyncWebAccess::Status::NoHttp
		                    : convertNetworkError(networkError);

		return std::make_pair(QByteArray(), status);
	}

	QString getUserAgent(AsyncWebAccess::Behavior behavior)
	{
		switch(behavior)
		{
			case AsyncWebAccess::Behavior::AsSayonara:
				return QString("Sayonara/%1").arg(SAYONARA_VERSION);
			case AsyncWebAccess::Behavior::AsBrowser:
				return QStringLiteral("Mozilla/5.0 (Linux; rv:35.0) Gecko/20100101 Firefox/35.0");
			case AsyncWebAccess::Behavior::Random:
				return Util::randomString(Util::randomNumber(8, 16));
			case AsyncWebAccess::Behavior::None:
			default:
				return QString();
		}
	}
}

struct AsyncWebAccess::Private
{
	AsyncWebAccessStopper* stopper;
	QNetworkAccessManager* networkAccessManager;
	QString url;
	QByteArray data;
	QMap<QByteArray, QByteArray> header;

	AsyncWebAccess::Behavior behavior;
	AsyncWebAccess::Status status {AsyncWebAccess::Status::NoError};

	Private(AsyncWebAccess* parent, AsyncWebAccess::Behavior behavior) :
		stopper {new AsyncWebAccessStopper(parent)},
		networkAccessManager {new QNetworkAccessManager(parent)},
		behavior {behavior} {}

	void reset()
	{
		status = AsyncWebAccess::Status::NoError;
		data.clear();
		networkAccessManager->clearAccessCache();
	}
};

AsyncWebAccess::AsyncWebAccess(QObject* parent, AsyncWebAccess::Behavior behavior) :
	QObject(parent),
	AbstractWebAccess()
{
	m = Pimpl::make<Private>(this, behavior);

	connect(m->stopper, &AsyncWebAccessStopper::sigTimeout, this, &AsyncWebAccess::timeout);
	connect(parent, &QObject::destroyed, this, &AsyncWebAccess::stop);
}

AsyncWebAccess::~AsyncWebAccess() = default;

void AsyncWebAccess::run(const QString& url, int timeout)
{
	m->reset();
	m->url = checkGetUrl(url);

	auto request = QNetworkRequest(m->url);
	request.setMaximumRedirectsAllowed(2);
	setRequestHeader(request, m->header);
	request.setHeader(QNetworkRequest::UserAgentHeader, getUserAgent(m->behavior));
	request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
	request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);

	spLog(Log::Debug, this) << "Call " << request.url().toString();

	auto* reply = m->networkAccessManager->get(request);
	connect(reply, &QNetworkReply::readyRead, this, &AsyncWebAccess::dataAvailable);
	connect(reply, &QNetworkReply::finished, this, &AsyncWebAccess::finished);

	connect(m->stopper, &AsyncWebAccessStopper::sigStopped, reply, &QNetworkReply::abort);
	connect(m->stopper, &AsyncWebAccessStopper::sigTimeout, this, &AsyncWebAccess::timeout);
	connect(m->stopper, &AsyncWebAccessStopper::sigTimeout, reply, &QNetworkReply::abort);

	m->stopper->startTimer(timeout);
}

void AsyncWebAccess::runPost(const QString& url, const QByteArray& postData, int timeout)
{
	m->reset();
	m->url = url;

	auto request = QNetworkRequest(m->url);
	setRequestHeader(request, m->header);
	request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/x-www-form-urlencoded"));

	auto* reply = m->networkAccessManager->post(request, postData);
	connect(reply, &QNetworkReply::finished, this, &AsyncWebAccess::finished);

	connect(m->stopper, &AsyncWebAccessStopper::sigStopped, reply, &QNetworkReply::abort);
	connect(m->stopper, &AsyncWebAccessStopper::sigTimeout, reply, &QNetworkReply::abort);

	m->stopper->startTimer(timeout);
}

void AsyncWebAccess::dataAvailable()
{
	auto* reply = static_cast<QNetworkReply*>(sender());
	if(isStream(reply) && !isPlaylistFile(QUrl(m->url)))
	{
		m->status = AsyncWebAccess::Status::AudioStream;
		m->data.clear();
		m->stopper->stop();
	}
}

void AsyncWebAccess::finished()
{
	m->stopper->stopTimer();

	auto* reply = static_cast<QNetworkReply*>(sender());
	if(reply->error() == QNetworkReply::NoError)
	{
		std::tie(m->data, m->status) = checkReplyData(reply, this);
	}

	else if(m->status == AsyncWebAccess::Status::NoError)
	{
		std::tie(m->data, m->status) = checkReplyError(reply, this);
	}

	emit sigFinished();
	reply->deleteLater();
}

void AsyncWebAccess::timeout()
{
	spLog(Log::Debug, this) << "Timeout reached";

	m->status = AsyncWebAccess::Status::Timeout;
	m->data.clear();
}

QByteArray AsyncWebAccess::data() const
{
	return m->data;
}

QImage AsyncWebAccess::image() const
{
	return QImage::fromData(m->data);
}

QString AsyncWebAccess::url() const
{
	return m->url;
}

void AsyncWebAccess::setBehavior(AsyncWebAccess::Behavior behavior)
{
	m->behavior = behavior;
}

void AsyncWebAccess::setRawHeader(const QMap<QByteArray, QByteArray>& header)
{
	m->header = header;
}

AsyncWebAccess::Status AsyncWebAccess::status() const
{
	return m->status;
}

bool AsyncWebAccess::hasData() const
{
	return (m->status == AsyncWebAccess::Status::GotData);
}

bool AsyncWebAccess::hasError() const
{
	return (m->status == AsyncWebAccess::Status::Error) ||
	       (m->status == AsyncWebAccess::Status::Timeout) ||
	       (m->status == AsyncWebAccess::Status::NotFound);
}

void AsyncWebAccess::stop()
{
	m->stopper->stop();
}

struct AsyncWebAccessStopper::Private
{
	QTimer* timer;

	Private(AsyncWebAccessStopper* parent) :
		timer {new QTimer(parent)}
	{
		timer->setSingleShot(true);
	}
};

AsyncWebAccessStopper::AsyncWebAccessStopper(QObject* parent) :
	QObject(parent)
{
	m = Pimpl::make<Private>(this);

	connect(m->timer, &QTimer::timeout, this, &AsyncWebAccessStopper::timeout);
}

AsyncWebAccessStopper::~AsyncWebAccessStopper() noexcept = default;

void AsyncWebAccessStopper::startTimer(int timeout)
{
	if(timeout > 0)
	{
		m->timer->start(timeout);
	}
}

void AsyncWebAccessStopper::timeout()
{
	stopTimer();
	emit sigTimeout();
}

void AsyncWebAccessStopper::stop()
{
	stopTimer();
	emit sigStopped();
}

void AsyncWebAccessStopper::stopTimer()
{
	m->timer->stop();
}