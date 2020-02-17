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
#include "Utils/Utils.h"
#include "Utils/Message/Message.h"
#include "Utils/Logger/Logger.h"

using namespace LastFM;

struct LoginThread::Private
{
	LoginStuff loginInfo;
};

LoginThread::LoginThread(QObject* parent) :
	QObject(parent)
{
	m = Pimpl::make<Private>();
}

LoginThread::~LoginThread() = default;

void LoginThread::login(const QString& username, const QString& password)
{
	auto* lfm_wa = new WebAccess();
	connect(lfm_wa, &WebAccess::sigResponse, this, &LoginThread::webaccessResponseReceived);
	connect(lfm_wa, &WebAccess::sigError, this, &LoginThread::webaccessErrorReceived);

	m->loginInfo.loggedIn = false;
	m->loginInfo.sessionKey = "";
	m->loginInfo.subscriber = false;

	UrlParams signature_data;
		signature_data["api_key"] = LFM_API_KEY;
		signature_data["method"] = "auth.getMobileSession";
		signature_data["password"] = password.toLocal8Bit();
		signature_data["username"] = username.toLocal8Bit();

	signature_data.appendSignature();

	QByteArray post_data;
	QString url = lfm_wa->createPostUrl("https://ws.audioscrobbler.com/2.0/", signature_data, post_data);

	lfm_wa->callPostUrl(url, post_data);
}

void LoginThread::webaccessResponseReceived(const QByteArray& data)
{
	QString str = QString::fromUtf8(data);

	m->loginInfo.loggedIn = true;
	m->loginInfo.sessionKey = Util::easyTagFinder("lfm.session.key", str);
	m->loginInfo.subscriber = (Util::easyTagFinder("lfm.session.subscriber", str).toInt() == 1);
	m->loginInfo.error = str;

	if(m->loginInfo.sessionKey.size() >= 32){
		emit sigLoggedIn(true);
	}

	else {
		emit sigLoggedIn(false);
	}

	if(sender()){
		sender()->deleteLater();
	}
}

void LoginThread::webaccessErrorReceived(const QString& error)
{
	spLog(Log::Warning, this) << "LastFM: Cannot login";
	spLog(Log::Warning, this) << error;

	emit sigError(error);

	if(sender()){
		sender()->deleteLater();
	}
}

LoginStuff LoginThread::getLoginStuff()
{
	return m->loginInfo;
}

