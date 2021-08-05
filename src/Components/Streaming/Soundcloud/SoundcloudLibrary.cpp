/* SoundcloudLibrary.cpp */

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

#include "SoundcloudLibrary.h"
#include "SoundcloudLibraryDatabase.h"
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
	QHash<int, int> mdIdIndexMap;
	QHash<int, IndexSet> mdArtistIdIndexMap;
	QHash<int, IndexSet> mdAlbumIdIndexMap;
	QHash<QString, IndexSet> mdNameIndexMap;

	QHash<int, int> albumIdIndexMap;
	QHash<QString, IndexSet> albumNameIndexMap;
	QHash<QString, IndexSet> artistNameAlbumIndexMap;

	QHash<int, int> artistIdIndexMap;
	QHash<QString, IndexSet> artistNameIndexMap;

	MetaDataList tracks;
	AlbumList albums;
	ArtistList artists;

	SC::Database* db = nullptr;
	SC::LibraryDatabase* libraryDatabase = nullptr;
	SearchInformationList search_information;

	Private()
	{
		db = new SC::Database();
		libraryDatabase = new SC::LibraryDatabase(db->connectionName(), db->databaseId(), -1);
	}

	~Private()
	{
		db->closeDatabase();
		delete db;
		db = nullptr;
		delete libraryDatabase;
		libraryDatabase = nullptr;
	}

	void clear_cache()
	{
		tracks.clear();
		albums.clear();
		artists.clear();
		search_information.clear();

		mdIdIndexMap.clear();
		mdArtistIdIndexMap.clear();
		mdAlbumIdIndexMap.clear();
		mdNameIndexMap.clear();
		albumIdIndexMap.clear();
		albumNameIndexMap.clear();
		artistNameAlbumIndexMap.clear();
		artistIdIndexMap.clear();
		artistNameIndexMap.clear();
	}
};

SC::Library::Library(LibraryPlaylistInteractor* playlistInteractor, QObject* parent) :
	AbstractLibrary(playlistInteractor, parent)
{
	m = Pimpl::make<Private>();
}

SC::Library::~Library() = default;

void SC::Library::load()
{
	AbstractLibrary::load();

	ArtistList artists;
	getAllArtists(artists);
}

void SC::Library::getAllArtists(ArtistList& artists) const
{
	if(m->artists.empty())
	{
		m->libraryDatabase->getAllArtists(artists, false);
		m->artists = artists;

		for(int i = 0; i < m->artists.count(); i++)
		{
			const Artist& artist = artists[size_t(i)];
			m->artistIdIndexMap[artist.id()] = i;
			m->artistNameIndexMap[artist.name()].insert(i);
		}
	}

	else
	{
		artists = m->artists;
	}

	artists.sort(sortorder().so_artists);
}

void SC::Library::getAllArtistsBySearchstring(::Library::Filter filter, ArtistList& artists) const
{
	if(filter.mode() != ::Library::Filter::Mode::Fulltext)
	{
		return;
	}

	if(m->search_information.isEmpty())
	{
		m->libraryDatabase->searchInformation(m->search_information);
	}

	QStringList filtertexts = filter.filtertext(false);
	for(const QString& filtertext : filtertexts)
	{
		IntSet artistIds = m->search_information.artistIds(filtertext);

		for(int artistId : artistIds)
		{
			int idx = m->artistIdIndexMap[artistId];

			Artist artist = m->artists[size_t(idx)];

			auto n_songs = uint16_t(m->mdArtistIdIndexMap[artistId].count());
			artist.setSongcount(n_songs);
			if(!artists.contains(artist.id()))
			{
				artists << artist;
			}
		}
	}

	artists.sort(sortorder().so_artists);
}

void SC::Library::getAllAlbums(AlbumList& albums) const
{
	if(m->albums.empty())
	{
		m->libraryDatabase->getAllAlbums(albums, false);
		m->albums = albums;

		for(int i = 0; i < albums.count(); i++)
		{
			const Album& album = albums[i];
			m->albumIdIndexMap[album.id()] = i;
			m->albumNameIndexMap[album.name()].insert(i);

			const QStringList artists = album.artists();
			for(const QString& artist : artists)
			{
				m->artistNameAlbumIndexMap[artist].insert(i);
			}
		}
	}

	else
	{
		albums = m->albums;
	}

	albums.sort(sortorder().so_albums);
}

void SC::Library::getAllAlbumsByArtist(IdList artistIds, AlbumList& albums, ::Library::Filter filter) const
{
	Q_UNUSED(filter)

	for(int artistId : artistIds)
	{
		int artistIdx = m->artistIdIndexMap[artistId];
		const Artist& artist = m->artists[size_t(artistIdx)];

		IndexSet albumIdxs = m->artistNameAlbumIndexMap[artist.name()];

		for(int albumIdx : albumIdxs)
		{
			if(!Util::between(albumIdx, m->albums))
			{
				spLog(Log::Warning, this) << __FUNCTION__ << " Invalid index: " << albumIdx << " (" << m->albums.size()
				                          << ")";
			}
			else
			{
				albums.push_back(m->albums[albumIdx]);
			}
		}
	}

	albums.sort(sortorder().so_albums);
}

