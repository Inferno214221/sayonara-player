/* WebAccess.cpp */

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


/*
 * WebAccess.cpp
 *
 *  Created on: Oct 22, 2011
 *      Author: Michael Lugmair (Lucio Carreras)
 */

#include "LFMGlobals.h"
#include "LFMWebAccess.h"
#include "Utils/WebAccess/AsyncWebAccess.h"
#include "Utils/Utils.h"

#include <QCryptographicHash>
#include <QByteArray>

using namespace LastFM;

void WebAccess::callUrl(const QString& url)
{
	AsyncWebAccess* awa = new AsyncWebAccess(this);
	connect(awa, &AsyncWebAccess::sigFinished, this, &WebAccess::awaFinished);
	awa->run(url, 10000);
}

void WebAccess::callPostUrl(const QString& url, const QByteArray& post_data)
{
	AsyncWebAccess* awa = new AsyncWebAccess(this);
	connect(awa, &AsyncWebAccess::sigFinished, this, &WebAccess::awaFinished);

	QMap<QByteArray, QByteArray> header;
	header["Content-Type"] = "application/x-www-form-urlencoded";

	awa->setRawHeader(header);
	awa->runPost(url, post_data, 10000);
}


void WebAccess::awaFinished()
{
	auto* awa = static_cast<AsyncWebAccess*>(sender());
	if(awa->status() != AsyncWebAccess::Status::GotData)
	{
		emit sigError("Cannot get data");
	}

	QByteArray data = awa->data();
	bool error = checkError(data);

	if(!error)
	{
		emit sigResponse(data);
	}
}

QString WebAccess::WebAccess::createStandardUrl(const QString& base_url, const UrlParams& data)
{
	QByteArray post_data;

	QString url = createPostUrl(base_url, data, post_data);
	url += "?";
	url += QString::fromLocal8Bit(post_data);

	return url;
}

QString WebAccess::createPostUrl(const QString& base_url, const UrlParams& sig_data, QByteArray& post_data)
{
	QString url = base_url;

	post_data.clear();

	for(auto it=sig_data.cbegin(); it != sig_data.cend(); it++)
	{
		QByteArray entry = it.key() + "=" + it.value();
		entry.replace("&", "%26");
		entry += QChar('&');

		post_data += entry;
	}

	post_data.remove(post_data.size() - 1, 1);

	return url;
}


bool WebAccess::checkError(const QByteArray& data)
{
	QString error_str = parseErrorMessage(data);
	if(!error_str.isEmpty()){
		emit sigError(error_str);
		return true;
	}

	return false;
}

QString WebAccess::parseErrorMessage(const QString& response)
{
	if(response.isEmpty()){
		return "";
	}

	if(response.leftRef(100).contains(QStringLiteral("failed")))
	{
		return response;
	}

	return "";
}


UrlParams::UrlParams() : QMap<QByteArray, QByteArray>() {}

void UrlParams::appendSignature()
{
	QByteArray signature;

	for(auto it=this->cbegin(); it != this->cend(); it++)
	{
		signature += it.key();
		signature += it.value();
	}

	signature += LFM_API_SECRET;

	QByteArray hash = Util::calcHash(signature);

	this->insert("api_sig", hash);
}
