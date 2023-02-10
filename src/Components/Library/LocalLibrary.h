/* LocalLibrary.h */

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

#ifndef SAYONARA_PLAYER_LOCAL_LIBRARY_H
#define SAYONARA_PLAYER_LOCAL_LIBRARY_H

#include "AbstractLibrary.h"
#include "Importer/LibraryImporter.h"
#include "Utils/Pimpl.h"

class ReloadThread;

namespace Library
{
	class Info;
	class Manager;
}

class LocalLibrary :
	public AbstractLibrary
{
		friend class Library::Manager;

	Q_OBJECT
	PIMPL(LocalLibrary)

	signals:
		void sigImportDialogRequested(const QString& targetDirectory);
		void sigRenamed(const QString& newName);
		void sigPathChanged(const QString& newPath);

	public:
		LocalLibrary(Library::Manager* libraryManager, LibraryId id, LibraryPlaylistInteractor* playlistInteractor,
		             QObject* parent = nullptr);
		~LocalLibrary() override;

		bool setLibraryPath(const QString& path);
		bool setLibraryName(const QString& name);

		[[nodiscard]] Library::Info info() const;
		[[nodiscard]] Library::Importer* importer();
		[[nodiscard]] bool isReloading() const override;

	public slots: // NOLINT(readability-redundant-access-specifiers)
		void deleteTracks(const MetaDataList& tracks, Library::TrackDeletionMode answer) override;
		void reloadLibrary(bool clear_first, Library::ReloadQuality quality) override;
		void importFiles(const QStringList& files) override;
		void importFilesTo(const QStringList& files, const QString& targetDirectory);

	protected:
		void initLibraryImpl() override;

	private:
		void applyDatabaseFixes();
		void initReloadThread();

		void getAllArtists(ArtistList& artists) const override;
		void getAllArtistsBySearchstring(Library::Filter filter, ArtistList& artists) const override;

		void getAllAlbums(AlbumList& albums) const override;
		void getAllAlbumsByArtist(IdList artistIds, AlbumList& albums, Library::Filter filter) const override;
		void getAllAlbumsBySearchstring(Library::Filter filter, AlbumList& albums) const override;

		[[nodiscard]] int getTrackCount() const override;
		void getAllTracks(MetaDataList& v_md) const override;
		void getAllTracks(const QStringList& paths, MetaDataList& tracks) const override;
		void getAllTracksByArtist(IdList artistIds, MetaDataList& tracks, Library::Filter filter) const override;
		void getAllTracksByAlbum(IdList albumIds, MetaDataList& tracks, Library::Filter filter) const override;
		void getAllTracksBySearchstring(Library::Filter filter, MetaDataList& v_md) const override;
		void getAllTracksByPath(const QStringList& paths, MetaDataList& v_md) const override;

		void getTrackById(TrackID trackId, MetaData& track) const override;
		void getAlbumById(AlbumId albumId, Album& album) const override;
		void getArtistById(ArtistId artistId, Artist& artist) const override;

		// not needed
		void refreshArtists() override;
		void refreshAlbums() override;
		void refreshTracks() override;

	private slots: // NOLINT(readability-redundant-access-specifiers)
		void reloadThreadNewBlock();
		void reloadThreadFinished();
		void searchModeChanged();
		void showAlbumArtistsChanged();
		void importStatusChanged(Library::Importer::ImportStatus status);
		void metadataChanged();
		void albumsChanged();
};

#endif // SAYONARA_PLAYER_LOCAL_LIBRARY_H
