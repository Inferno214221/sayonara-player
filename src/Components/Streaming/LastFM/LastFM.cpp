/* LastFM.cpp */

/* Copyright (C) 2011-2019 Lucio Carreras
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
 *      Author: Lucio Carreras
 */

#include "LastFM.h"
#include "LFMGlobals.h"
#include "LFMTrackChangedThread.h"
#include "LFMLoginThread.h"
#include "LFMWebAccess.h"

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

#include <ctime>

namespace Algorithm=Util::Algorithm;

using namespace LastFM;

struct Base::Private
{
	MetaData					md;

	QString						session_key;

	Seconds						old_pos;
	Seconds						old_pos_difference;

	TrackChangedThread*			track_changed_thread=nullptr;
	PlayManagerPtr				play_manager=nullptr;

	bool						logged_in;
	bool						scrobbled;


	Private(QObject* parent) :
		track_changed_thread(new TrackChangedThread(parent)),
		play_manager(PlayManager::instance()),
		logged_in(false),
		scrobbled(false)
	{}
};

Base::Base() :
	QObject()
{
	m = Pimpl::make<Private>(this);

	connect(m->play_manager, &PlayManager::sig_track_changed,	this, &Base::current_track_changed);
	connect(m->play_manager, &PlayManager::sig_position_changed_ms, this, &Base::position_ms_changed);
	connect(m->track_changed_thread, &TrackChangedThread::sig_similar_artists_available,
			this, &Base::similar_artists_fetched);

	ListenSetting(Set::LFM_Active, Base::lfm_active_changed);
}

Base::~Base() {}


bool Base::is_logged_in()
{
	return m->logged_in;
}

void Base::login(const QString& username, const QString& password)
{
	LoginThread* login_thread = new LoginThread(this);
	connect(login_thread, &LoginThread::sig_logged_in, this, &Base::login_thread_finished);
	connect(login_thread, &LoginThread::sig_error, this, [=](const QString& error_message){
		sp_log(Log::Warning, this) << error_message;
		emit sig_logged_in(false);
	});

	login_thread->login(username, password);
}


void Base::lfm_active_changed()
{
	m->logged_in = false;

	bool active = GetSetting(Set::LFM_Active);
	if(active)
	{
		QString username = GetSetting(Set::LFM_Username);
		QString password = Util::Crypt::decrypt(GetSetting(Set::LFM_Password));

		login(username, password);
	}
}

void Base::login_thread_finished(bool success)
{
	m->logged_in = success;

	if(!success){
		emit sig_logged_in(false);
		return;
	}

	LoginThread* login_thread = static_cast<LoginThread*>(sender());
	LoginStuff login_info = login_thread->getLoginStuff();

	m->logged_in = login_info.logged_in;
	m->session_key = login_info.session_key;

	SetSetting(Set::LFM_SessionKey, m->session_key);

	sp_log(Log::Debug, this) << "Got session key";

	if(!m->logged_in){
		sp_log(Log::Warning, this) << "Cannot login";
	}

	emit sig_logged_in(m->logged_in);

	sender()->deleteLater();
}

void Base::current_track_changed(const MetaData& md)
{
	Playlist::Mode pl_mode = GetSetting(Set::PL_Mode);
	if( Playlist::Mode::isActiveAndEnabled(pl_mode.dynamic()))
	{
		m->track_changed_thread->search_similar_artists(md);
	}

	bool active = GetSetting(Set::LFM_Active);
	if(!active || !m->logged_in) {
		return;
	}

	m->md = md;

	reset_scrobble();

	m->track_changed_thread->update_now_playing(m->session_key, md);
}


void Base::position_ms_changed(MilliSeconds pos_ms)
{
	bool active = GetSetting(Set::LFM_Active);
	if(!active){
		return;
	}

	check_scrobble(pos_ms);
}


void Base::reset_scrobble()
{
	m->scrobbled = false;
	m->old_pos = 0;
	m->old_pos_difference = 0;
}


