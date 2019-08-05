/* SoundcloudLibrary.cpp */

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

#include "SoundcloudLibrary.h"
#include "SoundcloudDataFetcher.h"
#include "SoundcloudGlobal.h"
#include "SearchInformation.h"
#include "Sorting.h"

#include "Components/Covers/CoverLookup.h"
#include "Components/Covers/CoverLocation.h"

#include "Utils/globals.h"
#include "Utils/MetaData/Album.h"
#include "Utils/MetaData/Artist.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Set.h"
#include "Utils/typedefs.h"

#include <QHash>

struct SC::Library::Private
{
	QHash<int, int>           md_id_idx_map;
	QHash<int, IndexSet>      md_artist_id_idx_map;
	QHash<int, IndexSet>      md_album_id_idx_map;
	QHash<QString, IndexSet>  md_name_idx_map;

	QHash<int, int>           album_id_idx_map;
	QHash<QString, IndexSet>  album_name_idx_map;
	QHash<QString, IndexSet>  artist_name_album_idx_map;

	QHash<int, int>         artist_id_idx_map;
	QHash<QString, IndexSet>  artist_name_idx_map;

	MetaDataList    v_md;
	AlbumList       albums;
	ArtistList      artists;

	SC::Database*           scd=nullptr;
	SearchInformationList	search_information;

	Private()
	{
		scd = new SC::Database();
	}

	~Private()
	{
		scd->close_db();
		delete scd;
	}

	void clear_cache()
	{
		v_md.clear();
		albums.clear();
		artists.clear();
		search_information.clear();

		md_id_idx_map.clear();
		md_artist_id_idx_map.clear();
		md_album_id_idx_map.clear();
		md_name_idx_map.clear();
		album_id_idx_map.clear();
		album_name_idx_map.clear();
		artist_name_album_idx_map.clear();
		artist_id_idx_map.clear();
		artist_name_idx_map.clear();
	}
};

SC::Library::Library(QObject *parent) :
	AbstractLibrary(parent)
{
	m = Pimpl::make<Private>();
}

SC::Library::~Library() {}

void SC::Library::load()
{
	AbstractLibrary::load();

	ArtistList artists;
	get_all_artists(artists);
}


void SC::Library::get_all_artists(ArtistList& artists)
{
	if(m->artists.empty())
	{
		m->scd->getAllArtists(artists, false);
		m->artists = artists;

		for(int i=0; i<m->artists.count(); i++)
		{
			const Artist& artist = artists[i];
			m->artist_id_idx_map[artist.id] = i;
			m->artist_name_idx_map[artist.name()].insert(i);
		}
	}

	else {
		artists = m->artists;
	}

	artists.sort(sortorder().so_artists);
}

void SC::Library::get_all_artists_by_searchstring(::Library::Filter filter, ArtistList& artists)
{
	if(filter.mode() != ::Library::Filter::Mode::Fulltext){
		return;
	}

	if(m->search_information.is_empty()){
		m->scd->getSearchInformation(m->search_information);
	}

	QStringList filtertexts = filter.filtertext(false);
	for(const QString& filtertext : filtertexts)
	{
		IntSet artist_ids = m->search_information.artist_ids(filtertext);

		for(int artist_id : artist_ids)
		{
			int idx = m->artist_id_idx_map[artist_id];

			Artist artist = m->artists[idx];
			artist.num_songs = m->md_artist_id_idx_map[artist_id].count();
			if(!artists.contains(artist.id)){
				artists << artist;
			}
		}
	}

	artists.sort(sortorder().so_artists);
}

void SC::Library::get_all_albums(AlbumList& albums)
{
	if(m->albums.empty())
	{
		m->scd->getAllAlbums(albums, false);
		m->albums = albums;

		for(int i=0; i<albums.count(); i++)
		{
			const Album& album = albums[i];
			m->album_id_idx_map[album.id] = i;
			m->album_name_idx_map[album.name()].insert(i);

			const QStringList artists = album.artists();
			for(const QString& artist : artists)
			{
				m->artist_name_album_idx_map[artist].insert(i);
			}
		}
	}

	else {
		albums = m->albums;
	}

	albums.sort(sortorder().so_albums);
}

