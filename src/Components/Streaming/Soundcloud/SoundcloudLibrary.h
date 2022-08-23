/* SoundcloudLibrary.h */

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

#ifndef SOUNDCLOUD_H
#define SOUNDCLOUD_H

#include "SoundcloudData.h"
#include "Components/Library/AbstractLibrary.h"
#include "Utils/Library/Sorting.h"
#include "Utils/Pimpl.h"

namespace Cover
{
	class Location;
}

namespace SC
{
	class Library :
		public AbstractLibrary
	{
		Q_OBJECT
		PIMPL(Library)

		public:
			explicit Library(LibraryPlaylistInteractor* playlistInteractor, QObject* parent = nullptr);
			~Library() override;
			
			void insertTracks(const MetaDataList& tracks, const ArtistList& artists, const AlbumList& albums);
			void getTrackById(TrackID trackId, MetaData& track) const override;
			void getAlbumById(AlbumId albumId, Album& album) const override;
			void getArtistById(ArtistId artistId, Artist& artist) const override;

			void refetch() override;

		protected:
			virtual void initLibraryImpl() override;

			void getAllArtists(ArtistList& artists) const override;
			void getAllArtistsBySearchstring(::Library::Filter filter, ArtistList& artists) const override;

			void getAllAlbums(AlbumList& albums) const override;
			void getAllAlbumsByArtist(IdList artistIds, AlbumList& albums, ::Library::Filter filter) const override;
			void getAllAlbumsBySearchstring(::Library::Filter filter, AlbumList& albums) const override;

			[[nodiscard]] int getTrackCount() const override;
			void getAllTracks(const QStringList& paths, MetaDataList& tracks) const override;
			void getAllTracks(MetaDataList& tracks) const override;
			void getAllTracksByArtist(IdList artistIds, MetaDataList& tracks, ::Library::Filter filter) const override;
			void getAllTracksByAlbum(IdList albumIds, MetaDataList& tracks, ::Library::Filter filter) const override;
			void getAllTracksBySearchstring(::Library::Filter filter, MetaDataList& tracks) const override;
			void getAllTracksByPath(const QStringList& paths, MetaDataList& tracks) const override;

			void updateTrack(const MetaData& track);
			void deleteTracks(const MetaDataList& tracks, ::Library::TrackDeletionMode mode) override;

		private slots:
			void artistsFetched(const ArtistList& artists);
			void tracksFetched(const MetaDataList& tracks);
			void albumsFetched(const AlbumList& albums);
			void coverFound(const Cover::Location& coverLocation);

		public slots:
			void reloadLibrary(bool clear_first, ::Library::ReloadQuality quality) override;
			void refreshArtists() override;
			void refreshAlbums() override;
			void refreshTracks() override;
	};
}

#endif // LocalLibrary_H