bool Base::check_scrobble(MilliSeconds pos_ms)
{
	if(pos_ms < 0)
	{
		return false;
	}

	if(!m->logged_in){
		return false;
	}

	if(m->scrobbled){
		return false;
	}

	if(m->md.length_ms <= 0){
		return false;
	}

	if(m->old_pos == 0){
		m->old_pos = pos_ms;
		m->old_pos_difference = 0;
		return false;
	}

	else{
		if(m->old_pos > pos_ms){
			m->old_pos = 0;
		}

		else if(pos_ms > m->old_pos + 2000){
			m->old_pos = 0;
		}

		else{
			MilliSeconds scrobble_time_ms = (MilliSeconds) (GetSetting(Set::LFM_ScrobbleTimeSec) * 1000);

			m->old_pos_difference += (pos_ms - m->old_pos);
			m->old_pos = pos_ms;

			if( (m->old_pos_difference > scrobble_time_ms) ||
				(m->old_pos_difference >= ((m->md.length_ms  * 3) / 4) && m->md.length_ms >= 1000))
			{
				scrobble(m->md);
			}
		}
	}

	return m->scrobbled;
}

void Base::scrobble(const MetaData& md)
{
	m->scrobbled = true;

	bool active = GetSetting(Set::LFM_Active);
	if(!active) {
		return;
	}

	if(!m->logged_in){
		return;
	}

	WebAccess* lfm_wa = new WebAccess();
	connect(lfm_wa, &WebAccess::sig_response, this, &Base::scrobble_response_received);
	connect(lfm_wa, &WebAccess::sig_error, this, &Base::scrobble_error_received);

	time_t rawtime, started;
	rawtime = time(nullptr);
	struct tm* ptm = localtime(&rawtime);
	started = mktime(ptm);

	QString artist = md.artist();
	QString title = md.title();

	UrlParams sig_data;
	sig_data["api_key"] = LFM_API_KEY;
	sig_data["artist"] = artist.toLocal8Bit();
	sig_data["duration"] = QString::number(md.length_ms / 1000).toLocal8Bit();
	sig_data["method"] = "track.scrobble";
	sig_data["sk"] = m->session_key.toLocal8Bit();
	sig_data["timestamp"] = QString::number(started).toLocal8Bit();
	sig_data["track"] = title.toLocal8Bit();

	sig_data.append_signature();

	QByteArray post_data;
	QString url = lfm_wa->create_std_url_post("http://ws.audioscrobbler.com/2.0/", sig_data, post_data);

	lfm_wa->call_post_url(url, post_data);
}

void Base::scrobble_response_received(const QByteArray& data){
	Q_UNUSED(data)
}

void Base::scrobble_error_received(const QString& error){
	sp_log(Log::Warning, this) << "Scrobble: " << error;
}



// private slot
void Base::similar_artists_fetched(IdList artist_ids)
{
	if(artist_ids.isEmpty()){
		return;
	}

	DB::Connector* db = DB::Connector::instance();
	DB::LibraryDatabase* lib_db = db->library_db(-1, 0);

	Playlist::Handler* plh = Playlist::Handler::instance();

	int active_idx = plh->active_index();
	PlaylistConstPtr active_playlist = plh->playlist(active_idx);
	const MetaDataList& v_md = active_playlist->tracks();

	std::random_shuffle(artist_ids.begin(), artist_ids.end());

	for( auto it=artist_ids.begin(); it != artist_ids.end(); it++ )
	{
		MetaDataList artist_tracks;
		lib_db->getAllTracksByArtist(IdList{*it}, artist_tracks);

		std::random_shuffle(artist_tracks.begin(), artist_tracks.end());

		// try all songs of artist
		for(int rounds=0; rounds < artist_tracks.count(); rounds++)
		{
			int rnd_track = RandomGenerator::get_random_number(0, artist_tracks.size()- 1);

			MetaData md = artist_tracks.take_at(rnd_track);

			// two times the same track is not allowed
			bool track_exists = Algorithm::contains(v_md, [md](const MetaData& it_md){
				return (md.id == it_md.id);
			});

			if(!track_exists){
				MetaDataList v_md; v_md << md;

				plh->append_tracks(v_md, active_idx);
				return;
			}
		}
	}
}