void SC::Library::get_all_albums_by_artist(IdList artist_ids, AlbumList& albums, ::Library::Filter filter)
{
	Q_UNUSED(filter)

	for(int artist_id : artist_ids)
	{
		int artist_idx = m->artist_id_idx_map[artist_id];
		const Artist& artist = m->artists[artist_idx];

		IndexSet album_idxs = m->artist_name_album_idx_map[artist.name()];

		for(int album_idx : album_idxs)
		{
			if(!Util::between(album_idx, m->albums)){
				sp_log(Log::Warning, this) << __FUNCTION__ << " Invalid index: " << album_idx << " (" << m->albums.size() << ")";
			}
			else {
				albums.push_back(m->albums[album_idx]);
			}
		}
	}

	albums.sort(sortorder().so_albums);
}

void SC::Library::get_all_albums_by_searchstring(::Library::Filter filter, AlbumList& albums)
{
	if(filter.mode() != ::Library::Filter::Mode::Fulltext){
		return;
	}

	if(m->search_information.is_empty()){
		m->scd->getSearchInformation(m->search_information);
	}

	QStringList filtertexts = filter.filtertext(false);
	for(const QString& filtertext : filtertexts)
	{
		IntSet album_ids = m->search_information.album_ids(filtertext);
		for(int album_id : album_ids)
		{
			int idx = m->album_id_idx_map[album_id];
			if(!Util::between(idx, m->albums)) {
				sp_log(Log::Warning, this) << __FUNCTION__ << " Invalid index: " << idx << " (" << m->albums.size() << ")";
				continue;
			}

			if(albums.contains(m->albums[idx].id))
			{
				albums << m->albums[idx];
			}
		}
	}

	albums.sort(sortorder().so_albums);
}

bool SC::Library::is_empty() const
{
	return false;
}

void SC::Library::get_all_tracks(const QStringList& paths, MetaDataList& v_md)
{
	Q_UNUSED(paths)
	Q_UNUSED(v_md)
	return;
}

void SC::Library::get_all_tracks(MetaDataList& v_md)
{
	if(m->v_md.isEmpty())
	{
		m->scd->getAllTracks(v_md);
		m->v_md = v_md;

		for(int i=0; i<m->v_md.count(); i++)
		{
			const MetaData& md = v_md[i];

			m->md_id_idx_map[md.id] = i;
			m->md_name_idx_map[md.title()].insert(i);
			m->md_album_id_idx_map[md.album_id].insert(i);
			m->md_artist_id_idx_map[md.artist_id].insert(i);
		}
	}

	else {
		v_md = m->v_md;
	}

	v_md.sort(sortorder().so_tracks);
}

void SC::Library::get_all_tracks_by_artist(IdList artist_ids, MetaDataList& v_md, ::Library::Filter filter)
{
	Q_UNUSED(filter)

	for(int artist_id : artist_ids)
	{
		const IndexSet& idxs = m->md_artist_id_idx_map[artist_id];

		for(int idx : idxs)
		{
			if(!Util::between(idx, m->v_md)) {
				sp_log(Log::Warning, this) << __FUNCTION__ << " Invalid index: " << idx << " (" << m->v_md.size() << ")";
			} else {
				v_md << m->v_md[idx];
			}
		}
	}

	v_md.sort(sortorder().so_tracks);
}

void SC::Library::get_all_tracks_by_album(IdList album_ids, MetaDataList& v_md, ::Library::Filter filter)
{
	Q_UNUSED(filter)

	for(int album_id : album_ids)
	{
		const IndexSet& idxs = m->md_album_id_idx_map[album_id];
		for(int idx : idxs) {
			v_md << m->v_md[idx];
		}
	}

	v_md.sort(sortorder().so_tracks);
}

void SC::Library::get_all_tracks_by_searchstring(::Library::Filter filter, MetaDataList& v_md)
{
	if(filter.mode() != ::Library::Filter::Mode::Fulltext){
		return;
	}

	if(m->search_information.is_empty()){
		m->scd->getSearchInformation(m->search_information);
	}

	QStringList filtertexts = filter.filtertext(false);
	for(const QString& filtertext : filtertexts)
	{
		IntSet track_ids = m->search_information.track_ids(filtertext);

		for(int track_id : track_ids)
		{
			int idx = m->md_id_idx_map[track_id];
			if(!v_md.contains(m->v_md[idx].id))
			{
				v_md << m->v_md[idx];
			}
		}
	}

	v_md.sort(sortorder().so_tracks);
}

void SC::Library::update_track(const MetaData& md)
{
	m->scd->updateTrack(md);
	refetch();
}

void SC::Library::update_album(const Album& album)
{
	m->scd->updateAlbum(album);
	refetch();
}

