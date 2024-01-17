/* AbstractLibrary.cpp */

/* Copyright (C) 2011-2024 Michael Lugmair (Lucio Carreras)
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

#include "AbstractLibrary.h"

#include "Components/Playlist/LibraryPlaylistInteractor.h"
#include "Components/Tagging/ChangeNotifier.h"
#include "Utils/Algorithm.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/MetaData/Album.h"
#include "Utils/MetaData/Artist.h"
#include "Utils/MetaData/MetaDataSorting.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Language/Language.h"
#include "Utils/FileUtils.h"
#include "Utils/ExtensionSet.h"
#include "Utils/Set.h"

#include <QHash>

namespace
{
	template<typename T>
	IndexSet restoreSelectedIndexes(const T& items, const IdSet& oldSelections)
	{
		auto newSelections = IndexSet {};
		for(auto it = items.begin(); it != items.end(); it++)
		{
			if(oldSelections.contains(it->id()))
			{
				newSelections.insert(std::distance(items.begin(), it));
			}
		}

		return newSelections;
	}

	Gui::ExtensionSet extractExtensions(const MetaDataList& tracks)
	{
		Gui::ExtensionSet extensions;
		for(const auto& track: tracks)
		{
			extensions.addExtension(Util::File::getFileExtension(track.filepath()), false);
		}

		return extensions;
	}
}

struct AbstractLibrary::Private
{
	LibraryPlaylistInteractor* playlistInteractor;
	Util::Set<ArtistId> selectedArtists;
	Util::Set<AlbumId> selectedAlbums;
	Util::Set<TrackID> selectedTracks;

	ArtistList artists;
	AlbumList albums;
	MetaDataList tracks;
	MetaDataList currentTracks;
	MetaDataList filteredTracks;

	Gui::ExtensionSet extensions;

	Library::Sortings sortorder {GetSetting(Set::Lib_Sorting)};
	Library::Filter filter;
	bool loaded {false};

	explicit Private(LibraryPlaylistInteractor* playlistInteractor) :
		playlistInteractor {playlistInteractor} {}
};

AbstractLibrary::AbstractLibrary(LibraryPlaylistInteractor* playlistInteractor, QObject* parent) :
	QObject(parent),
	m {Pimpl::make<Private>(playlistInteractor)} {}

AbstractLibrary::~AbstractLibrary() = default;

void AbstractLibrary::init()
{
	if(!isLoaded())
	{
		ListenSettingNoCall(Set::Lib_SortModeMask, AbstractLibrary::refreshCurrentView);

		initLibraryImpl();
		refetch();
	}

	m->loaded = true;
}

bool AbstractLibrary::isLoaded() const
{
	return m->loaded;
}

void AbstractLibrary::emitAll()
{
	prepareArtists();
	prepareAlbums();
	prepareTracks();

	emit sigAllArtistsLoaded();
	emit sigAllAlbumsLoaded();
	emit sigAllTracksLoaded();
}

void AbstractLibrary::refetch()
{
	m->selectedArtists.clear();
	m->selectedAlbums.clear();
	m->selectedTracks.clear();
	m->filter.clear();

	m->artists.clear();
	m->albums.clear();
	m->tracks.clear();

	getAllArtists(m->artists);
	getAllAlbums(m->albums);
	getAllTracks(m->tracks);

	emitAll();
}

void AbstractLibrary::refreshCurrentView()
{
	/* Waring! Sorting after each fetch is important here! */
	/* Do not call emit_stuff() in order to avoid double sorting */
	const auto selectedArtists = std::move(m->selectedArtists);
	const auto selectedAlbums = std::move(m->selectedAlbums);
	const auto selectedTracks = std::move(m->selectedTracks);

	fetchByFilter(m->filter, true);

	prepareArtists();
	const auto selectedArtistIndexes = restoreSelectedIndexes(artists(), selectedArtists);
	changeArtistSelection(selectedArtistIndexes);

	prepareAlbums();
	const auto selectedAlbumIndexes = restoreSelectedIndexes(albums(), selectedAlbums);
	changeAlbumSelection(selectedAlbumIndexes);

	prepareTracks();
	const auto selectedTrackIndexes = restoreSelectedIndexes(tracks(), selectedTracks);

	emit sigAllAlbumsLoaded();
	emit sigAllArtistsLoaded();
	emit sigAllTracksLoaded();

	if(!selectedTrackIndexes.isEmpty())
	{
		changeTrackSelection(selectedTrackIndexes);
	}
}

