/* AbstractLibrary.cpp */

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

#include "AbstractLibrary.h"

#include "Components/Playlist/PlaylistHandler.h"
#include "Components/Playlist/Playlist.h"
#include "Components/Tagging/ChangeNotifier.h"

#include "Utils/MetaData/MetaDataList.h"
#include "Utils/MetaData/Album.h"
#include "Utils/MetaData/Artist.h"
#include "Utils/MetaData/MetaDataSorting.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Language/Language.h"
#include "Utils/Library/SearchMode.h"
#include "Utils/Utils.h"
#include "Utils/FileUtils.h"
#include "Utils/ExtensionSet.h"
#include "Utils/Set.h"

#include <QHash>

namespace
{
	template<typename Tracks>
	void createPlaylist(const Tracks& tracks, bool createNewPlaylist)
	{
		auto* plh = Playlist::HandlerProvider::instance()->handler();
		const auto name = (createNewPlaylist) ? plh->requestNewPlaylistName() : QString();

		plh->createPlaylist(tracks, name);
		plh->applyPlaylistActionAfterDoubleClick();
	}

	void insertTracksAfterCurrentTrack(const MetaDataList& tracks)
	{
		auto* plh = Playlist::HandlerProvider::instance()->handler();
		auto pl = plh->activePlaylist();
		pl->insertTracks(tracks, pl->currentTrackIndex() + 1);
	}

	void appendTracks(const MetaDataList& tracks)
	{
		auto* plh = Playlist::HandlerProvider::instance()->handler();
		auto pl = plh->activePlaylist();
		pl->appendTracks(tracks);
	}
}

struct AbstractLibrary::Private
{
	Util::Set<ArtistId> selectedArtists;
	Util::Set<AlbumId> selectedAlbums;
	Util::Set<TrackID> selectedTracks;

	ArtistList artists;
	AlbumList albums;
	MetaDataList tracks;
	MetaDataList currentTracks;
	MetaDataList filteredTracks;            // a subset of tracks with the desired filename extension

	Gui::ExtensionSet extensions;

	int trackCount;

	Library::Sortings sortorder;
	Library::Filter filter;
	bool loaded;

	Private() :
		trackCount(0),
		sortorder(GetSetting(Set::Lib_Sorting)),
		loaded(false)
	{
		filter.setMode(Library::Filter::Fulltext);
		filter.setFiltertext("", GetSetting(Set::Lib_SearchMode));
	}
};

AbstractLibrary::AbstractLibrary(QObject* parent) :
	QObject(parent)
{
	m = Pimpl::make<Private>();

	auto* mdcn = Tagging::ChangeNotifier::instance();
	connect(mdcn, &Tagging::ChangeNotifier::sigMetadataChanged,
	        this, &AbstractLibrary::metadataChanged);

	connect(mdcn, &Tagging::ChangeNotifier::sigMetadataDeleted,
	        this, &AbstractLibrary::metadataChanged);

	connect(mdcn, &Tagging::ChangeNotifier::sigAlbumsChanged,
	        this, &AbstractLibrary::albumsChanged);
}

AbstractLibrary::~AbstractLibrary() = default;

void AbstractLibrary::load()
{
	{ // init artist sorting mode
		ListenSettingNoCall(Set::Lib_SortIgnoreArtistArticle, AbstractLibrary::ignoreArtistArticleChanged);

		bool b = GetSetting(Set::Lib_SortIgnoreArtistArticle);
		MetaDataSorting::setIgnoreArticle(b);
	}

	m->filter.clear();

	refetch();
	m->trackCount = getTrackCount();

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
	IndexSet selectedArtistIndexes, selectedAlbumIndexes, selectedTrackIndexes;

	IdSet selectedArtists = m->selectedArtists;
	IdSet selectedAlbums = m->selectedAlbums;
	IdSet selectedTracks = m->selectedTracks;

	fetchByFilter(m->filter, true);

	prepareArtists();
	for(int i = 0; i < m->artists.count(); i++)
	{
		if(selectedArtists.contains(m->artists[i].id()))
		{
			selectedArtistIndexes.insert(i);
		}
	}

	changeArtistSelection(selectedArtistIndexes);

	prepareAlbums();
	for(int i = 0; i < m->albums.count(); i++)
	{
		if(selectedAlbums.contains(m->albums[i].id()))
		{
			selectedAlbumIndexes.insert(i);
		}
	}

	changeAlbumSelection(selectedAlbumIndexes);

	prepareTracks();

	const MetaDataList& tracks = this->tracks();
	for(int i = 0; i < tracks.count(); i++)
	{
		if(selectedTracks.contains(tracks[i].id()))
		{
			selectedTrackIndexes.insert(i);
		}
	}

	emit sigAllAlbumsLoaded();
	emit sigAllArtistsLoaded();
	emit sigAllTracksLoaded();

	if(!selectedTrackIndexes.isEmpty())
	{
		changeTrackSelection(selectedTrackIndexes);
	}
}

