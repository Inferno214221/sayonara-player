/* DatabaseTracks.h */

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

#ifndef DATABASETRACKS_H
#define DATABASETRACKS_H

#include "Database/SearchableModule.h"
#include "Utils/Pimpl.h"

#include <QMap>

namespace Library
{
	class Filter;
}

class Genre;

namespace DB
{
	class Tracks
	{
		public:
			Tracks();
			virtual ~Tracks();

			void initViews();

			virtual bool dbFetchTracks(Query& q, MetaDataList& result) const;

			virtual int getNumTracks() const;
			virtual bool getAllTracks(MetaDataList& result) const;

            virtual bool getAllTracksByAlbum(const IdList& albumsIds, MetaDataList& result) const;
			virtual bool getAllTracksByAlbum(const IdList& albumIds, MetaDataList& result,
                                             const ::Library::Filter& filter, int discnumber) const;
			virtual bool getAllTracksByArtist(const IdList& artistIds, MetaDataList& result) const;
			virtual bool getAllTracksByArtist(const IdList& artistIds, MetaDataList& result,
                                              const ::Library::Filter& filter) const;
			virtual bool getAllTracksBySearchString(const ::Library::Filter& filter, MetaDataList& result) const;
			virtual bool getAllTracksByPaths(const QStringList& paths, MetaDataList& v_md) const;

			virtual MetaData	getTrackById(TrackID id) const;
			virtual bool		getTracksByIds(const QList<TrackID> &ids, MetaDataList &v_md) const;
			virtual MetaData	getTrackByPath(const QString& path) const;
			virtual bool		getMultipleTracksByPath(const QStringList& paths, MetaDataList& v_md) const;

			virtual bool insertTrackIntoDatabase(const MetaData& data, ArtistId artistId, AlbumId albumId);
			virtual bool insertTrackIntoDatabase(const MetaData& data, ArtistId artistId, AlbumId albumId, ArtistId album_artistId);
			virtual bool updateTrack(const MetaData& data);
			virtual bool updateTracks(const MetaDataList& lst);

			virtual bool renameFilepaths(const QMap<QString, QString>& paths, LibraryId libraryId);
			virtual bool renameFilepath(const QString& old_path, const QString& new_path, LibraryId libraryId);

			virtual bool deleteTrack(TrackID id);
			virtual bool deleteTracks(const MetaDataList&);
			virtual bool deleteTracks(const IdList& ids);


			// some tracks may be inserted two times
			// this function deletes BOTH copies but returns those tracks
			// which were found twice. Those tracks should be inserted by the store_metadata()
			// function of LibraryDatabase
			virtual bool deleteInvalidTracks(const QString& library_path, MetaDataList& double_metadata);

			virtual QString fetchQueryTracks() const;

			virtual Util::Set<Genre> getAllGenres() const;

			void deleteAllTracks(bool also_views);

		protected:
			virtual QString artistIdField() const=0;
			virtual QString artistNameField() const=0;
			virtual QString trackView() const=0;
			virtual QString trackSearchView() const=0;
			virtual ::Library::SearchModeMask searchMode() const=0;
			virtual LibraryId libraryId() const=0;

			virtual Module* module()=0;
			virtual const Module* module() const=0;

			virtual void updateTrackCissearch();
		};
}

#endif // DATABASETRACKS_H
