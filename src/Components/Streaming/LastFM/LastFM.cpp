/* LastFM.cpp */

/* Copyright (C) 2011-2020 Michael Lugmair (Lucio Carreras)
 *
 * This file is part of sayonara player
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Gfeneral Public License as published by
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
 * LastFM.cpp
 *
 *  Created on: Apr 19, 2011
 *      Author: Michael Lugmair (Lucio Carreras)
 */

#include "LastFM.h"
#include "LFMGlobals.h"
#include "LFMTrackChangedThread.h"
#include "LFMLoginThread.h"
#include "LFMWebAccess.h"
#include "Interfaces/Notification/NotificationHandler.h"

#include "Utils/Algorithm.h"
#include "Utils/FileUtils.h"
#include "Utils/RandomGenerator.h"
#include "Utils/Playlist/PlaylistMode.h"
#include "Utils/Settings/Settings.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Crypt.h"

#include "Components/PlayManager/PlayManager.h"
#include "Components/Playlist/Playlist.h"
#include "Components/Playlist/PlaylistHandler.h"

#include "Database/Connector.h"
#include "Database/LibraryDatabase.h"

#include <QDomDocument>
#include <QUrl>
#include <QTimer>

#include <ctime>

namespace Algorithm = Util::Algorithm;

using namespace LastFM;

struct Base::Private
{
	QString sessionKey;
	QTimer* scrobbleTimer = nullptr;
	QTimer* trackChangedTimer = nullptr;
	TrackChangedThread* trackChangeThread = nullptr;
	bool loggedIn;

	Private(QObject* parent) :
		trackChangeThread(new TrackChangedThread(parent)),
		loggedIn(false)
	{
		scrobbleTimer = new QTimer();
		scrobbleTimer->setSingleShot(true);

		trackChangedTimer = new QTimer();
		trackChangedTimer->setSingleShot(true);
	}
};

Base::Base() :
	QObject()
{
	m = Pimpl::make<Private>(this);

	connect(m->scrobbleTimer, &QTimer::timeout, this, &Base::scrobble);
	connect(m->trackChangedTimer, &QTimer::timeout, this, &Base::trackChangedTimerTimedOut);
	connect(PlayManager::instance(), &PlayManager::sigCurrentTrackChanged,
	        this, &Base::currentTrackChanged);

	ListenSetting(Set::LFM_Active, Base::activeChanged);
}

Base::~Base() = default;

bool Base::isLoggedIn()
{
	return m->loggedIn;
}

void Base::login(const QString& username, const QString& password)
{
	auto* loginThread = new LoginThread(this);

	connect(loginThread, &LoginThread::sigLoggedIn, this, &Base::loginThreadFinished);
	connect(loginThread, &LoginThread::sigError,
	        this,
	        [=](const QString& error_message) {
		        spLog(Log::Warning, this) << error_message;
		        emit sigLoggedIn(false);
	        });

	loginThread->login(username, password);
}

void Base::activeChanged()
{
	m->loggedIn = false;

	if(GetSetting(Set::LFM_Active))
	{
		const QString username = GetSetting(Set::LFM_Username);
		const QString password = Util::Crypt::decrypt(GetSetting(Set::LFM_Password));

		login(username, password);
	}
}

void Base::loginThreadFinished(bool success)
{
	auto* loginThread = static_cast<LoginThread*>(sender());

	m->loggedIn = success;

	const auto errorMessage = tr("Cannot login to Last.fm");
	if(!success)
	{
		NotificationHandler::instance()->notify("Sayonara", errorMessage);
		emit sigLoggedIn(false);
		return;
	}

	const auto loginInfo = loginThread->getLoginStuff();
	m->loggedIn = loginInfo.loggedIn;
	m->sessionKey = loginInfo.sessionKey;

	SetSetting(Set::LFM_SessionKey, m->sessionKey);

	spLog(Log::Debug, this) << "Got session key";

	if(!m->loggedIn)
	{
		NotificationHandler::instance()->notify("Sayonara", errorMessage);
		spLog(Log::Warning, this) << "Cannot login";
	}

	emit sigLoggedIn(m->loggedIn);

	sender()->deleteLater();
}

void Base::currentTrackChanged(const MetaData& track)
{
	Q_UNUSED(track)

	const auto playlistMode = GetSetting(Set::PL_Mode);
	if(Playlist::Mode::isActiveAndEnabled(playlistMode.dynamic()))
	{
		m->trackChangedTimer->stop();
		m->trackChangedTimer->start(1000);
	}

	// scrobble
	if(GetSetting(Set::LFM_Active) && m->loggedIn)
	{
		const auto seconds = GetSetting(Set::LFM_ScrobbleTimeSec);
		if(seconds > 0)
		{
			m->scrobbleTimer->stop();
			m->scrobbleTimer->start(seconds * 1000);
		}
	}
}

void Base::scrobble()
{
	if(!GetSetting(Set::LFM_Active) || !m->loggedIn)
	{
		return;
	}

	const MetaData track = PlayManager::instance()->currentTrack();
	if(track.title().isEmpty() || track.artist().isEmpty())
	{
		return;
	}

	spLog(Log::Debug, this) << "Scrobble " << track.title() << " by "
	                        << track.artist();

	auto* webAccess = new WebAccess();
	connect(webAccess, &WebAccess::sigResponse, this, &Base::scrobbleResponseReceived);
	connect(webAccess, &WebAccess::sigError, this, &Base::scrobbleErrorReceived);

	auto rawtime = time(nullptr);
	auto* ptm = localtime(&rawtime);
	auto started = mktime(ptm);

	UrlParams sigData;
	if(!track.album().isEmpty())
	{
		sigData["album"] = track.album();
	}

	sigData["api_key"] = LFM_API_KEY;
	sigData["artist"] = track.artist();
	sigData["duration"] = QString::number(track.durationMs() / 1000);
	sigData["method"] = "track.scrobble";
	sigData["sk"] = m->sessionKey;
	sigData["timestamp"] = QString::number(started);
	sigData["track"] = track.title();

	sigData.appendSignature();

	QByteArray postData;
	QString url = webAccess->createPostUrl("http://ws.audioscrobbler.com/2.0/",
	                                       sigData,
	                                       postData);

	webAccess->callPostUrl(url, postData);
}

void Base::scrobbleResponseReceived(const QByteArray& data) { Q_UNUSED(data) }

void Base::scrobbleErrorReceived(const QString& error)
{
	spLog(Log::Warning, this) << "Scrobble: " << error;
}

void Base::trackChangedTimerTimedOut()
{
	const auto& track = PlayManager::instance()->currentTrack();

	if(GetSetting(Set::LFM_Active) && m->loggedIn)
	{
		m->trackChangeThread->updateNowPlaying(m->sessionKey, track);
	}
}