void SC::Library::delete_tracks(const MetaDataList& v_md, ::Library::TrackDeletionMode mode)
{
	Q_UNUSED(mode)

	m->scd->deleteTracks(v_md);
	refetch();
}

void SC::Library::refetch()
{
	m->clear_cache();

	AbstractLibrary::refetch();

	m->scd->getSearchInformation(m->search_information);
}

void SC::Library::reload_library(bool b, ::Library::ReloadQuality quality)
{
	Q_UNUSED(b)
	Q_UNUSED(quality)

	m->clear_cache();
}

void SC::Library::refresh_artist()
{
	if(selected_artists().isEmpty()){
		return;
	}

	ArtistId artist_id = selected_artists().first();

	MetaDataList v_md;
	get_all_tracks_by_artist({artist_id}, v_md, ::Library::Filter());
	delete_tracks(v_md, ::Library::TrackDeletionMode::None);

	sp_log(Log::Debug, this) << "Deleted " << v_md.size() << " soundcloud tracks";

	SC::DataFetcher* fetcher = new SC::DataFetcher(this);

	connect(fetcher, &SC::DataFetcher::sig_artists_fetched,
			this, &SC::Library::artists_fetched);

	fetcher->get_artist(artist_id);
}


void SC::Library::refresh_albums() {}

void SC::Library::refresh_tracks() {}

void SC::Library::cover_found(const Cover::Location& cl)
{
	Q_UNUSED(cl)
	//sp_log(Log::Debug, this) << "Saved sound cloud cover: " << cl.toString();
}


//void SC::Library::insert_tracks(const MetaDataList& v_md)
//{
//	Q_UNUSED(v_md)
//	return;
//}

void SC::Library::insert_tracks(const MetaDataList& v_md, const ArtistList& artists, const AlbumList& albums)
{
	Artist artist_tmp;
	Album album_tmp;

	for(const Artist& artist : artists)
	{
		if(!m->scd->getArtistByID(artist.id, artist_tmp) || artist.id != artist_tmp.id) {
			m->scd->insertArtistIntoDatabase(artist);
		}
	}

	for(const Album& album : albums)
	{
		if(!m->scd->getAlbumByID(album.id, album_tmp) || album.id != album_tmp.id) {
			m->scd->insertAlbumIntoDatabase(album);
		}

		else {
			album_tmp.print();
		}
	}

	if(!m->scd->getAlbumByID(-1, album_tmp))
	{
		Album album;
		album.set_name("None");
		album.id = 0;

		m->scd->insertAlbumIntoDatabase(album);
	}

	m->scd->store_metadata(v_md);

	AbstractLibrary::refresh_current_view();

	refetch();
}



void SC::Library::artists_fetched(const ArtistList& artists)
{
	for(const Artist& artist : artists)
	{
		sp_log(Log::Debug, this) << "Artist " << artist.name() << " fetched";

		SC::DataFetcher* fetcher;

		if(artist.id <= 0) {
			continue;
		}

		m->scd->updateArtist(artist);

		fetcher = new SC::DataFetcher(this);

		connect(fetcher, &SC::DataFetcher::sig_playlists_fetched,
				this, &SC::Library::albums_fetched);

		connect(fetcher, &SC::DataFetcher::sig_tracks_fetched,
				this, &SC::Library::tracks_fetched);

		fetcher->get_tracks_by_artist(artist.id);
	}

	sender()->deleteLater();
	refetch();
}

void SC::Library::tracks_fetched(const MetaDataList& v_md)
{
	for(const MetaData& md : v_md){
		if(md.id > 0){
			m->scd->insertTrackIntoDatabase(md, md.artist_id, md.album_id);
		}
	}

	sender()->deleteLater();
	refetch();
}

void SC::Library::albums_fetched(const AlbumList& albums)
{
	for(const Album& album : albums){
		if(album.id > 0){
			m->scd->insertAlbumIntoDatabase(album);
		}
	}

	sender()->deleteLater();
	refetch();
}


void SC::Library::get_track_by_id(TrackID track_id, MetaData& md)
{
	Q_UNUSED(track_id)
	Q_UNUSED(md)
}

void SC::Library::get_artist_by_id(ArtistId artist_id, Artist& artist)
{
	Q_UNUSED(artist_id)
	Q_UNUSED(artist)
}


void SC::Library::get_album_by_id(AlbumId album_id, Album& album)
{
	Q_UNUSED(album_id)
	Q_UNUSED(album)
}
