/* WebAccess.cpp */

/* Copyright (C) 2011-2020 Lucio Carreras
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
 *      Author: Lucio Carreras
 */

#include "LFMGlobals.h"
#include "LFMWebAccess.h"
#include "Utils/WebAccess/AsyncWebAccess.h"
#include "Utils/Utils.h"

#include <QCryptographicHash>
#include <QByteArray>

using namespace LastFM;

void WebAccess::call_url(const QString& url)
{
	AsyncWebAccess* awa = new AsyncWebAccess(this);
	connect(awa, &AsyncWebAccess::sig_finished, this, &WebAccess::awa_finished);
	awa->run(url, 10000);
}

void WebAccess::call_post_url(const QString& url, const QByteArray& post_data)
{
	AsyncWebAccess* awa = new AsyncWebAccess(this);
	connect(awa, &AsyncWebAccess::sig_finished, this, &WebAccess::awa_finished);

	QMap<QByteArray, QByteArray> header;
	header["Content-Type"] = "application/x-www-form-urlencoded";

	awa->set_raw_header(header);
	awa->run_post(url, post_data, 10000);
}


void WebAccess::awa_finished()
{
	auto* awa = static_cast<AsyncWebAccess*>(sender());
	if(awa->status() != AsyncWebAccess::Status::GotData)
	{
		emit sig_error("Cannot get data");
	}

	QByteArray data = awa->data();
	bool error = check_error(data);

	if(!error)
	{
		emit sig_response(data);
	}
}

QString WebAccess::WebAccess::create_std_url(const QString& base_url, const UrlParams& data)
{
	QByteArray post_data;

	QString url = create_std_url_post(base_url, data, post_data);
	url += "?";
	url += QString::fromLocal8Bit(post_data);

	return url;
}

QString WebAccess::create_std_url_post(const QString& base_url, const UrlParams& sig_data, QByteArray& post_data)
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


bool WebAccess::check_error(const QByteArray& data)
{
	QString error_str = parse_error_message(data);
	if(!error_str.isEmpty()){
		emit sig_error(error_str);
		return true;
	}

	return false;
}

QString WebAccess::parse_error_message(const QString& response)
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

void UrlParams::append_signature()
{
	QByteArray signature;

	for(auto it=this->cbegin(); it != this->cend(); it++)
	{
		signature += it.key();
		signature += it.value();
	}

	signature += LFM_API_SECRET;

	QByteArray hash = Util::calc_hash(signature);

	this->insert("api_sig", hash);
}
