/* LoginThread.cpp */

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

#include "LFMLoginThread.h"
#include "LFMGlobals.h"
#include "LFMWebAccess.h"

#include "Utils/Logger/Logger.h"
#include "Utils/Message/Message.h"
#include "Utils/Utils.h"

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

		auto* webAccess = new WebAccess();
		connect(webAccess, &WebAccess::sigResponse, this, &LoginThread::webaccessResponseReceived);
		connect(webAccess, &WebAccess::sigError, this, &LoginThread::webaccessErrorReceived);
		connect(webAccess, &WebAccess::sigFinished, this, &QObject::deleteLater);

		m->loginInfo = LoginInfo {};

		const auto urlParams = UrlParams {
			{"api_key",  ApiKey},
			{"method",   AuthMethodName},
			{"password", password},
			{"username", username}};

		const auto postData = LastFM::createPostData(urlParams);
		webAccess->callPostUrl(BaseUrl, postData);
	}

	void LoginThread::webaccessResponseReceived(const QByteArray& data)
	{
		const auto str = QString::fromUtf8(data);
		const auto sessionKey = Util::easyTagFinder("lfm.session.key", str);
		const auto isSubscriber = (Util::easyTagFinder("lfm.session.subscriber", str).toInt() == 1);
		const auto success = (sessionKey.size() >= 32); // NOLINT(readability-magic-numbers)

		m->loginInfo.loggedIn = success;
		m->loginInfo.sessionKey = sessionKey;
		m->loginInfo.subscriber = isSubscriber;
		m->loginInfo.error = str;

		emit sigLoggedIn(success);
	}

	void LoginThread::webaccessErrorReceived(const QString& error)
	{
		spLog(Log::Warning, this) << "LastFM: Cannot login: " << error;

		emit sigError(error);
	}

	LoginInfo LoginThread::loginInfo() const { return m->loginInfo; }
}
