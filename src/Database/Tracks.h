/* DatabaseTracks.h */

/* Copyright (C) 2011-2019  Lucio Carreras
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

#ifndef DATABASETRACKS_H
#define DATABASETRACKS_H

#include "Database/SearchableModule.h"
#include "Utils/Pimpl.h"

namespace Library
{
	class Filter;
}

class Genre;

namespace DB
{
	class Tracks :
			private SearchableModule
	{
		PIMPL(Tracks)

		public:
			Tracks(const QString& connection_name, DbId db_id, LibraryId _library_id);
			~Tracks() override;

			virtual bool db_fetch_tracks(Query& q, MetaDataList& result);

            virtual bool getAllTracksByAlbum(const IdList& albumsIds, MetaDataList& result);
			virtual bool getAllTracksByAlbum(const IdList& albumIds, MetaDataList& result,
                                             const ::Library::Filter& filter, int discnumber);

			virtual bool getAllTracksByArtist(const IdList& artistIds, MetaDataList& result);
			virtual bool getAllTracksByArtist(const IdList& artistIds, MetaDataList& result,
                                              const ::Library::Filter& filter);

			virtual bool getAllTracksBySearchString(const ::Library::Filter& filter, MetaDataList& result);

			virtual bool insertTrackIntoDatabase(const MetaData& data, ArtistId artist_id, AlbumId album_id);
			virtual bool insertTrackIntoDatabase(const MetaData& data, ArtistId artist_id, AlbumId album_id, ArtistId album_artist_id);
			virtual bool updateTrack(const MetaData& data);
			virtual bool updateTracks(const MetaDataList& lst);

			virtual bool getAllTracks(MetaDataList& result);

			virtual MetaData getTrackById(int id);
			virtual bool getTracksByIds(const QList<TrackID> &ids, MetaDataList &v_md);
			virtual MetaData getTrackByPath(const QString& path);
			virtual bool getMultipleTracksByPath(const QStringList& paths, MetaDataList& v_md);

			virtual bool deleteTrack(TrackID id);
			virtual bool deleteTracks(const MetaDataList&);
			virtual bool deleteTracks(const IdList& ids);


			// some tracks may be inserted two times
			// this function deletes BOTH copies but returns those tracks
			// which were found twice. Those tracks should be inserted by the store_metadata()
			// function of LibraryDatabase
			virtual bool deleteInvalidTracks(const QString& library_path, MetaDataList& double_metadata);

			virtual QString fetch_query_tracks() const;

			virtual Util::Set<Genre> getAllGenres();
			virtual void updateTrackCissearch();

			void deleteAllTracks(bool also_views);
			void drop_track_view();
			void drop_search_view();

		protected:
			virtual QString artistid_field() const=0;
			virtual QString artistname_field() const=0;

		private:
			void create_track_view(const QString& select_statement);
			void create_track_search_view(const QString& select_statement);
		};
}

#endif // DATABASETRACKS_H
