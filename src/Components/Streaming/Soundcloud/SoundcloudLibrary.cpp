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
#include "SearchInformation.h"
#include "Sorting.h"

#include "Components/Covers/CoverLookup.h"
#include "Components/Covers/CoverLocation.h"

#include "Utils/Algorithm.h"
#include "Utils/Logger/Logger.h"
#include "Utils/MetaData/Album.h"
#include "Utils/MetaData/Artist.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/MetaData/MetaDataSorting.h"
#include "Utils/Set.h"
#include "Utils/Settings/Settings.h"
#include "Utils/globals.h"
#include "Utils/typedefs.h"

#include <QHash>

namespace
{
	using IdIndexMap = QHash<Id, int>;
	using IdIndexSetMap = QHash<Id, Util::Set<int>>;

	template<typename Element>
	QMap<QString, Element> createMap(const std::deque<Element>& container)
	{
		QMap<QString, Element> result;

		for(const auto& item: container)
		{
			result[item.name()] = item;
		}

		return result;
	}

	template<typename Element, typename Inserter>
	void insertMissingIntoDatabase(const std::deque<Element>& container, QMap<QString, Element>& map, Inserter&& fn)
	{
		for(auto item: container)
		{
			if(!map.contains(item.name()))
			{
				const auto id = fn(item);
				item.setId(id);
				map.insert(item.name(), std::move(item));
			}
		}
	}

	MetaDataList getAllTracksByEntity(const IdList& ids, const IdIndexSetMap& indexMap, const MetaDataList& trackPool,
	                                  Library::SortOrder sortOrder)
	{
		MetaDataList result;
		for(const auto id: ids)
		{
			if(indexMap.contains(id))
			{
				const auto& indexes = indexMap[id];
				for(const auto& index: indexes)
				{
					if(Util::between(index, trackPool))
					{
						result << trackPool[index];
					}
				}
			}
		}

		MetaDataSorting::sortMetadata(result, sortOrder, GetSetting(Set::Lib_SortModeMask));

		return result;
	}
}

struct SC::Library::Private
{
	IdIndexMap trackIdIndexMap;
	IdIndexSetMap trackArtistIdIndexMap;
	IdIndexSetMap trackAlbumIdIndexMap;
	QHash<QString, IndexSet> trackNameIndexMap;

	IdIndexMap albumIdIndexMap;
	QHash<QString, IndexSet> albumNameIndexMap;
	QHash<QString, IndexSet> artistNameAlbumIndexMap;

	IdIndexMap artistIdIndexMap;
	QHash<QString, IndexSet> artistNameIndexMap;

	MetaDataList tracks;
	AlbumList albums;
	ArtistList artists;

	std::shared_ptr<SC::Database> db {std::make_shared<SC::Database>()};
	std::shared_ptr<SC::LibraryDatabase> libraryDatabase;
	SearchInformationList searchInformation;

	Private() :
		libraryDatabase {std::make_shared<SC::LibraryDatabase>(db->connectionName(), db->databaseId(), -1)} {}

	~Private()
	{
		db->closeDatabase();
	}

	void clearCache()
	{
		tracks.clear();
		albums.clear();
		artists.clear();
		searchInformation.clear();
		trackIdIndexMap.clear();
		trackArtistIdIndexMap.clear();
		trackAlbumIdIndexMap.clear();
		trackNameIndexMap.clear();
		albumIdIndexMap.clear();
		albumNameIndexMap.clear();
		artistNameAlbumIndexMap.clear();
		artistIdIndexMap.clear();
		artistNameIndexMap.clear();
	}
};

SC::Library::Library(LibraryPlaylistInteractor* playlistInteractor, QObject* parent) :
	AbstractLibrary(playlistInteractor, parent),
	m {Pimpl::make<Private>()} {}

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

		for(auto i = 0; i < m->artists.count(); i++)
		{
			const auto& artist = artists[size_t(i)];
			m->artistIdIndexMap[artist.id()] = i;
			m->artistNameIndexMap[artist.name()].insert(i);
		}
	}

	else
	{
		artists = m->artists;
	}

	MetaDataSorting::sortArtists(artists, sortorder().so_artists, GetSetting(Set::Lib_SortModeMask));
}