void AbstractLibrary::metadataChanged()
{
	auto* mdcn = static_cast<Tagging::ChangeNotifier*>(sender());
	const auto& changedTracks = mdcn->changedMetadata();

	QHash<TrackID, int> idRowMap;
	{ // build lookup tree
		int i = 0;
		for(auto it = m->tracks.begin(); it != m->tracks.end(); it++, i++)
		{
			idRowMap[it->id()] = i;
		}
	}

	auto needsRefresh = false;
	for(auto it = changedTracks.begin(); it != changedTracks.end(); it++)
	{
		const auto& oldTrack = it->first;
		const auto& newTrack = it->second;

		needsRefresh =
			(oldTrack.artist() != newTrack.artist()) ||
			(oldTrack.albumArtist() != newTrack.albumArtist()) ||
			(oldTrack.album() != newTrack.album());

		if(idRowMap.contains(oldTrack.id()))
		{
			const auto row = idRowMap[oldTrack.id()];
			m->tracks[row] = newTrack;

			emit sigCurrentTrackChanged(row);
		}
	}

	if(needsRefresh)
	{
		refreshCurrentView();
	}
}

void AbstractLibrary::albumsChanged()
{
	auto* mdcn = static_cast<Tagging::ChangeNotifier*>(sender());

	QHash<AlbumId, int> idRowMap;
	{ // build lookup tree
		int i = 0;
		for(auto it = m->albums.begin(); it != m->albums.end(); it++, i++)
		{
			idRowMap[it->id()] = i;
		}
	}

	const QList<AlbumPair> changedAlbums = mdcn->changedAlbums();
	for(const AlbumPair& albumPair : changedAlbums)
	{
		const Album& oldAlbum = albumPair.first;
		const Album& newAlbum = albumPair.second;

		if(idRowMap.contains(oldAlbum.id()))
		{
			int row = idRowMap[oldAlbum.id()];
			m->albums[row] = newAlbum;

			emit sigCurrentAlbumChanged(row);
		}
	}
}

void AbstractLibrary::findTrack(TrackID id)
{
	MetaData md;
	getTrackById(id, md);

	if(md.id() < 0)
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

	m->tracks << md;

	{ // artist
		Artist artist;
		getArtistById(md.artistId(), artist);
		m->artists << artist;
	}

	{ // album
		Album album;
		getAlbumById(md.albumId(), album);
		m->albums << album;
	}

	getAllTracksByAlbum({md.albumId()}, m->tracks, Library::Filter());
	m->selectedTracks << md.id();

	emitAll();
}

void AbstractLibrary::prepareFetchedTracksForPlaylist(bool createNewPlaylist)
{
	createPlaylist(tracks(), createNewPlaylist);
}

void AbstractLibrary::prepareCurrentTracksForPlaylist(bool createNewPlaylist)
{
	createPlaylist(currentTracks(), createNewPlaylist);
}

void AbstractLibrary::prepareTracksForPlaylist(const QStringList& paths, bool createNewPlaylist)
{
	createPlaylist(paths, createNewPlaylist);
}

void AbstractLibrary::playNextFetchedTracks()
{
	insertTracksAfterCurrentTrack(tracks());
}

void AbstractLibrary::playNextCurrentTracks()
{
	insertTracksAfterCurrentTrack(currentTracks());
}

void AbstractLibrary::appendFetchedTracks()
{
	appendTracks(tracks());
}

