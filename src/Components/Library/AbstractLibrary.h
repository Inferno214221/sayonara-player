/* AbstractLibrary.h */

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

#ifndef ABSTRACTLIBRARY_H
#define ABSTRACTLIBRARY_H

#include "Utils/Library/LibraryNamespaces.h"
#include "Utils/Library/Filter.h"
#include "Utils/Library/Sorting.h"
#include "Utils/Pimpl.h"

#include <QFile>

class Genre;
namespace Gui
{
	class ExtensionSet;
}

class LibraryPlaylistInteractor;

class AbstractLibrary :
	public QObject
{
	Q_OBJECT
	PIMPL(AbstractLibrary)

	signals:
		void sigAllTracksLoaded();
		void sigAllAlbumsLoaded();
		void sigAllArtistsLoaded();

		void sigReloadingLibrary(const QString& message, int progress);
		void sigReloadingLibraryFinished();

		void sigDeleteAnswer(QString);

		void sigCurrentAlbumChanged(int row);
		void sigCurrentTrackChanged(int row);

	public:
		explicit AbstractLibrary(LibraryPlaylistInteractor* playlistInteractor, QObject* parent = nullptr);
		~AbstractLibrary() override;

		virtual void init();

		[[nodiscard]] Library::Sortings sortorder() const;
		[[nodiscard]] Library::Filter filter() const;
		// calls fetch_by_filter and emits
		void changeFilter(Library::Filter, bool force = false);

		[[nodiscard]] const MetaDataList& tracks() const;
		[[nodiscard]] const AlbumList& albums() const;
		[[nodiscard]] const ArtistList& artists() const;
		[[nodiscard]] const MetaDataList& currentTracks() const;

		[[nodiscard]] const Util::Set<AlbumId>& selectedAlbums() const;
		[[nodiscard]] const Util::Set<ArtistId>& selectedArtists() const;

		// emits new tracks, very similar to psl_selected_albums_changed
		void changeCurrentDisc(Disc track);

		[[nodiscard]] bool isLoaded() const;

		void setExtensions(const Gui::ExtensionSet& extensions);
		[[nodiscard]] Gui::ExtensionSet extensions() const;

		[[nodiscard]] virtual bool isReloading() const;
		[[nodiscard]] virtual bool isEmpty() const;

		[[nodiscard]] LibraryPlaylistInteractor* playlistInteractor() const;

	public slots: // NOLINT(*-redundant-access-specifiers)
		virtual void reloadLibrary(bool clear_first, Library::ReloadQuality quality) = 0;

		virtual void refetch();

		virtual void refreshCurrentView();

		virtual void findTrack(TrackID id);

		virtual void selectedArtistsChanged(const IndexSet& indexes);
		virtual void selectedAlbumsChanged(const IndexSet& indexes, bool ignore_artists = false);
		virtual void selectedTracksChanged(const IndexSet& indexes);

		// Those two functions are identical (1) calls (2)
		virtual void prepareCurrentTracksForPlaylist(bool new_playlist);
		virtual void prepareFetchedTracksForPlaylist(bool new_playlist);
		void prepareTracksForPlaylist(const QStringList& file_paths, bool new_playlist);

		/* append tracks after current played track in playlist */
		virtual void playNextFetchedTracks();
		virtual void playNextCurrentTracks();

		/* append tracks after last track in playlist */
		virtual void appendFetchedTracks();
		virtual void appendCurrentTracks();

		/* a searchfilter has been entered, nothing is emitted */
		virtual void fetchByFilter(const Library::Filter& filter, bool force);
		virtual void fetchTracksByPath(const QStringList& paths);

		virtual void deleteTracks(const MetaDataList& tracks, Library::TrackDeletionMode mode) = 0;

		virtual void deleteFetchedTracks(Library::TrackDeletionMode mode);
		virtual void deleteCurrentTracks(Library::TrackDeletionMode mode);
		virtual void deleteAllTracks();

		virtual void importFiles(const QStringList& files);

		virtual void changeTrackSortorder(Library::TrackSortorder sortOrder);
		virtual void changeAlbumSortorder(Library::AlbumSortorder sortOrder);
		virtual void changeArtistSortorder(Library::ArtistSortorder sortOrder);

		virtual void refreshArtists() = 0;
		virtual void refreshAlbums() = 0;
		virtual void refreshTracks() = 0;

	protected:
		/* Emit 3 signals with shown artists, shown album, shown tracks */
		virtual void initLibraryImpl() = 0;
		virtual void emitAll();

		virtual void getAllArtists(ArtistList& artists) const = 0;
		virtual void getAllArtistsBySearchstring(Library::Filter filter, ArtistList& artists) const = 0;

		virtual void getAllAlbums(AlbumList& albums) const = 0;
		virtual void getAllAlbumsByArtist(IdList artistIds, AlbumList& albums, Library::Filter filter) const = 0;
		virtual void getAllAlbumsBySearchstring(Library::Filter filter, AlbumList& albums) const = 0;

		[[nodiscard]] virtual int getTrackCount() const = 0;
		virtual void getAllTracks(MetaDataList& tracks) const = 0;
		virtual void getAllTracks(const QStringList& paths, MetaDataList& tracks) const = 0;
		virtual void getAllTracksByArtist(IdList artistIds, MetaDataList& tracks, Library::Filter filter) const = 0;
		virtual void getAllTracksByAlbum(IdList albumIds, MetaDataList& tracks, Library::Filter filter) const = 0;
		virtual void getAllTracksBySearchstring(Library::Filter filter, MetaDataList& tracks) const = 0;
		virtual void getAllTracksByPath(const QStringList& paths, MetaDataList& tracks) const = 0;

		virtual void getTrackById(TrackID trackId, MetaData& md) const = 0;
		virtual void getAlbumById(AlbumId albumId, Album& album) const = 0;
		virtual void getArtistById(ArtistId artistId, Artist& artist) const = 0;

		void replaceAlbum(int index, const Album& album);
		void replaceTrack(int index, const MetaData& track);

		void prepareTracks();
		void prepareAlbums();
		void prepareArtists();

	private:
		void changeTrackSelection(const IndexSet& indexes);
		void changeArtistSelection(const IndexSet& indexes);
		void changeAlbumSelection(const IndexSet& indexes, bool ignore_artists = false);
};

#endif // ABSTRACTLIBRARY_H