void SC::Library::getAllArtistsBySearchstring(::Library::Filter filter, ArtistList& artists) const
{
	if(filter.mode() != ::Library::Filter::Mode::Fulltext)
	{
		return;
	}

	if(m->searchInformation.isEmpty())
	{
		m->libraryDatabase->searchInformation(m->searchInformation);
	}

	const auto filtertexts = filter.filtertext(false);
	for(const auto& filtertext: filtertexts)
	{
		const auto artistIds = m->searchInformation.artistIds(filtertext);
		for(const auto& artistId: artistIds)
		{
			const auto index = m->artistIdIndexMap[artistId];
			auto artist = m->artists[static_cast<size_t>(index)];

			const auto contains = Util::Algorithm::contains(artists, [id = artist.id()](const auto& a) {
				return (a.id() == id);
			});

			if(!contains)
			{
				const auto songCount = static_cast<uint16_t>(m->trackArtistIdIndexMap[artistId].count());
				artist.setSongcount(songCount);
				artists << artist;
			}
		}
	}

	MetaDataSorting::sortArtists(artists, sortorder().so_artists, GetSetting(Set::Lib_SortModeMask));
}

void SC::Library::getAllAlbums(AlbumList& albums) const
{
	if(m->albums.empty())
	{
		m->libraryDatabase->getAllAlbums(albums, false);
		m->albums = albums;

		for(auto i = 0; i < albums.count(); i++)
		{
			const auto& album = albums[i];
			m->albumIdIndexMap[album.id()] = i;
			m->albumNameIndexMap[album.name()].insert(i);

			const auto artists = album.artists();
			for(const auto& artist: artists)
			{
				m->artistNameAlbumIndexMap[artist].insert(i);
			}
		}
	}

	else
	{
		albums = m->albums;
	}

	MetaDataSorting::sortAlbums(albums, sortorder().so_albums, GetSetting(Set::Lib_SortModeMask));
}

void
SC::Library::getAllAlbumsByArtist(IdList artistIds, AlbumList& albums, [[maybe_unused]] ::Library::Filter filter) const
{
	for(const auto artistId: artistIds)
	{
		const auto index = m->artistIdIndexMap[artistId];
		const auto& artist = m->artists[static_cast<size_t>(index)];

		const auto indexes = m->artistNameAlbumIndexMap[artist.name()];
		for(const auto& artistIndex: indexes)
		{
			if(Util::between(index, m->albums))
			{
				albums.push_back(m->albums[artistIndex]);
			}
		}
	}

	MetaDataSorting::sortAlbums(albums, sortorder().so_albums, GetSetting(Set::Lib_SortModeMask));
}

void SC::Library::getAllAlbumsBySearchstring(::Library::Filter filter, AlbumList& albums) const
{
	if(filter.mode() != ::Library::Filter::Mode::Fulltext)
	{
		return;
	}

	if(m->searchInformation.isEmpty())
	{
		m->libraryDatabase->searchInformation(m->searchInformation);
	}

	const auto filtertexts = filter.filtertext(false);
	for(const auto& filtertext: filtertexts)
	{
		IntSet albumIds = m->searchInformation.albumIds(filtertext);
		for(const auto& albumId: albumIds)
		{
			const auto index = m->albumIdIndexMap[albumId];
			const auto contains = Util::Algorithm::contains(albums, [id = m->albums[index].id()](const auto& album) {
				return album.id() == id;
			});

			if(Util::between(index, m->albums) && !contains)
			{
				albums << m->albums[index];
			}
		}
	}

	MetaDataSorting::sortAlbums(albums, sortorder().so_albums, GetSetting(Set::Lib_SortModeMask));
}