void SC::Library::getAllAlbumsBySearchstring(::Library::Filter filter, AlbumList& albums) const
{
	if(filter.mode() != ::Library::Filter::Mode::Fulltext)
	{
		return;
	}

	if(m->search_information.isEmpty())
	{
		m->libraryDatabase->searchInformation(m->search_information);
	}

	QStringList filtertexts = filter.filtertext(false);
	for(const QString& filtertext : filtertexts)
	{
		IntSet albumIds = m->search_information.albumIds(filtertext);
		for(int albumId : albumIds)
		{
			int idx = m->albumIdIndexMap[albumId];
			if(!Util::between(idx, m->albums))
			{
				spLog(Log::Warning, this) << __FUNCTION__ << " Invalid index: " << idx << " (" << m->albums.size()
				                          << ")";
				continue;
			}

			if(albums.contains(m->albums[idx].id()))
			{
				albums << m->albums[idx];
			}
		}
	}

	albums.sort(sortorder().so_albums);
}

int SC::Library::getTrackCount() const
{
	return m->tracks.count();
}

void SC::Library::getAllTracks(const QStringList& paths, MetaDataList& v_md) const
{
	Q_UNUSED(paths)
	Q_UNUSED(v_md)
	return;
}

void SC::Library::getAllTracks(MetaDataList& v_md) const
{
	if(m->tracks.isEmpty())
	{
		m->libraryDatabase->getAllTracks(v_md);
		m->tracks = v_md;

		for(int i = 0; i < m->tracks.count(); i++)
		{
			const MetaData& md = v_md[i];

			m->mdIdIndexMap[md.id()] = i;
			m->mdNameIndexMap[md.title()].insert(i);
			m->mdAlbumIdIndexMap[md.albumId()].insert(i);
			m->mdArtistIdIndexMap[md.artistId()].insert(i);
		}
	}

	else
	{
		v_md = m->tracks;
	}

	v_md.sort(sortorder().so_tracks);
}

void SC::Library::getAllTracksByArtist(IdList artistIds, MetaDataList& v_md, ::Library::Filter filter) const
{
	Q_UNUSED(filter)

	for(int artistId : artistIds)
	{
		const IndexSet& idxs = m->mdArtistIdIndexMap[artistId];

		for(int idx : idxs)
		{
			if(!Util::between(idx, m->tracks))
			{
				spLog(Log::Warning, this) << __FUNCTION__ << " Invalid index: " << idx << " (" << m->tracks.size()
				                          << ")";
			}
			else
			{
				v_md << m->tracks[idx];
			}
		}
	}

	v_md.sort(sortorder().so_tracks);
}

void SC::Library::getAllTracksByAlbum(IdList albumIds, MetaDataList& v_md, ::Library::Filter filter) const
{
	Q_UNUSED(filter)

	for(int albumId : albumIds)
	{
		const IndexSet& idxs = m->mdAlbumIdIndexMap[albumId];
		for(int idx : idxs)
		{
			v_md << m->tracks[idx];
		}
	}

	v_md.sort(sortorder().so_tracks);
}

void SC::Library::getAllTracksBySearchstring(::Library::Filter filter, MetaDataList& v_md) const
{
	if(filter.mode() != ::Library::Filter::Mode::Fulltext)
	{
		return;
	}

	if(m->search_information.isEmpty())
	{
		m->libraryDatabase->searchInformation(m->search_information);
	}

	QStringList filtertexts = filter.filtertext(false);
	for(const QString& filtertext : filtertexts)
	{
		IntSet trackIds = m->search_information.trackIds(filtertext);

		for(int trackId : trackIds)
		{
			int idx = m->mdIdIndexMap[trackId];
			if(!v_md.contains(m->tracks[idx].id()))
			{
				v_md << m->tracks[idx];
			}
		}
	}

	v_md.sort(sortorder().so_tracks);
}

void SC::Library::getAllTracksByPath(const QStringList& paths, MetaDataList& v_md) const
{
	Q_UNUSED(paths)
	Q_UNUSED(v_md)
}

void SC::Library::updateTrack(const MetaData& md)
{
	m->libraryDatabase->updateTrack(md);
	refetch();
}

void SC::Library::updateAlbum(const Album& album)
{
	m->libraryDatabase->updateAlbum(album);
	refetch();
}

void SC::Library::deleteTracks(const MetaDataList& tracks, [[maybe_unused]] ::Library::TrackDeletionMode mode)
{
	m->libraryDatabase->deleteTracks(tracks.trackIds());
	refetch();
}

void SC::Library::refetch()
{
	m->clear_cache();

	AbstractLibrary::refetch();

	m->libraryDatabase->searchInformation(m->search_information);
}

void SC::Library::reloadLibrary(bool b, ::Library::ReloadQuality quality)
{
	Q_UNUSED(b)
	Q_UNUSED(quality)

	m->clear_cache();
}

