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
	auto* awa = new AsyncWebAccess(this);
	connect(awa, &AsyncWebAccess::sigFinished, this, &WebAccess::awaFinished);
	awa->run(url, 10000);
}

void WebAccess::callPostUrl(const QString& url, const QByteArray& post_data)
{
	auto* awa = new AsyncWebAccess(this);
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

	const auto data = awa->data();
	const auto error = checkError(data);
	if(!error)
	{
		emit sigResponse(data);
	}

	emit sigFinished();
}

QString WebAccess::createPostUrl(const QString& baseUrl, const UrlParams& signatureData,
                                 QByteArray& postData)
{
	postData.clear();

	QStringList dataList;
	for(auto it = signatureData.cbegin(); it != signatureData.cend(); it++)
	{
		auto item = QString("%1=%2")
			.arg(it.key())
			.arg(it.value());

		item.replace('&', "%26");
		dataList << item;
	}

	postData = dataList.join('&').toUtf8();

	return baseUrl;
}

bool WebAccess::checkError(const QByteArray& data)
{
	const auto errorString = parseErrorMessage(data);
	if(!errorString.isEmpty())
	{
		emit sigError(errorString);
	}

	return (!errorString.isEmpty());
}

QString WebAccess::parseErrorMessage(const QString& response)
{
	if(response.isEmpty())
	{
		return QString();
	}

	if(response.leftRef(100).contains(QStringLiteral("failed")))
	{
		return response;
	}

	return QString();
}

UrlParams::UrlParams() :
	QMap<QString, QString>() {}

void UrlParams::appendSignature()
{
	QString signature;

	for(auto it = this->cbegin(); it != this->cend(); it++)
	{
		signature += it.key();
		signature += it.value();
	}

	signature += LFM_API_SECRET;

	const auto hash = Util::calcHash(signature.toUtf8());
	this->insert("api_sig", hash);
}