int SC::Library::getTrackCount() const
{
	return m->tracks.count();
}

void
SC::Library::getAllTracks([[maybe_unused]] const QStringList& paths, [[maybe_unused]]  MetaDataList& tracks) const {}

void SC::Library::getAllTracks(MetaDataList& tracks) const
{
	if(m->tracks.isEmpty())
	{
		m->libraryDatabase->getAllTracks(tracks);
		m->tracks = tracks;

		for(auto i = 0; i < m->tracks.count(); i++)
		{
			const auto& track = tracks[i];

			m->trackIdIndexMap[track.id()] = i;
			m->trackNameIndexMap[track.title()].insert(i);
			m->trackAlbumIdIndexMap[track.albumId()].insert(i);
			m->trackArtistIdIndexMap[track.artistId()].insert(i);
		}
	}

	else
	{
		tracks = m->tracks;
	}

	MetaDataSorting::sortMetadata(tracks, sortorder().so_tracks, GetSetting(Set::Lib_SortModeMask));
}

void SC::Library::getAllTracksByArtist(IdList artistIds, MetaDataList& tracks,
                                       [[maybe_unused]]::Library::Filter filter) const
{
	tracks = getAllTracksByEntity(artistIds, m->trackArtistIdIndexMap, m->tracks, sortorder().so_tracks);
}

void
SC::Library::getAllTracksByAlbum(IdList albumIds, MetaDataList& tracks, [[maybe_unused]] ::Library::Filter filter) const
{
	tracks = getAllTracksByEntity(albumIds, m->trackAlbumIdIndexMap, m->tracks, sortorder().so_tracks);
}

void SC::Library::getAllTracksBySearchstring(::Library::Filter filter, MetaDataList& tracks) const
{
	if(filter.mode() != ::Library::Filter::Mode::Fulltext)
	{
		return;
	}

	if(m->searchInformation.isEmpty())
	{
		m->libraryDatabase->searchInformation(m->searchInformation);
	}

	const auto filterTexts = filter.filtertext(false);
	for(const auto& filterText: filterTexts)
	{
		const auto trackIds = m->searchInformation.trackIds(filterText);
		for(const auto trackId: trackIds)
		{
			const auto index = m->trackIdIndexMap[trackId];
			if(!tracks.contains(m->tracks[index].id()))
			{
				tracks << m->tracks[index];
			}
		}
	}

	MetaDataSorting::sortMetadata(tracks, sortorder().so_tracks, GetSetting(Set::Lib_SortModeMask));
}

void SC::Library::getAllTracksByPath([[maybe_unused]] const QStringList& paths,
                                     [[maybe_unused]] MetaDataList& tracks) const {}

void SC::Library::updateTrack(const MetaData& track)
{
	m->libraryDatabase->updateTrack(track);
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
	m->clearCache();

	AbstractLibrary::refetch();

	m->libraryDatabase->searchInformation(m->searchInformation);
}

void SC::Library::reloadLibrary([[maybe_unused]]bool b, [[maybe_unused]]::Library::ReloadQuality quality)
{
	m->clearCache();
}

void SC::Library::refreshArtists()
{
	if(selectedArtists().isEmpty())
	{
		return;
	}

	const auto artistId = selectedArtists().first();

	MetaDataList tracks;
	getAllTracksByArtist({artistId}, tracks, ::Library::Filter());
	deleteTracks(tracks, ::Library::TrackDeletionMode::None);

	spLog(Log::Debug, this) << "Deleted " << tracks.size() << " soundcloud tracks";

	auto* fetcher = new SC::DataFetcher(this);
	connect(fetcher, &SC::DataFetcher::sigArtistsFetched,
	        this, &SC::Library::artistsFetched);

	fetcher->getArtist(artistId);
}