void SC::Library::refreshArtists()
{
	if(selectedArtists().isEmpty())
	{
		return;
	}

	ArtistId artistId = selectedArtists().first();

	MetaDataList v_md;
	getAllTracksByArtist({artistId}, v_md, ::Library::Filter());
	deleteTracks(v_md, ::Library::TrackDeletionMode::None);

	spLog(Log::Debug, this) << "Deleted " << v_md.size() << " soundcloud tracks";

	auto* fetcher = new SC::DataFetcher(this);
	connect(fetcher, &SC::DataFetcher::sigArtistsFetched,
	        this, &SC::Library::artistsFetched);

	fetcher->getArtist(artistId);
}

void SC::Library::refreshAlbums() {}

void SC::Library::refreshTracks() {}

void SC::Library::coverFound(const Cover::Location& cl)
{
	Q_UNUSED(cl)
	//sp_log(Log::Debug, this) << "Saved sound cloud cover: " << cl.toString();
}

void SC::Library::insertTracks(const MetaDataList& v_md, const ArtistList& artists, const AlbumList& albums)
{

	QMap<QString, Artist> artist_map;
	QMap<QString, Album> album_map;

	ArtistList tmp_artists;
	AlbumList tmp_albums;
	m->libraryDatabase->getAllAlbums(tmp_albums, true);
	m->libraryDatabase->getAllArtists(tmp_artists, true);

	{ // insert empty artist and empty album
		Artist artist_tmp;
		artist_tmp.setId(0);
		Album album_tmp;
		album_tmp.setId(0);

		tmp_albums << album_tmp;
		tmp_artists << artist_tmp;
	}

	for(const Artist& artist : tmp_artists)
	{
		artist_map[artist.name()] = artist;
	}

	for(const Album& album : tmp_albums)
	{
		album_map[album.name()] = album;
	}

	for(Artist artist : artists)
	{
		if(!artist_map.contains(artist.name()))
		{
			ArtistId id = m->libraryDatabase->insertArtistIntoDatabase(artist);
			artist.setId(id);
			artist_map.insert(artist.name(), artist);
		}
	}

	for(Album album : albums)
	{
		if(!album_map.contains(album.name()))
		{
			AlbumId id = m->libraryDatabase->insertAlbumIntoDatabase(album);
			album.setId(id);
			album_map.insert(album.name(), album);
		}
	}

	MetaDataList v_md_corrected;
	for(MetaData md : v_md)
	{
		if(!artist_map.contains(md.artist()))
		{
			ArtistId id = m->libraryDatabase->insertArtistIntoDatabase(md.artist());
			md.setArtistId(id);

			Artist artist;
			artist.setId(id);
			artist.setName(md.artist());
			artist_map.insert(artist.name(), artist);
		}

		if(!album_map.contains(md.album()))
		{
			AlbumId id = m->libraryDatabase->insertAlbumIntoDatabase(md.album());
			md.setAlbumId(id);

			Album album;
			album.setId(id);
			album.setName(md.album());
			album_map.insert(album.name(), album);
		}

		v_md_corrected << std::move(md);
	}

	m->libraryDatabase->storeMetadata(v_md_corrected);

	AbstractLibrary::refreshCurrentView();

	refetch();
}

void SC::Library::artistsFetched(const ArtistList& artists)
{
	for(const Artist& artist : artists)
	{
		spLog(Log::Debug, this) << "Artist " << artist.name() << " fetched";

		if(artist.id() <= 0)
		{
			continue;
		}

		m->libraryDatabase->updateArtist(artist);

		auto* fetcher = new SC::DataFetcher(this);

		connect(fetcher, &SC::DataFetcher::sigPlaylistsFetched,
		        this, &SC::Library::albumsFetched);

		connect(fetcher, &SC::DataFetcher::sigTracksFetched,
		        this, &SC::Library::tracksFetched);

		fetcher->getTracksByArtist(artist.id());
	}

	sender()->deleteLater();
	refetch();
}

void SC::Library::tracksFetched(const MetaDataList& v_md)
{
	for(const MetaData& md : v_md)
	{
		if(md.id() > 0)
		{
			m->libraryDatabase->insertTrackIntoDatabase(md, md.artistId(), md.albumId(), md.albumArtistId());
		}
	}

	sender()->deleteLater();
	refetch();
}

void SC::Library::albumsFetched(const AlbumList& albums)
{
	for(const Album& album : albums)
	{
		if(album.id() > 0)
		{
			m->libraryDatabase->insertAlbumIntoDatabase(album);
		}
	}

	sender()->deleteLater();
	refetch();
}

void SC::Library::getTrackById(TrackID trackId, MetaData& md) const
{
	Q_UNUSED(trackId)
	Q_UNUSED(md)
}

void SC::Library::getArtistById(ArtistId artistId, Artist& artist) const
{
	Q_UNUSED(artistId)
	Q_UNUSED(artist)
}

void SC::Library::getAlbumById(AlbumId albumId, Album& album) const
{
	Q_UNUSED(albumId)
	Q_UNUSED(album)
}
