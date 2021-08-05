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

		QString fetchQueryAlbums(bool also_empty=false) const override;
		QString fetchQueryArtists(bool also_empty=false) const override;
		QString fetchQueryTracks(const QString& where) const override;

		bool dbFetchTracks(::DB::Query& q, MetaDataList& result) const override;
		bool dbFetchAlbums(::DB::Query& q, AlbumList& result) const override;
		bool dbFetchArtists(::DB::Query& q, ArtistList& result) const override;

		ArtistId updateArtist(const Artist& artist);
		ArtistId insertArtistIntoDatabase (const Artist& artist) override;
		ArtistId insertArtistIntoDatabase (const QString& artist) override;

		bool getAllAlbums(AlbumList& result, bool alsoEmpty) const override;
		AlbumId updateAlbum(const Album& album);
		AlbumId insertAlbumIntoDatabase (const Album& album) override;
		AlbumId insertAlbumIntoDatabase (const QString& album) override;

		bool updateTrack(const MetaData& md) override;
		bool storeMetadata(const MetaDataList& v_md) override;

		bool insertTrackIntoDatabase(const MetaData& md, int artistId, int albumId, int albumArtistId) override;

		bool searchInformation(SC::SearchInformationList& list);
	};
}

#endif // SOUNDCLOUDLIBRARYDATABASE_H
