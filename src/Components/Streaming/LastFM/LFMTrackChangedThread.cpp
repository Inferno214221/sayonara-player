/* LFMTrackChangedThread.cpp

 * Copyright (C) 2011-2017 Lucio Carreras
 *
 * This file is part of sayonara-player
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * created by Lucio Carreras,
 * Jul 18, 2012
 *
 */

#include "LFMTrackChangedThread.h"
#include "LFMWebAccess.h"
#include "LFMGlobals.h"
#include "LFMSimArtistsParser.h"
#include "ArtistMatch.h"

#include "Database/Connector.h"
#include "Database/LibraryDatabase.h"

#include "Utils/Utils.h"
#include "Utils/FileUtils.h"
#include "Utils/MetaData/Artist.h"
#include "Utils/MetaData/MetaData.h"
#include "Utils/Compressor/Compressor.h"
#include "Utils/Logger/Logger.h"

#ifdef SMART_COMPARE
	#include "Utils/SmartCompare/SmartCompare.h"
#endif

#include <QMap>
#include <QStringList>
#include <QUrl>
#include <QHash>

using namespace LastFM;

struct TrackChangedThread::Private
{
	QString						artist;

	QHash<QString, ArtistMatch>  sim_artists_cache;
	MetaData					md;

#ifdef SMART_COMPARE
	SmartCompare*				_smart_comparison=nullptr;
#endif
};

TrackChangedThread::TrackChangedThread(QObject* parent) :
	QObject(parent)
{
	m = Pimpl::make<TrackChangedThread::Private>();

	ArtistList artists;
	DB::Connector* db = DB::Connector::instance();
	DB::LibraryDatabase* lib_db = db->library_db(-1, 0);

	lib_db->getAllArtists(artists);

#ifdef SMART_COMPARE
	_smart_comparison = new SmartCompare(artists);
#endif

}

TrackChangedThread::~TrackChangedThread() {}

void TrackChangedThread::update_now_playing(const QString& session_key, const MetaData& md)
{
	m->md = md;

	WebAccess* lfm_wa = new WebAccess();

	connect(lfm_wa, &WebAccess::sig_response, this, &TrackChangedThread::response_update);
	connect(lfm_wa, &WebAccess::sig_error, this, &TrackChangedThread::error_update);

	QString artist = m->md.artist();
	QString title = m->md.title();

	if(artist.trimmed().size() == 0) artist = "Unknown";
	artist.replace("&", "&amp;");

	UrlParams sig_data;
	sig_data["api_key"] = LFM_API_KEY;
	sig_data["artist"] = artist.toLocal8Bit();
	sig_data["duration"] = QString::number(m->md.length_ms / 1000).toLocal8Bit();
	sig_data["method"] = QString("track.updatenowplaying").toLocal8Bit();
	sig_data["sk"] = session_key.toLocal8Bit();
	sig_data["track"] =  title.toLocal8Bit();

	sig_data.append_signature();


	QByteArray post_data;
	QString url = lfm_wa->create_std_url_post(
				QString("http://ws.audioscrobbler.com/2.0/"),
				sig_data,
				post_data);

	lfm_wa->call_post_url(url, post_data);
}


void TrackChangedThread::response_update(const QByteArray& data)
{
	Q_UNUSED(data)
	WebAccess* lfm_wa = static_cast<WebAccess*>(sender());
	lfm_wa->deleteLater();
}


void TrackChangedThread::error_update(const QString& error){
	WebAccess* lfm_wa = static_cast<WebAccess*>(sender());

	sp_log(Log::Warning, this) << "Last.fm: Cannot update track";
	sp_log(Log::Warning, this) << "Last.fm: " << error;

	lfm_wa->deleteLater();
}