void AbstractLibrary::appendCurrentTracks()
{
	appendTracks(currentTracks());
}

void AbstractLibrary::changeArtistSelection(const IndexSet& indexes)
{
	Util::Set<ArtistId> selected_artists;
	for(int idx : indexes)
	{
		const Artist& artist = m->artists[size_t(idx)];
		selected_artists.insert(artist.id());
	}

	if(selected_artists == m->selectedArtists)
	{
		return;
	}

	m->albums.clear();
	m->tracks.clear();

	m->selectedArtists = selected_artists;

	if(m->selectedArtists.size() > 0)
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
	if(m->filteredTracks.isEmpty())
	{
		return m->tracks;
	}

	return m->filteredTracks;
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
	if(m->selectedTracks.isEmpty())
	{
		return tracks();
	}

	return m->currentTracks;
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
		m->tracks.removeTracks([disc](const MetaData& md) {
			return (md.discnumber() != disc);
		});
	}

	prepareTracks();
	emit sigAllTracksLoaded();
}

const IdSet& AbstractLibrary::selectedTracks() const
{
	return m->selectedTracks;
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
	QStringList filtertext = filter.filtertext(false);

	if(!filter.isInvalidGenre())
	{
		if(filtertext.join("").size() < 3)
		{
			filter.clear();
		}

		else
		{
			Library::SearchModeMask mask = GetSetting(Set::Lib_SearchMode);
			filter.setFiltertext(filtertext.join(","), mask);
		}
	}

	if(filter == m->filter)
	{
		return;
	}

	fetchByFilter(filter, force);
	emitAll();
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
	Util::Set<AlbumId> selected_albums;
	bool show_album_artists = GetSetting(Set::Lib_ShowAlbumArtists);

	for(auto it = indexes.begin(); it != indexes.end(); it++)
	{
		int idx = *it;
		if(idx >= m->albums.count())
		{
			break;
		}

		const Album& album = m->albums[idx];
		selected_albums.insert(album.id());
	}

	m->tracks.clear();
	m->selectedAlbums = selected_albums;

	// only show tracks of selected album / artist
	if(m->selectedArtists.size() > 0 && !ignoreArtists)
	{
		if(m->selectedAlbums.size() > 0)
		{
			MetaDataList v_md;

			getAllTracksByAlbum(m->selectedAlbums.toList(), v_md, m->filter);

			// filter by artist

			for(const MetaData& md : v_md)
			{
				ArtistId artistId;
				if(show_album_artists)
				{
					artistId = md.albumArtistId();
				}

				else
				{
					artistId = md.artistId();
				}

				if(m->selectedArtists.contains(artistId))
				{
					m->tracks << std::move(md);
				}
			}
		}

		else
		{
			getAllTracksByArtist(m->selectedArtists.toList(), m->tracks, m->filter);
		}
	}

		// only album is selected
	else if(m->selectedAlbums.size() > 0)
	{
		getAllTracksByAlbum(m->selectedAlbums.toList(), m->tracks, m->filter);
	}

		// neither album nor artist, but searchstring
	else if(!m->filter.cleared())
	{
		getAllTracksBySearchstring(m->filter, m->tracks);
	}

		// no album, no artist, no searchstring
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

	for(int idx : indexes)
	{
		if(idx < 0 || idx >= tracks().count())
		{
			continue;
		}

		const MetaData& md = tracks()[idx];

		m->currentTracks << md;
		m->selectedTracks.insert(md.id());
	}
}

void AbstractLibrary::selectedTracksChanged(const IndexSet& indexes)
{
	changeTrackSelection(indexes);
}

