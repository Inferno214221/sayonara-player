#ifndef SOUNDCLOUDLIBRARYDATABASE_H
#define SOUNDCLOUDLIBRARYDATABASE_H

#include "Database/LibraryDatabase.h"

namespace SC
{
	class SearchInformationList;

	class LibraryDatabase : public ::DB::LibraryDatabase
	{
		public:
			LibraryDatabase(const QString& connectionName, DbId databaseId, LibraryId libraryId);
			~LibraryDatabase() override;

			QString fetchQueryAlbums(bool alsoEmpty = false) const override;
			QString fetchQueryArtists(bool alsoEmpty = false) const override;
			QString fetchQueryTracks(const QString& where) const override;

			bool dbFetchTracks(::DB::Query& query, MetaDataList& result) const override;
			bool dbFetchAlbums(::DB::Query& query, AlbumList& result) const override;
			bool dbFetchArtists(::DB::Query& query, ArtistList& result) const override;

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
