/* WebAccess.cpp */

/* Copyright (C) 2011-2024 Michael Lugmair (Lucio Carreras)
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
#include "Utils/WebAccess/WebClientImpl.h"
#include "Utils/Utils.h"
#include "Utils/Logger/Logger.h"

#include <QCryptographicHash>
#include <QByteArray>
#include <QUrl>

#include <chrono>

namespace LastFM
{
	namespace
	{
		constexpr const auto Timeout = std::chrono::milliseconds(15'000);

		QString createSignature(const UrlParams& urlParams)
		{
			auto signatureItems = QStringList {};

			for(auto it = urlParams.begin(); it != urlParams.end(); it++)
			{
				signatureItems << it.key() + it.value();
			}

			signatureItems.sort();
			signatureItems << LastFM::ApiSecret;

			auto signatureData = signatureItems.join("");

			return Util::calcHash(signatureData.toUtf8());
		}

		QString htmlFormat(const QString& str) { return QUrl::toPercentEncoding(str); }
	} // namespace

	struct WebAccess::Private
	{
		QByteArray data;
	};

	WebAccess::WebAccess(QObject* parent) :
		QObject(parent),
		m {Pimpl::make<Private>()} {}

	WebAccess::~WebAccess() = default;

	WebClient* WebAccess::initWebClient()
	{
		m->data.clear();

		auto* webClient = new WebClientImpl(this);
		connect(webClient, &WebClient::sigFinished, this, &WebAccess::webClientFinished);

		return webClient;
	}

	void WebAccess::callUrl(const QString& url)
	{
		auto* webClient = initWebClient();
		webClient->run(url, Timeout.count());
	}

	void WebAccess::callPostUrl(const QString& url, const QByteArray& postData)
	{
		QMap<QByteArray, QByteArray> header;
		header["Content-Type"] = "application/x-www-form-urlencoded";

		auto* webClient = initWebClient();
		webClient->setRawHeader(header);
		webClient->runPost(url, postData, Timeout.count());
	}

	void WebAccess::webClientFinished()
	{
		auto* webClient = dynamic_cast<WebClient*>(sender());

		m->data = webClient->hasData()
		          ? webClient->data()
		          : webClient->errorData();

		emit sigFinished();
	}

	QByteArray WebAccess::data() const { return m->data; }

	QByteArray createPostData(UrlParams urlParams)
	{
		auto dataList = QStringList {};

		const auto signature = createSignature(urlParams);
		urlParams["api_sig"] = signature;

		for(auto it = urlParams.cbegin(); it != urlParams.cend(); it++)
		{
			const auto item = QString("%1=%2")
				.arg(htmlFormat(it.key()))
				.arg(htmlFormat(it.value()));

			dataList << item;
		}

		dataList.sort();

		return dataList.join('&').toUtf8();
	}
}
