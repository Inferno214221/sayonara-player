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

#include "Components/Notification/NotificationHandler.h"
#include "Components/PlayManager/PlayManager.h"
#include "Database/Connector.h"
#include "Utils/Algorithm.h"
#include "Utils/Crypt.h"
#include "Utils/Logger/Logger.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Playlist/PlaylistMode.h"
#include "Utils/RandomGenerator.h"
#include "Utils/Settings/Settings.h"

#include <QUrl>
#include <QTimer>

#include <ctime>

namespace LastFM
{
	namespace
	{
		QString currentTimestamp()
		{
			auto rawtime = time(nullptr);
			auto* ptm = localtime(&rawtime);

			return QString::number(mktime(ptm));
		}
	}
	struct Base::Private
	{
		QString sessionKey;
		PlayManager* playManager;
		NotificationHandler* notificationHandler;
		QTimer* scrobbleTimer {new QTimer()};
		QTimer* trackChangedTimer {new QTimer()};
		TrackChangedThread* trackChangeThread;
		bool loggedIn {false};

		Private(PlayManager* playManager, NotificationHandler* notificationHandler, QObject* parent) :
			playManager(playManager),
			notificationHandler(notificationHandler),
			trackChangeThread(new TrackChangedThread(parent))
		{
			scrobbleTimer->setSingleShot(true);
			trackChangedTimer->setSingleShot(true);
		}
	};

	Base::Base(PlayManager* playManager, NotificationHandler* notificationHandler)
	{
		m = Pimpl::make<Private>(playManager, notificationHandler, this);

		connect(m->scrobbleTimer, &QTimer::timeout, this, &Base::scrobble);
		connect(m->trackChangedTimer, &QTimer::timeout, this, &Base::trackChangedTimerTimedOut);
		connect(m->playManager, &PlayManager::sigCurrentTrackChanged, this, &Base::currentTrackChanged);

		ListenSetting(Set::LFM_Active, Base::activeChanged);
	}

	Base::~Base() = default;

	bool Base::isLoggedIn() { return m->loggedIn; }

	void Base::login(const QString& username, const QString& password)
	{
		auto* loginThread = new LoginThread(this);

		connect(loginThread, &LoginThread::sigLoggedIn, this, &Base::loginThreadFinished);
		connect(loginThread, &LoginThread::sigError, this, [&](const auto& errorMessage) {
			spLog(Log::Warning, this) << "Login Error: " << errorMessage;
			emit sigLoggedIn(false);
		});

		loginThread->login(username, password);
	}

	void Base::activeChanged()
	{
		m->loggedIn = false;

		if(GetSetting(Set::LFM_Active))
		{
			const auto username = GetSetting(Set::LFM_Username);
			const auto password = Util::Crypt::decrypt(GetSetting(Set::LFM_Password));

			login(username, password);
		}
	}

	void Base::loginThreadFinished(const bool success)
	{
		auto* loginThread = dynamic_cast<LoginThread*>(sender());

		const auto errorMessage = tr("Cannot login to Last.fm");
		if(!success)
		{
			m->loggedIn = false;
		}

		else
		{
			const auto loginInfo = loginThread->loginInfo();
			m->loggedIn = loginInfo.loggedIn;
			m->sessionKey = loginInfo.sessionKey;

			SetSetting(Set::LFM_SessionKey, m->sessionKey);
			spLog(Log::Debug, this) << "Got session key";
		}

		if(!m->loggedIn)
		{
			m->notificationHandler->notify("Sayonara", errorMessage);
			spLog(Log::Warning, this) << "Cannot login";
		}

		emit sigLoggedIn(m->loggedIn);

		sender()->deleteLater();
	}

	void Base::currentTrackChanged(const MetaData& /*track*/)
	{
		const auto playlistMode = GetSetting(Set::PL_Mode);
		if(Playlist::Mode::isActiveAndEnabled(playlistMode.dynamic()))
		{
			constexpr const auto Timeout = 1000;
			m->trackChangedTimer->stop();
			m->trackChangedTimer->start(Timeout);
		}

		if(GetSetting(Set::LFM_Active) && m->loggedIn)
		{
			const auto seconds = GetSetting(Set::LFM_ScrobbleTimeSec);
			if(seconds > 0)
			{
				m->scrobbleTimer->stop();
				m->scrobbleTimer->start(seconds * 1000); // NOLINT(readability-magic-numbers)
			}
		}
	}

	void Base::scrobble()
	{
		if(!GetSetting(Set::LFM_Active) || !m->loggedIn)
		{
			return;
		}

		const auto& track = m->playManager->currentTrack();
		if(track.title().isEmpty() || track.artist().isEmpty())
		{
			return;
		}

		spLog(Log::Debug, this) << "Scrobble " << track.title() << " by " << track.artist();

		auto* webAccess = new WebAccess();
		connect(webAccess, &WebAccess::sigError, this, &Base::scrobbleErrorReceived);
		connect(webAccess, &WebAccess::sigFinished, this, &QObject::deleteLater);

		constexpr const auto* MethodName = "track.scrobble";

		auto urlParams = UrlParams {
			{"api_key",   ApiKey},
			{"artist",    track.artist()},
			{"duration",  QString::number(track.durationMs() / 1000)},// NOLINT(readability-magic-numbers)
			{"method",    MethodName},
			{"sk",        m->sessionKey},
			{"timestamp", currentTimestamp()},
			{"track",     track.title()}};

		if(!track.album().isEmpty())
		{
			urlParams["album"] = track.album();
		}

		const auto postData = LastFM::createPostData(urlParams);
		webAccess->callPostUrl(BaseUrl, postData);
	}

	void Base::scrobbleErrorReceived(const QString& error) // NOLINT(readability-make-member-function-const)
	{
		spLog(Log::Warning, this) << "Scrobble: " << error;
	}

	void Base::trackChangedTimerTimedOut()
	{
		if(GetSetting(Set::LFM_Active) && m->loggedIn)
		{
			const auto& track = m->playManager->currentTrack();
			m->trackChangeThread->updateNowPlaying(m->sessionKey, track);
		}
	}
}
