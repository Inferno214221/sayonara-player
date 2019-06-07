/* SoundcloudDataFetcher.cpp */

/* Copyright (C) 2011-2019  Lucio Carreras
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

#include "SoundcloudDataFetcher.h"
#include "SoundcloudWebAccess.h"
#include "SoundcloudJsonParser.h"

#include "Utils/WebAccess/AsyncWebAccess.h"

#include "Utils/MetaData/MetaDataList.h"
#include "Utils/MetaData/Album.h"
#include "Utils/MetaData/Artist.h"


struct SC::DataFetcher::Private
{
	MetaDataList	playlist_tracks;
	AlbumList		playlists;
	ArtistList		artists;
	int				artist_id;

	Private()
	{
		artist_id = -1;
	}
};


SC::DataFetcher::DataFetcher(QObject* parent) :
	QObject(parent)
{
	m = Pimpl::make<SC::DataFetcher::Private>();

	clear();
}

SC::DataFetcher::~DataFetcher() {}


void SC::DataFetcher::search_artists(const QString& artist_name)
{
	clear();

	AsyncWebAccess* awa = new AsyncWebAccess(this);
	connect(awa, &AsyncWebAccess::sig_finished,
			this, &SC::DataFetcher::artists_fetched);
	awa->run( SC::WebAccess::create_dl_get_artist(artist_name));
}

void SC::DataFetcher::get_artist(int artist_id)
{
	clear();

	AsyncWebAccess* awa = new AsyncWebAccess(this);
	connect(awa, &AsyncWebAccess::sig_finished,
			this, &SC::DataFetcher::artists_fetched);

	awa->run( SC::WebAccess::create_dl_get_artist(artist_id) );
}


void SC::DataFetcher::get_tracks_by_artist(int artist_id)
{
	clear();

	m->artist_id = artist_id;

	AsyncWebAccess* awa = new AsyncWebAccess(this);
	connect(awa, &AsyncWebAccess::sig_finished,
			this, &SC::DataFetcher::playlist_tracks_fetched);

	awa->run( SC::WebAccess::create_dl_get_playlists(artist_id) );
}


void SC::DataFetcher::artists_fetched()
{
	ArtistList artists;
	AsyncWebAccess* awa = static_cast<AsyncWebAccess*>(sender());
	if(awa->status() != AsyncWebAccess::Status::GotData) {
		awa->deleteLater();
		return;
	}

	QByteArray data = awa->data();
	SC::JsonParser parser(data);
	parser.parse_artists(artists);

	emit sig_artists_fetched(artists);
	awa->deleteLater();
}


void SC::DataFetcher::playlist_tracks_fetched()
{
	AsyncWebAccess* awa = static_cast<AsyncWebAccess*>(sender());

	if(awa->status() != AsyncWebAccess::Status::GotData) {
		awa->deleteLater();
		return;
	}

	QByteArray data = awa->data();
	SC::JsonParser parser(data);
	parser.parse_playlists(m->artists, m->playlists, m->playlist_tracks);

	AsyncWebAccess* awa_new = new AsyncWebAccess(this);
	connect(awa_new, &AsyncWebAccess::sig_finished,
			this, &SC::DataFetcher::tracks_fetched);

	awa_new->run( SC::WebAccess::create_dl_get_tracks(m->artist_id) );

	awa->deleteLater();
}

void SC::DataFetcher::tracks_fetched()
{
	MetaDataList v_md;
	ArtistList artists;
	AsyncWebAccess* awa = static_cast<AsyncWebAccess*>(sender());

	if(awa->status() != AsyncWebAccess::Status::GotData) {
		awa->deleteLater();
		return;
	}

	QByteArray data = awa->data();
	SC::JsonParser parser(data);
	parser.parse_tracks(artists, v_md);

	for(const MetaData& md : v_md){
		if(!m->playlist_tracks.contains(md.id)){
			m->playlist_tracks << md;
		}
	}

	for(const Artist& artist : artists){
		if(!m->artists.contains(artist.id)){
			m->artists << artist;
		}
	}

	emit sig_playlists_fetched(m->playlists);
	emit sig_tracks_fetched(m->playlist_tracks);
	emit sig_ext_artists_fetched(m->artists);

	awa->deleteLater();
}

void SC::DataFetcher::clear()
{
	m->playlist_tracks.clear();
	m->playlists.clear();
	m->artists.clear();
	m->artist_id = -1;
}