void TrackChangedThread::search_similar_artists(const MetaData& md)
{
	if(md.db_id() != 0) {
		return;
	}

	if(md.radio_mode() == RadioMode::Station){
		return;
	}

	m->md = md;

	if(m->md.artist().trimmed().isEmpty()){
		return;
	}

	// check if already in cache
	if(m->sim_artists_cache.contains(m->md.artist()))
	{
		const ArtistMatch& artist_match = m->sim_artists_cache.value(m->md.artist());
		evaluate_artist_match(artist_match);
		return;
	}

	m->artist = m->md.artist();

	WebAccess* lfm_wa = new WebAccess();

	connect(lfm_wa, &WebAccess::sig_response, this, &TrackChangedThread::response_sim_artists);
	connect(lfm_wa, &WebAccess::sig_error, this, &TrackChangedThread::error_sim_artists);

	QString url = 	QString("http://ws.audioscrobbler.com/2.0/?");
	QString encoded = QUrl::toPercentEncoding( m->md.artist() );
	url += QString("method=artist.getsimilar&");
	url += QString("artist=") + encoded + QString("&");
	url += QString("api_key=") + LFM_API_KEY;

	lfm_wa->call_url(url);
	return;
}


void TrackChangedThread::evaluate_artist_match(const ArtistMatch& artist_match)
{
	if(!artist_match.is_valid()){
		return;
	}

	QByteArray arr = Compressor::compress(artist_match.to_string().toLocal8Bit());
	Util::File::create_directories(Util::sayonara_path() + "/similar_artists/");
	Util::File::write_file(arr, Util::sayonara_path() + "/similar_artists/" + artist_match.get_artist_name() + ".comp");

	// if we always take the best, it's boring
	ArtistMatch::Quality quality, quality_org;

	int rnd_number = Util::random_number(1, 999);

	if(rnd_number > 350) {
		quality = ArtistMatch::Quality::Very_Good;	// [250-999]
	}

	else if(rnd_number > 75){
		quality = ArtistMatch::Quality::Well;		// [50-250]
	}

	else {
		quality = ArtistMatch::Quality::Poor;
	}

	quality_org = quality;
	QMap<QString, int> possible_artists;

	while(possible_artists.isEmpty())
	{
		possible_artists = filter_available_artists(artist_match, quality);

		switch(quality){
			case ArtistMatch::Quality::Poor:
				quality = ArtistMatch::Quality::Very_Good;
				break;
			case ArtistMatch::Quality::Well:
				quality = ArtistMatch::Quality::Poor;
				break;
			case ArtistMatch::Quality::Very_Good:
				quality = ArtistMatch::Quality::Well;
				break;
			default: // will never be executed
				quality = quality_org;
				break;
		}

		if(quality == quality_org) {
			break;
		}
	}

	if(possible_artists.isEmpty()){
		return;
	}

	IdList chosen_ids;
	for(auto it = possible_artists.begin(); it != possible_artists.end(); it++) {
		chosen_ids << it.value();
	}

	emit sig_similar_artists_available(chosen_ids);
}


QMap<QString, int> TrackChangedThread::filter_available_artists(const ArtistMatch& artist_match, ArtistMatch::Quality quality)
{
	QMap<ArtistMatch::ArtistDesc, double> bin = artist_match.get(quality);
	QMap<QString, int> possible_artists;

	DB::Connector* db = DB::Connector::instance();
	DB::LibraryDatabase* lib_db = db->library_db(m->md.library_id, 0);

	for(auto it=bin.cbegin(); it!=bin.cend(); it++)
	{
		ArtistId artist_id = lib_db->getArtistID(it.key().artist_name);
		if(artist_id >= 0 )
		{
			possible_artists[it.key().artist_name] = artist_id;
		}
	}

	return possible_artists;
}


void TrackChangedThread::response_sim_artists(const QByteArray& data)
{
	SimArtistsParser parser(m->artist, data);

	ArtistMatch artist_match = parser.artist_match();

	if(artist_match.is_valid()){
		QString artist_name = artist_match.get_artist_name();
		m->sim_artists_cache[artist_name] = artist_match;
	}

	evaluate_artist_match(artist_match);
}

void TrackChangedThread::error_sim_artists(const QString& error)
{
	sp_log(Log::Warning, this) << "Last.fm: Search similar artists" << error;
}