void SC::Library::insertTracks(const MetaDataList& tracks, const ArtistList& artists, const AlbumList& albums)
{
	ArtistList tmpArtists;
	AlbumList tmpAlbums;
	m->libraryDatabase->getAllAlbums(tmpAlbums, true);
	m->libraryDatabase->getAllArtists(tmpArtists, true);

	auto artistMap = createMap(tmpArtists);
	auto albumMap = createMap(tmpAlbums);

	insertMissingIntoDatabase(artists, artistMap, [&](const auto& artist) -> ArtistId {
		return m->libraryDatabase->insertArtistIntoDatabase(artist);
	});

	insertMissingIntoDatabase(albums, albumMap, [&](const auto& album) -> AlbumId {
		return m->libraryDatabase->insertAlbumIntoDatabase(album);
	});

	MetaDataList tracksCorrected;
	tracksCorrected.reserve(tracks.size());

	for(auto track: tracks)
	{
		if(!artistMap.contains(track.artist()))
		{
			const auto artistId = m->libraryDatabase->insertArtistIntoDatabase(track.artist());

			Artist artist;
			artist.setId(artistId);
			artist.setName(track.artist());
			artistMap.insert(artist.name(), artist);
		}

		track.setArtistId(artistMap[track.artist()].id());

		if(!albumMap.contains(track.album()))
		{
			const auto albumId = m->libraryDatabase->insertAlbumIntoDatabase(track.album());

			Album album;
			album.setId(albumId);
			album.setName(track.album());
			albumMap.insert(album.name(), album);
		}

		track.setAlbumId(albumMap[track.album()].id());

		tracksCorrected << std::move(track);
	}

	m->libraryDatabase->storeMetadata(tracksCorrected);

	AbstractLibrary::refreshCurrentView();

	refetch();
}

void SC::Library::artistsFetched(const ArtistList& artists)
{
	for(const auto& artist: artists)
	{
		spLog(Log::Debug, this) << "Artist " << artist.name() << " fetched";
		if(artist.id() <= 0)
		{
			continue;
		}

		m->libraryDatabase->updateArtist(artist);

		auto* fetcher = new SC::DataFetcher(this);
		connect(fetcher, &SC::DataFetcher::sigPlaylistsFetched, this, &SC::Library::albumsFetched);
		connect(fetcher, &SC::DataFetcher::sigTracksFetched, this, &SC::Library::tracksFetched);

		fetcher->getTracksByArtist(artist.id());
	}

	sender()->deleteLater();
	refetch();
}

void SC::Library::tracksFetched(const MetaDataList& tracks)
{
	m->libraryDatabase->db().transaction();

	for(const auto& track: tracks)
	{
		spLog(Log::Info, this) << "Try to insert track " << track.title() << " (" << track.id() << ")";

		if(track.id() > 0)
		{
			m->libraryDatabase->insertTrackIntoDatabase(
				track,
				track.artistId(),
				track.albumId(),
				track.albumArtistId());
		}
	}

	m->libraryDatabase->db().commit();

	sender()->deleteLater();
	refetch();
}

void SC::Library::albumsFetched(const AlbumList& albums)
{
	m->libraryDatabase->db().transaction();

	for(const auto& album: albums)
	{
		spLog(Log::Info, this) << "Try to insert album " << album.name() << " (" << album.id() << ")";
		if(album.id() >= 0)
		{
			m->libraryDatabase->insertAlbumIntoDatabase(album);
		}
	}

	m->libraryDatabase->db().commit();

	sender()->deleteLater();
	refetch();
}

void SC::Library::getTrackById([[maybe_unused]] TrackID trackId, [[maybe_unused]] MetaData& track) const {}

void SC::Library::getArtistById([[maybe_unused]] ArtistId artistId, [[maybe_unused]] Artist& artist) const {}

void SC::Library::getAlbumById([[maybe_unused]] AlbumId albumId, [[maybe_unused]] Album& album) const {}

void SC::Library::refreshAlbums() {}

void SC::Library::refreshTracks() {}

void SC::Library::coverFound([[maybe_unused]] const Cover::Location& coverLocation) {}