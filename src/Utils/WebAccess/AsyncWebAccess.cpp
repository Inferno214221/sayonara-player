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
#include <QNetworkProxy>
#include <QImage>
#include <QTimer>
#include <QRegExp>

using HeaderMap=QMap<QByteArray, QByteArray>;

struct AsyncWebAccess::Private
{
	QNetworkAccessManager*	nam=nullptr;
	QTimer*					timer=nullptr;
	QNetworkReply*			reply=nullptr;

	QString					url;
	QByteArray				data;
	HeaderMap				header;

	AsyncWebAccess::Behavior	behavior;
	AsyncWebAccess::Status		status;
	bool						ignore_finished;
	bool						aborted;

	Private(AsyncWebAccess::Behavior behavior) :
		behavior(behavior),
		status(AsyncWebAccess::Status::NoData),
		ignore_finished(false),
		aborted(false)
	{}

	~Private()
	{
		if(reply){
			reply->deleteLater(); reply=nullptr;
		}
	}

	void abort_request(bool ignore_finished_slot=false)
	{
		ignore_finished = ignore_finished_slot;
		aborted = true;

		if(reply)
		{
			if(reply->isRunning())
			{
				reply->abort();
				spLog(Log::Debug, this) << "Request was aborted: " << url;
			}

			delete_reply();
		}

		if(timer){
			timer->stop();
		}
	}

	void delete_reply()
	{
		if(reply){
			reply->deleteLater(); reply = nullptr;
		}
	}
};

AsyncWebAccess::AsyncWebAccess(QObject* parent, const QByteArray& header, AsyncWebAccess::Behavior behavior) :
	QObject(parent),
	AbstractWebAccess()
{
	Q_UNUSED(header)

	m = Pimpl::make<Private>(behavior);

	m->nam = new QNetworkAccessManager(this);
	m->timer = new QTimer(this);

	connect(m->timer, &QTimer::timeout, this, &AsyncWebAccess::timeout);
	connect(parent, &QObject::destroyed, this, &AsyncWebAccess::stop);
}

AsyncWebAccess::~AsyncWebAccess() = default;

void AsyncWebAccess::run(const QString& url, int timeout)
{
	m->status = AsyncWebAccess::Status::NoData;
	m->header.clear();
	m->data.clear();
	m->url = url;
	m->nam->clearAccessCache();
	m->ignore_finished = false;
	m->aborted = false;

	QRegExp re("(itpc|feed)://");
	if(re.indexIn(url) >= 0){
		m->url.replace(re, "http://");
	}

	QNetworkRequest request;
	request.setUrl(m->url);

	QString user_agent;

	switch(m->behavior){
	case AsyncWebAccess::Behavior::AsSayonara:
		user_agent = "Sayonara/" + QString(SAYONARA_VERSION) ;
		break;

	case AsyncWebAccess::Behavior::AsBrowser:
		user_agent = "Mozilla/5.0 (Linux; rv:35.0) Gecko/20100101 Firefox/35.0";
		break;

	case AsyncWebAccess::Behavior::Random:
		user_agent = Util::randomString(Util::randomNumber(8, 16));
		break;
	case AsyncWebAccess::Behavior::None:
	default:
		break;
	}

	request.setHeader(QNetworkRequest::UserAgentHeader, user_agent);

	const QString urlString = request.url().toString();
	spLog(Log::Debug, this) << "Call " << urlString;

	m->reply = m->nam->get(request);

	connect(m->reply, &QNetworkReply::readyRead, this, &AsyncWebAccess::dataAvailable);
	connect(m->reply, &QNetworkReply::finished, this, &AsyncWebAccess::finished);

	if(timeout > 0){
		m->timer->start(timeout);
	}
}

