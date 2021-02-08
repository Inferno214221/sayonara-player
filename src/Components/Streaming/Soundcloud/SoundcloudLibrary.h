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

	signals:
		// called, when webservice returns artists/albums/tracks
		void sigArtistsFound(const ArtistList& artists);
		void sigAlbumsFound(const AlbumList& albums);
		void sigTracksFound(const MetaDataList& v_md);

	public:
		explicit Library(Playlist::Handler* playlistHandler, QObject* parent=nullptr);
		~Library() override;

		void	load() override;
		void	searchArtist(const QString& artist_name);
		void	fetchTracksByArtist(int64_t artist_sc_id);
		void	fetchPlaylistsByArtist(int64_t artist_sc_id);
		//void	insert_tracks(const MetaDataList& v_md) override;
		void	insertTracks(const MetaDataList& v_md, const ArtistList& artists, const AlbumList& albums);
		void	getTrackById(TrackID trackId, MetaData& md) const override;
		void	getAlbumById(AlbumId albumId, Album& album) const override;
		void  	getArtistById(ArtistId artistId, Artist& artist) const override;

	protected:
		void	getAllArtists(ArtistList& artists) const override;
		void	getAllArtistsBySearchstring(::Library::Filter filter, ArtistList& artists) const override;

		void	getAllAlbums(AlbumList& albums) const override;
		void	getAllAlbumsByArtist(IdList artistIds, AlbumList& albums, ::Library::Filter filter) const override;
		void	getAllAlbumsBySearchstring(::Library::Filter filter, AlbumList& albums) const override;

		int		getTrackCount() const override;
		void	getAllTracks(const QStringList& paths, MetaDataList& v_md) const override;
		void	getAllTracks(MetaDataList& v_md) const override;
		void	getAllTracksByArtist(IdList artistIds, MetaDataList& v_md, ::Library::Filter filter) const override;
		void	getAllTracksByAlbum(IdList albumIds, MetaDataList& v_md, ::Library::Filter filter) const override;
		void	getAllTracksBySearchstring(::Library::Filter filter, MetaDataList& v_md) const override;
		void	getAllTracksByPath(const QStringList& paths, MetaDataList& v_md) const override;

		void	updateTrack(const MetaData& md);
		void	updateAlbum(const Album& album);
		void	deleteTracks(const MetaDataList& v_md, ::Library::TrackDeletionMode mode) override;

		void    refetch() override;

		void	applyArtistAndAlbumToMetadata();

	private slots:
		void	artistsFetched(const ArtistList& artists);
		void	tracksFetched(const MetaDataList& v_md);
		void	albumsFetched(const AlbumList& albums);
		void	coverFound(const Cover::Location& cl);

	public slots:
		void	reloadLibrary(bool clear_first, ::Library::ReloadQuality quality) override;
		void	refreshArtists() override;
		void	refreshAlbums() override;
		void	refreshTracks() override;
	};
}

#endif // LocalLibrary_H
