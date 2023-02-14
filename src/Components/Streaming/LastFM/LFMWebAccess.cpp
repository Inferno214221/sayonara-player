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
#include "Utils/WebAccess/WebClientImpl.h"
#include "Utils/Utils.h"
#include "Utils/Logger/Logger.h"

#include <QCryptographicHash>
#include <QByteArray>
#include <QUrl>

namespace LastFM
{
	namespace
	{
		constexpr const auto Timeout = 10'000;

		QString parseErrorMessage(const QString& response)
		{
			if(response.isEmpty())
			{
				return {};
			}

			constexpr const auto BufferSize = 100;
			if(response.leftRef(BufferSize).contains(QStringLiteral("failed")))
			{
				return response;
			}

			return {};
		}

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

		QString htmlFormat(const QString& str)
		{
			return QUrl::toPercentEncoding(str);
		}
	}

	void WebAccess::callUrl(const QString& url)
	{
		auto* webClient = new WebClientImpl(this);
		connect(webClient, &WebClient::sigFinished, this, &WebAccess::webClientFinished);
		webClient->run(url, Timeout);
	}

	void WebAccess::callPostUrl(const QString& url, const QByteArray& postData)
	{
		auto* webClient = new WebClientImpl(this);
		connect(webClient, &WebClient::sigFinished, this, &WebAccess::webClientFinished);

		QMap<QByteArray, QByteArray> header;
		header["Content-Type"] = "application/x-www-form-urlencoded";

		webClient->setRawHeader(header);
		webClient->runPost(url, postData, Timeout);
	}

	void WebAccess::webClientFinished()
	{
		auto* webClient = dynamic_cast<WebClient*>(sender());
		if(webClient->status() != WebClient::Status::GotData)
		{
			emit sigError("Cannot get data");
		}

		const auto data = webClient->data();
		const auto error = checkError(data);
		if(!error)
		{
			emit sigResponse(data);
		}

		emit sigFinished();

		webClient->deleteLater();
	}

	bool WebAccess::checkError(const QByteArray& data)
	{
		const auto errorString = parseErrorMessage(data);
		if(!errorString.isEmpty())
		{
			spLog(Log::Error, this) << QString::fromUtf8(data);
			emit sigError(errorString);
		}

		return (!errorString.isEmpty());
	}

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