void AbstractLibrary::findTrack(TrackID id)
{
	MetaData track;
	getTrackById(id, track);

	if(track.id() < 0)
	{
		return;
	}

	{ // clear old selections/filters
		if(!m->selectedArtists.isEmpty())
		{
			selectedArtistsChanged(IndexSet());
		}

		if(!m->selectedAlbums.isEmpty())
		{
			selectedAlbumsChanged(IndexSet());
		}

		// make sure, that no artist_selection_changed or album_selection_changed
		// messes things up
		emitAll();
	}

	{ // clear old fetched artists/albums/tracks
		m->tracks.clear();
		m->artists.clear();
		m->albums.clear();

		m->selectedTracks.clear();
		m->filteredTracks.clear();
		m->selectedArtists.clear();
		m->selectedAlbums.clear();
	}

	Artist artist;
	getArtistById(track.artistId(), artist);
	m->artists.push_back(std::move(artist));

	Album album;
	getAlbumById(track.albumId(), album);
	m->albums.push_back(std::move(album));

	getAllTracksByAlbum({track.albumId()}, m->tracks, Library::Filter());
	m->selectedTracks << track.id();

	emitAll();
}

void AbstractLibrary::prepareFetchedTracksForPlaylist(bool createNewPlaylist)
{
	m->playlistInteractor->createPlaylist(tracks(), createNewPlaylist);
}

void AbstractLibrary::prepareCurrentTracksForPlaylist(bool createNewPlaylist)
{
	m->playlistInteractor->createPlaylist(currentTracks(), createNewPlaylist);
}

void AbstractLibrary::prepareTracksForPlaylist(const QStringList& paths, bool createNewPlaylist)
{
	m->playlistInteractor->createPlaylist(paths, createNewPlaylist);
}

void AbstractLibrary::playNextFetchedTracks()
{
	m->playlistInteractor->insertAfterCurrentTrack(tracks());
}

void AbstractLibrary::playNextCurrentTracks()
{
	m->playlistInteractor->insertAfterCurrentTrack(currentTracks());
}

void AbstractLibrary::appendFetchedTracks()
{
	m->playlistInteractor->append(tracks());
}

void AbstractLibrary::appendCurrentTracks()
{
	m->playlistInteractor->append(currentTracks());
}

void AbstractLibrary::changeArtistSelection(const IndexSet& indexes)
{
	Util::Set<ArtistId> selectedArtists;
	for(auto idx: indexes)
	{
		const auto& artist = m->artists[static_cast<ArtistList::size_type>(idx)];
		selectedArtists.insert(artist.id());
	}

	if(selectedArtists == m->selectedArtists)
	{
		return;
	}

	m->selectedArtists = std::move(selectedArtists);
	m->albums.clear();
	m->tracks.clear();

	if(!m->selectedArtists.isEmpty() && (m->selectedArtists.size() < m->artists.size()))
	{
		getAllTracksByArtist(m->selectedArtists.toList(), m->tracks, m->filter);
		getAllAlbumsByArtist(m->selectedArtists.toList(), m->albums, m->filter);
	}

	else if(!m->filter.cleared())
	{
		getAllTracksBySearchstring(m->filter, m->tracks);
		getAllAlbumsBySearchstring(m->filter, m->albums);
		getAllArtistsBySearchstring(m->filter, m->artists);
	}

	else
	{
		getAllTracks(m->tracks);
		getAllAlbums(m->albums);
	}

	prepareArtists();
	prepareAlbums();
	prepareTracks();
}

const MetaDataList& AbstractLibrary::tracks() const
{
	return (m->filteredTracks.isEmpty())
	       ? m->tracks
	       : m->filteredTracks;
}

const AlbumList& AbstractLibrary::albums() const
{
	return m->albums;
}

const ArtistList& AbstractLibrary::artists() const
{
	return m->artists;
}

const MetaDataList& AbstractLibrary::currentTracks() const
{
	return (m->selectedTracks.isEmpty())
	       ? tracks()
	       : m->currentTracks;
}

void AbstractLibrary::changeCurrentDisc(Disc disc)
{
	if(m->selectedAlbums.size() != 1)
	{
		return;
	}

	getAllTracksByAlbum(m->selectedAlbums.toList(), m->tracks, m->filter);

	if(disc != std::numeric_limits<Disc>::max())
	{
		m->tracks.removeTracks([disc](const MetaData& track) {
			return (track.discnumber() != disc);
		});
	}

	prepareTracks();
	emit sigAllTracksLoaded();
}

const IdSet& AbstractLibrary::selectedAlbums() const
{
	return m->selectedAlbums;
}

const IdSet& AbstractLibrary::selectedArtists() const
{
	return m->selectedArtists;
}

Library::Filter AbstractLibrary::filter() const
{
	return m->filter;
}

