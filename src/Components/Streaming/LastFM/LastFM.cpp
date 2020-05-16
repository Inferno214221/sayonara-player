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

namespace Algorithm=Util::Algorithm;

using namespace LastFM;

struct Base::Private
{
	QString						sessionKey;
	QTimer*						scrobbleTimer=nullptr;
	QTimer*						trackChangedTimer=nullptr;
	TrackChangedThread*			trackChangeThread=nullptr;
	bool						loggedIn;

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

	connect(PlayManager::instance(), &PlayManager::sigCurrentTrackChanged,	this, &Base::currentTrackChanged);
	connect(m->trackChangeThread, &TrackChangedThread::sigSimilarArtistsAvailable,
			this, &Base::similarArtistsFetched);

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
	connect(loginThread, &LoginThread::sigError, this, [=](const QString& error_message){
		spLog(Log::Warning, this) << error_message;
		emit sigLoggedIn(false);
	});

	loginThread->login(username, password);
}

void Base::activeChanged()
{
	m->loggedIn = false;

	bool active = GetSetting(Set::LFM_Active);
	if(active)
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

	const QString errorMessage = tr("Cannot login to Last.fm");
	if(!success) {
		NotificationHandler::instance()->notify("Sayonara", errorMessage);
		emit sigLoggedIn(false);
		return;
	}

	const LoginStuff loginInfo = loginThread->getLoginStuff();
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

void Base::currentTrackChanged(const MetaData& md)
{
	Q_UNUSED(md)

	Playlist::Mode pl_mode = GetSetting(Set::PL_Mode);
	if( Playlist::Mode::isActiveAndEnabled(pl_mode.dynamic()))
	{
		m->trackChangedTimer->stop();
		m->trackChangedTimer->start(1000);
	}

	// scrobble
	if(GetSetting(Set::LFM_Active) && m->loggedIn)
	{
		int secs = GetSetting(Set::LFM_ScrobbleTimeSec);
		if(secs > 0)
		{
			m->scrobbleTimer->stop();
			m->scrobbleTimer->start(secs * 1000);
		}
	}
}


void Base::scrobble()
{
	if(!GetSetting(Set::LFM_Active) || !m->loggedIn) {
		return;
	}

	const MetaData md = PlayManager::instance()->currentTrack();
	if(md.title().isEmpty() || md.artist().isEmpty()){
		return;
	}

	spLog(Log::Debug, this) << "Scrobble " << md.title() << " by " << md.artist();

	auto* lfm_wa = new WebAccess();
	connect(lfm_wa, &WebAccess::sigResponse, this, &Base::scrobbleResponseReceived);
	connect(lfm_wa, &WebAccess::sigError, this, &Base::scrobbleErrorReceived);

	time_t rawtime = time(nullptr);
	struct tm* ptm = localtime(&rawtime);
	time_t started = mktime(ptm);

	UrlParams sigData;
	if(!md.album().isEmpty()) {
		sigData["album"] =		md.album().toLocal8Bit();
	}
	sigData["api_key"] =	LFM_API_KEY;
	sigData["artist"] =		md.artist().toLocal8Bit();
	sigData["duration"] =	QString::number(md.durationMs() / 1000).toLocal8Bit();
	sigData["method"] =		"track.scrobble";
	sigData["sk"] =			m->sessionKey.toLocal8Bit();
	sigData["timestamp"] =	QString::number(started).toLocal8Bit();
	sigData["track"] =		md.title().toLocal8Bit();

	sigData.appendSignature();

	QByteArray post_data;
	QString url = lfm_wa->createPostUrl("http://ws.audioscrobbler.com/2.0/", sigData, post_data);

	lfm_wa->callPostUrl(url, post_data);
}

void Base::scrobbleResponseReceived(const QByteArray& data) {	Q_UNUSED(data) }

void Base::scrobbleErrorReceived(const QString& error)
{
	spLog(Log::Warning, this) << "Scrobble: " << error;
}

void Base::trackChangedTimerTimedOut()
{
	MetaData md = PlayManager::instance()->currentTrack();

	if(md.radioMode() == RadioMode::Off)
	{
		m->trackChangeThread->searchSimilarArtists(md);
	}

	if(GetSetting(Set::LFM_Active) && m->loggedIn)
	{
		m->trackChangeThread->updateNowPlaying(m->sessionKey, md);
	}
}

// private slot
void Base::similarArtistsFetched(IdList artistIds)
{
	if(artistIds.isEmpty()){
		return;
	}

	auto* db = DB::Connector::instance();
	DB::LibraryDatabase* lib_db = db->libraryDatabase(-1, 0);

	auto* plh = Playlist::Handler::instance();

	int active_idx = plh->activeIndex();
	PlaylistConstPtr active_playlist = plh->playlist(active_idx);

	if(!active_playlist){
		return;
	}

	const MetaDataList& v_md = active_playlist->tracks();

	Util::Algorithm::shuffle(artistIds);

	for( auto it=artistIds.begin(); it != artistIds.end(); it++ )
	{
		MetaDataList artist_tracks;
		{
			lib_db->getAllTracksByArtist(IdList{*it}, artist_tracks);
			Util::Algorithm::shuffle(artist_tracks);
		}

		// try all songs of artist
		for(int rounds=0; rounds < artist_tracks.count(); rounds++)
		{
			int index = RandomGenerator::getRandomNumber(0, int(artist_tracks.size()) - 1);

			MetaData md = artist_tracks.takeAt(index);

			// two times the same track is not allowed
			bool track_exists = Algorithm::contains(v_md, [md](const MetaData& it_md) {
				return (md.id() == it_md.id());
			});

			if(!track_exists)
			{
				MetaDataList v_md; v_md << md;

				plh->appendTracks(v_md, active_idx);
				return;
			}
		}
	}
}

