#ifndef SOUNDCLOUDLIBRARYDATABASE_H
#define SOUNDCLOUDLIBRARYDATABASE_H

#include "Database/LibraryDatabase.h"

namespace SC
{
	class SearchInformationList;

	class LibraryDatabase : public ::DB::LibraryDatabase
	{
	public:
		LibraryDatabase(const QString& connection_name, DbId db_id, LibraryId library_id);
		~LibraryDatabase() override;

		QString fetch_query_albums(bool also_empty=false) const override;
		QString fetch_query_artists(bool also_empty=false) const override;
		QString fetch_query_tracks() const override;

		bool db_fetch_tracks(::DB::Query& q, MetaDataList& result) const override;
		bool db_fetch_albums(::DB::Query& q, AlbumList& result) const override;
		bool db_fetch_artists(::DB::Query& q, ArtistList& result) const override;

		ArtistId updateArtist(const Artist& artist);
		ArtistId insertArtistIntoDatabase (const Artist& artist) override;
		ArtistId insertArtistIntoDatabase (const QString& artist) override;

		AlbumId updateAlbum(const Album& album);
		AlbumId insertAlbumIntoDatabase (const Album& album) override;
		AlbumId insertAlbumIntoDatabase (const QString& album) override;

		bool updateTrack(const MetaData& md) override;
		bool store_metadata(const MetaDataList& v_md) override;
		bool insertTrackIntoDatabase(const MetaData& md, int artist_id, int album_id, int album_artist_id) override;
		bool insertTrackIntoDatabase(const MetaData& md, int artist_id, int album_id) override;

		bool search_information(SC::SearchInformationList& list);
	};
}

#endif // SOUNDCLOUDLIBRARYDATABASE_H