void AsyncWebAccess::runPost(const QString& url, const QByteArray &post_data, int timeout)
{
	m->status = AsyncWebAccess::Status::NoData;
	m->header.clear();
	m->data.clear();
	m->url = url;
	m->nam->clearAccessCache();
	m->ignore_finished = false;
	m->aborted = false;

	QUrl my_url(url);
	QNetworkRequest request(my_url);
	request.setHeader(QNetworkRequest::ContentTypeHeader, QString("application/x-www-form-urlencoded"));

	if(!m->header.isEmpty())
	{
		for(auto it=m->header.cbegin(); it != m->header.cend(); it++)
		{
			request.setRawHeader(it.key(), it.value());
		}
	}

	m->reply  = m->nam->post(request, post_data);
	connect(m->reply , &QNetworkReply::finished, this, &AsyncWebAccess::finished);

	if(timeout > 0){
		m->timer->start(timeout);
	}
}

void AsyncWebAccess::dataAvailable()
{
	spLog(Log::Crazy, this) << "Data available";
	QNetworkReply* reply = static_cast<QNetworkReply*>(sender());

	int content_length = reply->header(QNetworkRequest::ContentLengthHeader).toInt();
	qint32 length = static_cast<qint32>(content_length + 1024 * 1024); // some streaming sites use maxint for streams
	bool valid_length = (content_length > 0) && (length > 0);

	QString content_type = reply->header(QNetworkRequest::ContentTypeHeader).toString();
	QString url_file = QUrl(m->url).fileName();

	if(content_type.contains("audio/", Qt::CaseInsensitive) &&
			(valid_length == false) &&
			(Util::File::isPlaylistFile(url_file) == false))
	{
		m->abort_request(true);
		m->status = AsyncWebAccess::Status::AudioStream;
		emit sigFinished();
	}
}

void AsyncWebAccess::redirected(const QUrl& url)
{
	Q_UNUSED(url)
}


void AsyncWebAccess::finished()
{
	QNetworkReply* reply = static_cast<QNetworkReply*>(sender());

	if(m->ignore_finished)
	{
		m->abort_request();
		return;
	}

	QNetworkReply::NetworkError err = reply->error();
	bool success = (err == QNetworkReply::NoError);

	spLog(Log::Develop, this) << reply->url().toString() << " finished: " << (int) err;
	if(success)
	{
		QString redirect_url = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();

		if( !redirect_url.isEmpty() &&
				redirect_url != m->url)
		{
			redirectRequest(redirect_url);
			return;
		}

		if( reply->bytesAvailable() > 0 )
		{
			spLog(Log::Develop, this) << "Got " << reply->bytesAvailable() << " bytes";
			m->data = reply->readAll();
			m->status = AsyncWebAccess::Status::GotData;
		}

		else {
			spLog(Log::Develop, this) << "Answer contains no data";
			m->status = AsyncWebAccess::Status::NoData;
			m->data.clear();
		}
	}

	else
	{
		if(!m->aborted)
		{
			spLog(Log::Warning, this) << "Cannot open " << m->url << ": "
								   << reply->errorString()
								   << " (" << (int) err << ")";
		}

		if(err == QNetworkReply::TimeoutError)
		{
			m->status = AsyncWebAccess::Status::Timeout;
		}

		else if(err == QNetworkReply::RemoteHostClosedError)
		{
			m->status = AsyncWebAccess::Status::NoHttp;
		}

		else {
			m->status = AsyncWebAccess::Status::Error;
		}
	}

	m->abort_request();

	emit sigFinished();
}


void AsyncWebAccess::timeout()
{
	spLog(Log::Debug, this) << "Timeout reached";
	m->abort_request();
}


void AsyncWebAccess::redirectRequest(QString redirect_url)
{
	if(redirect_url.startsWith("/")){
		QUrl new_url(m->url);
		redirect_url.prepend(new_url.scheme() + "://" + new_url.host());
	}

	spLog(Log::Debug, this) << "Redirect from " << m->url << " to " << redirect_url;

	m->abort_request();

	m->url = redirect_url;

	run(redirect_url);
}

QByteArray AsyncWebAccess::data() const
{
	return m->data;
}

QImage AsyncWebAccess::image() const
{
	QImage img;
	img.loadFromData(m->data);
	return img;
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
	switch(m->status)
	{
	case AsyncWebAccess::Status::Error:
	case AsyncWebAccess::Status::Timeout:
	case AsyncWebAccess::Status::NoHttp:
		return true;

	default:
		return false;
	}
}

void AsyncWebAccess::stop()
{
	m->abort_request();
}
