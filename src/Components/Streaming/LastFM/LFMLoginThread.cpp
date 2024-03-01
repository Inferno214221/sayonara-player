/* LoginThread.cpp */

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

#include "LFMLoginThread.h"
#include "LFMGlobals.h"
#include "LFMWebAccess.h"

#include "Utils/Logger/Logger.h"
#include "Utils/Message/Message.h"
#include "Utils/Utils.h"
#include "Utils/WebAccess/WebClientFactory.h"

#include <QXmlStreamReader>

namespace
{
	LastFM::LoginInfo parse(const QByteArray& data)
	{
		const auto s = QString::fromLocal8Bit(data);
		auto result = LastFM::LoginInfo {};
		auto reader = QXmlStreamReader(data);
		while(reader.readNextStartElement())
		{
			const auto name = reader.name().toString();
			if(reader.name() == "lfm")
			{
				const auto attributes = reader.attributes();
				if(attributes.hasAttribute("status"))
				{
					const auto status = attributes.value("status").toString();
					result.hasError = (status != "ok");
				}
			}

			if(reader.name() == "name")
			{
				result.name = reader.readElementText();
			}

			else if(reader.name() == "key")
			{
				result.key = reader.readElementText();
			}

			else if(reader.name() == "error")
			{
				const auto attributes = reader.attributes();
				if(attributes.hasAttribute("code"))
				{
					result.errorCode = attributes.value("code").toInt();
				}

				result.error = reader.readElementText();
			}
		}

		return result;
	}
}

namespace LastFM
{
	struct LoginThread::Private
	{
		LoginInfo loginInfo;
	};

	LoginThread::LoginThread(QObject* parent) :
		QObject(parent),
		m {Pimpl::make<Private>()} {}

	LoginThread::~LoginThread() = default;

	void LoginThread::login(const QString& username, const QString& password)
	{
		constexpr const auto* AuthMethodName = "auth.getMobileSession";

		auto* webAccess = new WebAccess(std::make_shared<WebClientFactory>(), this);
		connect(webAccess, &WebAccess::sigFinished, this, &LoginThread::webClientFinished);
		connect(webAccess, &WebAccess::sigFinished, webAccess, &QObject::deleteLater);

		m->loginInfo = LoginInfo {};

		const auto urlParams = UrlParams {
			{"api_key",  ApiKey},
			{"method",   AuthMethodName},
			{"password", password},
			{"username", username}};

		const auto postData = LastFM::createPostData(urlParams);
		webAccess->callPostUrl(BaseUrl, postData);
	}

	void LoginThread::webClientFinished()
	{
		auto* webClient = dynamic_cast<WebAccess*>(sender());
		m->loginInfo = parse(webClient->data());

		emit sigFinished();
	}

	LoginInfo LoginThread::loginInfo() const { return m->loginInfo; }
}