void AbstractLibrary::changeFilter(Library::Filter filter, bool force)
{
	if(filter.mode() != Library::Filter::InvalidGenre)
	{
		const auto filtertext = filter.filtertext(false);
		const auto searchStringLength = GetSetting(Set::Lib_SearchStringLength);

		if(filtertext.join("").size() < searchStringLength)
		{
			filter.clear();
		}

		else
		{
			filter.setFiltertext(filtertext.join(","));
		}
	}

	if(!filter.isEqual(m->filter, GetSetting(Set::Lib_SearchStringLength)))
	{
		fetchByFilter(filter, force);
		emitAll();
	}
}

void AbstractLibrary::selectedArtistsChanged(const IndexSet& indexes)
{
	// happens, when the model is set at initialization of table views
	if(m->selectedArtists.isEmpty() && indexes.isEmpty())
	{
		return;
	}

	changeArtistSelection(indexes);

	emit sigAllAlbumsLoaded();
	emit sigAllTracksLoaded();
}

void AbstractLibrary::changeAlbumSelection(const IndexSet& indexes, bool ignoreArtists)
{
	Util::Set<AlbumId> selectedAlbums;

	for(const auto& index: indexes)
	{
		if(index < m->albums.count())
		{
			const auto& album = m->albums[index];
			selectedAlbums.insert(album.id());
		}
	}

	m->tracks.clear();
	m->selectedAlbums = selectedAlbums;

	// only show tracks of selected album / artist
	if(!m->selectedArtists.isEmpty() && !ignoreArtists)
	{
		if(!m->selectedAlbums.isEmpty())
		{
			MetaDataList tracks;
			getAllTracksByAlbum(m->selectedAlbums.toList(), tracks, m->filter);

			Util::Algorithm::moveIf(tracks, m->tracks, [&](const auto& track) {
				const auto artistId = (GetSetting(Set::Lib_ShowAlbumArtists))
				                      ? track.albumArtistId()
				                      : track.artistId();

				return m->selectedArtists.contains(artistId);
			});
		}

		else
		{
			getAllTracksByArtist(m->selectedArtists.toList(), m->tracks, m->filter);
		}
	}

	else if(!m->selectedAlbums.isEmpty())
	{
		getAllTracksByAlbum(m->selectedAlbums.toList(), m->tracks, m->filter);
	}

	else if(!m->filter.cleared())
	{
		getAllTracksBySearchstring(m->filter, m->tracks);
	}

	else
	{
		getAllTracks(m->tracks);
	}

	prepareTracks();
}

void AbstractLibrary::selectedAlbumsChanged(const IndexSet& indexes, bool ignoreArtists)
{
	// happens, when the model is set at initialization of table views
	if(m->selectedAlbums.isEmpty() && indexes.isEmpty())
	{
		return;
	}

	changeAlbumSelection(indexes, ignoreArtists);
	emit sigAllTracksLoaded();
}

void AbstractLibrary::changeTrackSelection(const IndexSet& indexes)
{
	m->selectedTracks.clear();
	m->currentTracks.clear();

	for(const auto& index: indexes)
	{
		if(index < 0 || index >= tracks().count())
		{
			continue;
		}

		const auto& track = tracks().at(index);

		m->currentTracks << track;
		m->selectedTracks.insert(track.id());
	}
}

void AbstractLibrary::selectedTracksChanged(const IndexSet& indexes)
{
	changeTrackSelection(indexes);
}

void AbstractLibrary::fetchByFilter(const Library::Filter& filter, bool force)
{
	if((m->filter.isEqual(filter, GetSetting(Set::Lib_SearchStringLength))) &&
	   (m->selectedArtists.empty()) &&
	   (m->selectedAlbums.empty()) &&
	   !force)
	{
		return;
	}

	m->filter = filter;

	m->artists.clear();
	m->albums.clear();
	m->tracks.clear();

	m->selectedArtists.clear();
	m->selectedAlbums.clear();

	if(m->filter.cleared())
	{
		getAllArtists(m->artists);
		getAllAlbums(m->albums);
		getAllTracks(m->tracks);
	}

	else
	{
		getAllArtistsBySearchstring(m->filter, m->artists);
		getAllAlbumsBySearchstring(m->filter, m->albums);
		getAllTracksBySearchstring(m->filter, m->tracks);
	}
}

void AbstractLibrary::fetchTracksByPath(const QStringList& paths)
{
	m->tracks.clear();

	if(!paths.isEmpty())
	{
		getAllTracksByPath(paths, m->tracks);
	}

	emitAll();
}

void AbstractLibrary::changeTrackSortorder(const Library::TrackSortorder sortOrder)
{
	if(sortOrder == m->sortorder.tracks)
	{
		return;
	}

	auto sorting = GetSetting(Set::Lib_Sorting);
	sorting.tracks = sortOrder;
	SetSetting(Set::Lib_Sorting, sorting);
	m->sortorder = sorting;

	prepareTracks();
	emit sigAllTracksLoaded();
}

