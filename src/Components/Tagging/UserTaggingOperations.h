/* UserTaggingOperations.h */

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

#ifndef SAYONARA_PLAYER_USERTAGGINGOPERATIONS_H
#define SAYONARA_PLAYER_USERTAGGINGOPERATIONS_H

#include <QObject>
#include "Utils/Pimpl.h"

class Genre;

namespace Tagging
{
	class Editor;
	class UserOperations :
		public QObject
	{
		Q_OBJECT
		PIMPL(UserOperations)

		signals:
			void sigFinished();
			void sigProgress(int);

		public:
			UserOperations(LibraryId libraryId, QObject* parent = nullptr);
			~UserOperations() override;

			void setTrackRating(const MetaData& md, Rating rating);
			void setTrackRating(const MetaDataList& tracks, Rating rating);
			Rating oldRating(TrackID trackId) const;
			Rating newRating(TrackID trackId) const;

			void setAlbumRating(const Album& album, Rating rating);

			void mergeArtists(const Util::Set<Id>& artisIids, ArtistId targetArtistId);
			void mergeAlbums(const Util::Set<Id>& albumsIds, AlbumId targetAlbumId);

			void addGenre(const IdSet ids, const Genre& genre);
			void deleteGenres(const Util::Set<Genre>& genres);
			void renameGenre(const Genre& genre, const Genre& newGenre);
			void applyGenreToMetadata(const MetaDataList& tracks, const Genre& genre);

		private:
			Editor* createEditor();
			void runEditor(Editor* editor);
	};
}

#endif // SAYONARA_PLAYER_USERTAGGINGOPERATIONS_H
