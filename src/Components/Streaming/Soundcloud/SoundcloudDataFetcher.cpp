/* SoundcloudDataFetcher.cpp */

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

#include "SoundcloudDataFetcher.h"
#include "SoundcloudWebAccess.h"
#include "SoundcloudJsonParser.h"

#include "Utils/WebAccess/AsyncWebAccess.h"

#include "Utils/MetaData/MetaDataList.h"
#include "Utils/MetaData/Album.h"
#include "Utils/MetaData/Artist.h"


struct SC::DataFetcher::Private
{
	MetaDataList	playlistTracks;
	AlbumList		playlists;
	ArtistList		artists;
	int				artistId;

	Private() : artistId(-1) {}
};


SC::DataFetcher::DataFetcher(QObject* parent) :
	QObject(parent)
{
	m = Pimpl::make<SC::DataFetcher::Private>();

	clear();
}

SC::DataFetcher::~DataFetcher() = default;

void SC::DataFetcher::searchArtists(const QString& artist_name)
{
	clear();

	auto* awa = new AsyncWebAccess(this);
	connect(awa, &AsyncWebAccess::sigFinished,
			this, &SC::DataFetcher::artistsFetched);
	awa->run( SC::WebAccess::createLinkGetArtist(artist_name));
}

void SC::DataFetcher::getArtist(int artistId)
{
	clear();

	auto* awa = new AsyncWebAccess(this);
	connect(awa, &AsyncWebAccess::sigFinished,
			this, &SC::DataFetcher::artistsFetched);

	awa->run( SC::WebAccess::createLinkGetArtist(artistId) );
}


void SC::DataFetcher::getTracksByArtist(int artistId)
{
	clear();

	m->artistId = artistId;

	auto* awa = new AsyncWebAccess(this);
	connect(awa, &AsyncWebAccess::sigFinished,
			this, &SC::DataFetcher::playlistTracksFetched);

	awa->run( SC::WebAccess::createLinkGetPlaylists(artistId) );
}


void SC::DataFetcher::artistsFetched()
{
	auto* awa = static_cast<AsyncWebAccess*>(sender());
	if(awa->status() != AsyncWebAccess::Status::GotData) {
		awa->deleteLater();
		return;
	}

	QByteArray data = awa->data();
	SC::JsonParser parser(data);

	ArtistList artists;
	parser.parseArtists(artists);

	emit sigArtistsFetched(artists);
	awa->deleteLater();
}


void SC::DataFetcher::playlistTracksFetched()
{
	auto* awa = static_cast<AsyncWebAccess*>(sender());
	if(awa->status() != AsyncWebAccess::Status::GotData) {
		awa->deleteLater();
		return;
	}

	QByteArray data = awa->data();
	SC::JsonParser parser(data);
	parser.parsePlaylists(m->artists, m->playlists, m->playlistTracks);

	AsyncWebAccess* awa_new = new AsyncWebAccess(this);
	connect(awa_new, &AsyncWebAccess::sigFinished,
			this, &SC::DataFetcher::tracksFetched);

	awa_new->run( SC::WebAccess::createLinkGetTracks(m->artistId) );

	awa->deleteLater();
}

void SC::DataFetcher::tracksFetched()
{
	auto* awa = static_cast<AsyncWebAccess*>(sender());
	if(awa->status() != AsyncWebAccess::Status::GotData) {
		awa->deleteLater();
		return;
	}

	QByteArray data = awa->data();
	SC::JsonParser parser(data);

	MetaDataList tracks;
	ArtistList artists;
	parser.parseTracks(artists, tracks);

	for(const MetaData& md : tracks)
	{
		if(!m->playlistTracks.contains(md.id())){
			m->playlistTracks << md;
		}
	}

	for(const Artist& artist : artists)
	{
		if(!m->artists.contains(artist.id())){
			m->artists << artist;
		}
	}

	emit sigPlaylistsFetched(m->playlists);
	emit sigTracksFetched(m->playlistTracks);
	emit sigExtArtistsFetched(m->artists);

	awa->deleteLater();
}

void SC::DataFetcher::clear()
{
	m->playlistTracks.clear();
	m->playlists.clear();
	m->artists.clear();
	m->artistId = -1;
}