void AbstractLibrary::changeAlbumSortorder(const Library::AlbumSortorder sortOrder)
{
	if(sortOrder == m->sortorder.album)
	{
		return;
	}

	auto sorting = GetSetting(Set::Lib_Sorting);
	sorting.album = sortOrder;
	SetSetting(Set::Lib_Sorting, sorting);

	m->sortorder = sorting;

	prepareAlbums();
	emit sigAllAlbumsLoaded();
}

void AbstractLibrary::changeArtistSortorder(const Library::ArtistSortorder sortOrder)
{
	if(sortOrder == m->sortorder.artist)
	{
		return;
	}

	auto sorting = GetSetting(Set::Lib_Sorting);
	sorting.artist = sortOrder;
	SetSetting(Set::Lib_Sorting, sorting);

	m->sortorder = sorting;

	prepareArtists();
	emit sigAllArtistsLoaded();
}

Library::Sortings AbstractLibrary::sortorder() const
{
	return m->sortorder;
}

void AbstractLibrary::importFiles(const QStringList& /* files */) {}

void AbstractLibrary::deleteCurrentTracks(Library::TrackDeletionMode mode)
{
	if(mode != Library::TrackDeletionMode::None)
	{
		deleteTracks(currentTracks(), mode);
	}
}

void AbstractLibrary::deleteFetchedTracks(Library::TrackDeletionMode mode)
{
	if(mode != Library::TrackDeletionMode::None)
	{
		deleteTracks(tracks(), mode);
	}
}

void AbstractLibrary::deleteAllTracks()
{
	MetaDataList tracks;
	getAllTracks(tracks);
	deleteTracks(tracks, Library::TrackDeletionMode::OnlyLibrary);
}

void AbstractLibrary::deleteTracks(const MetaDataList& tracks, Library::TrackDeletionMode mode)
{
	if(mode == Library::TrackDeletionMode::None)
	{
		return;
	}

	const auto fileEntry = (mode == Library::TrackDeletionMode::AlsoFiles)
	                       ? Lang::get(Lang::Files)
	                       : Lang::get(Lang::Entries);
	QString answerString;

	auto failCount = 0UL;
	if(mode == Library::TrackDeletionMode::AlsoFiles)
	{
		failCount = std::count_if(tracks.begin(), tracks.end(), [](const auto& track) {
			return !QFile(track.filepath()).remove();
		});
	}

	answerString = (failCount == 0)
	               ? tr("All %1 could be removed").arg(fileEntry)
	               : tr("%1 of %2 %3 could not be removed")
		               .arg(failCount)
		               .arg(tracks.size())
		               .arg(fileEntry);

	emit sigDeleteAnswer(answerString);
	Tagging::ChangeNotifier::instance()->deleteMetadata(tracks);

	refreshCurrentView();
}

void AbstractLibrary::replaceAlbum(const int index, const Album& album)
{
	if(Util::between(index, m->albums))
	{
		m->albums[index] = album;
		emit sigCurrentAlbumChanged(index);
	}
}

void AbstractLibrary::replaceTrack(const int index, const MetaData& track)
{
	if(Util::between(index, m->tracks))
	{
		m->tracks[index] = track;
		emit sigCurrentTrackChanged(index);
	}
}

void AbstractLibrary::prepareTracks()
{
	m->filteredTracks.clear();
	m->extensions = extractExtensions(tracks());

	MetaDataSorting::sortMetadata(m->tracks, m->sortorder.tracks, GetSetting(Set::Lib_SortModeMask));
}

void AbstractLibrary::prepareAlbums()
{
	MetaDataSorting::sortAlbums(m->albums, m->sortorder.album, GetSetting(Set::Lib_SortModeMask));
}

void AbstractLibrary::prepareArtists()
{
	MetaDataSorting::sortArtists(m->artists, m->sortorder.artist, GetSetting(Set::Lib_SortModeMask));
}

Gui::ExtensionSet AbstractLibrary::extensions() const
{
	return m->extensions;
}

bool AbstractLibrary::isReloading() const
{
	return false;
}

bool AbstractLibrary::isEmpty() const
{
	return m->tracks.isEmpty() && (getTrackCount() == 0);
}

void AbstractLibrary::setExtensions(const Gui::ExtensionSet& extensions)
{
	m->extensions = extensions;
	m->filteredTracks.clear();

	if(m->extensions.hasEnabledExtensions())
	{
		Util::Algorithm::copyIf(m->tracks, m->filteredTracks, [&](const auto& track) {
			const auto extension = ::Util::File::getFileExtension(track.filepath());
			return (m->extensions.isEnabled(extension));
		});
	}

	emit sigAllTracksLoaded();
}