void AbstractLibrary::fetchByFilter(Library::Filter filter, bool force)
{
	if((m->filter == filter) &&
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

void AbstractLibrary::changeTrackSortorder(Library::SortOrder s)
{
	if(s == m->sortorder.so_tracks)
	{
		return;
	}

	Library::Sortings so = GetSetting(Set::Lib_Sorting);
	so.so_tracks = s;
	SetSetting(Set::Lib_Sorting, so);
	m->sortorder = so;

	prepareTracks();
	emit sigAllTracksLoaded();
}

void AbstractLibrary::changeAlbumSortorder(Library::SortOrder s)
{
	if(s == m->sortorder.so_albums)
	{
		return;
	}

	Library::Sortings so = GetSetting(Set::Lib_Sorting);
	so.so_albums = s;
	SetSetting(Set::Lib_Sorting, so);

	m->sortorder = so;

	prepareAlbums();
	emit sigAllAlbumsLoaded();
}

void AbstractLibrary::changeArtistSortorder(Library::SortOrder s)
{
	if(s == m->sortorder.so_artists)
	{
		return;
	}

	Library::Sortings so = GetSetting(Set::Lib_Sorting);
	so.so_artists = s;
	SetSetting(Set::Lib_Sorting, so);

	m->sortorder = so;

	prepareArtists();
	emit sigAllArtistsLoaded();
}

Library::Sortings AbstractLibrary::sortorder() const
{
	return m->sortorder;
}

void AbstractLibrary::importFiles(const QStringList& files)
{
	Q_UNUSED(files)
}

void AbstractLibrary::deleteCurrentTracks(Library::TrackDeletionMode mode)
{
	if(mode == Library::TrackDeletionMode::None)
	{
		return;
	}

	deleteTracks(currentTracks(), mode);
}

void AbstractLibrary::deleteFetchedTracks(Library::TrackDeletionMode mode)
{
	if(mode == Library::TrackDeletionMode::None)
	{
		return;
	}

	deleteTracks(tracks(), mode);
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

	QString file_entry = Lang::get(Lang::Entries);
	QString answer_str;

	int n_fails = 0;
	if(mode == Library::TrackDeletionMode::AlsoFiles)
	{
		file_entry = Lang::get(Lang::Files);

		for(const MetaData& md : tracks)
		{
			QFile f(md.filepath());
			if(!f.remove())
			{
				n_fails++;
			}
		}
	}

	if(n_fails == 0)
	{
		// all entries could be removed
		answer_str = tr("All %1 could be removed").arg(file_entry);
	}

	else
	{
		// 5 of 20 entries could not be removed
		answer_str = tr("%1 of %2 %3 could not be removed")
			.arg(n_fails)
			.arg(tracks.size())
			.arg(file_entry);
	}

	emit sigDeleteAnswer(answer_str);
	Tagging::ChangeNotifier::instance()->deleteMetadata(tracks);

	refreshCurrentView();
}

void AbstractLibrary::deleteTracksByIndex(const IndexSet& indexes, Library::TrackDeletionMode mode)
{
	if(mode == Library::TrackDeletionMode::None || indexes.isEmpty())
	{
		return;
	}

	MetaDataList tracksToDelete;
	const MetaDataList& tracks = this->tracks();
	for(auto it = indexes.begin(); it != indexes.end(); it++)
	{
		tracksToDelete.push_back(tracks[*it]);
	}

	deleteTracks(tracksToDelete, mode);
}

void AbstractLibrary::prepareTracks()
{
	m->extensions.clear();
	m->filteredTracks.clear();

	for(const MetaData& md : tracks())
	{
		m->extensions.addExtension(Util::File::getFileExtension(md.filepath()), false);
	}

	m->tracks.sort(m->sortorder.so_tracks);
}

void AbstractLibrary::prepareAlbums()
{
	m->albums.sort(m->sortorder.so_albums);
}

void AbstractLibrary::prepareArtists()
{
	m->artists.sort(m->sortorder.so_artists);
}

void AbstractLibrary::ignoreArtistArticleChanged()
{
	bool b = GetSetting(Set::Lib_SortIgnoreArtistArticle);
	MetaDataSorting::setIgnoreArticle(b);

	refreshCurrentView();
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
	if(!m->tracks.isEmpty())
	{
		return false;
	}

	return (m->trackCount == 0);
}

void AbstractLibrary::setExtensions(const Gui::ExtensionSet& extensions)
{
	m->extensions = extensions;
	m->filteredTracks.clear();

	if(m->extensions.hasEnabledExtensions())
	{
		for(const MetaData& md : m->tracks)
		{
			QString ext = ::Util::File::getFileExtension(md.filepath());
			if(m->extensions.isEnabled(ext))
			{
				m->filteredTracks << md;
			}
		}
	}

	emit sigAllTracksLoaded();
}

