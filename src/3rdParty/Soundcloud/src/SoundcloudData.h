/* SoundcloudData.h */

/* Copyright (C) 2011-2017  Lucio Carreras
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

#ifndef SOUNDCLOUDDATA_H
#define SOUNDCLOUDDATA_H

#include "Database/Base.h"
#include "Database/LibraryDatabase.h"
#include <QObject>

/* this is the database interface
 * TODO: make database connector my parent
 * TODO: create real (new) database
 */
class MetaData;
class MetaDataList;
class Artist;
class AlbumList;


namespace SC
{
	class SearchInformationList;

	class Database :
			public DB::Base,
			public DB::LibraryDatabase
	{
		Q_OBJECT

	public:
		Database();
		~Database();

		bool db_fetch_tracks(::DB::Query& q, MetaDataList& result) override;
		bool db_fetch_albums(::DB::Query& q, AlbumList& result) override;
		bool db_fetch_artists(::DB::Query& q, ArtistList& result) override;

		ArtistId updateArtist(const Artist& artist) override;
		ArtistId insertArtistIntoDatabase (const Artist& artist) override;
		ArtistId insertArtistIntoDatabase (const QString& artist) override;

		AlbumId updateAlbum(const Album& album) override;
		AlbumId insertAlbumIntoDatabase (const Album& album) override;
		AlbumId insertAlbumIntoDatabase (const QString& album) override;

		bool updateTrack(const MetaData& md) override;
		bool store_metadata(const MetaDataList& v_md) override;
		bool insertTrackIntoDatabase(const MetaData& md, int artist_id, int album_id, int album_artist_id) override;
		bool insertTrackIntoDatabase(const MetaData& md, int artist_id, int album_id) override;

		// todo: assure to be called
		bool apply_fixes() override;

		QString load_setting(const QString& key);
		bool save_setting(const QString& key, const QString& value);
		bool insert_setting(const QString& key, const QString& value);

		bool getSearchInformation(SC::SearchInformationList& list);

		QString fetch_query_albums(bool also_empty=false) const override;
		QString fetch_query_artists(bool also_empty=false) const override;
		QString fetch_query_tracks() const override;

		private:
			const DB::Module* module() const;
	};
}

#endif // SOUNDCLOUDDATA_H
