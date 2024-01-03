/* SoundcloudLibraryDatabase.h
 *
 * Copyright (C) 2011-2024 Michael Lugmair
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

#ifndef SOUNDCLOUDLIBRARYDATABASE_H
#define SOUNDCLOUDLIBRARYDATABASE_H

#include "Database/LibraryDatabase.h"

namespace SC
{
	class SearchInformationList;

	class LibraryDatabase :
		public ::DB::LibraryDatabase
	{
		public:
			LibraryDatabase(const QString& connectionName, DbId databaseId, LibraryId libraryId);
			~LibraryDatabase() override;

			QString fetchQueryAlbums(bool alsoEmpty = false) const override;
			QString fetchQueryArtists(bool alsoEmpty = false) const override;
			QString fetchQueryTracks(const QString& where) const override;

			bool dbFetchTracks(QSqlQuery& query, MetaDataList& result) const override;
			bool dbFetchAlbums(QSqlQuery& query, AlbumList& result) const override;
			bool dbFetchArtists(QSqlQuery& query, ArtistList& result) const override;

			ArtistId updateArtist(const Artist& artist);
			ArtistId insertArtistIntoDatabase(const Artist& artist) override;
			ArtistId insertArtistIntoDatabase(const QString& artist) override;

			bool getAllAlbums(AlbumList& result, bool alsoEmpty) const override;
			AlbumId updateAlbum(const Album& album);
			AlbumId insertAlbumIntoDatabase(const Album& album) override;
			AlbumId insertAlbumIntoDatabase(const QString& album) override;

			bool updateTrack(const MetaData& track) override;
			bool storeMetadata(const MetaDataList& tracks) override;

			bool insertTrackIntoDatabase(const MetaData& track, int artistId, int albumId, int albumArtistId) override;

			bool searchInformation(SC::SearchInformationList& searchInformation);
	};
}

#endif // SOUNDCLOUDLIBRARYDATABASE_H
