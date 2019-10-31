/* UserTaggingOperations.h */

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

#ifndef USERTAGGINGOPERATIONS_H
#define USERTAGGINGOPERATIONS_H

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
		void sig_finished();
		void sig_progress(int);

	public:
		UserOperations(LibraryId library_id, QObject* parent=nullptr);
		~UserOperations();

		void set_track_rating(const MetaData& md, Rating rating);
		void set_track_rating(const MetaDataList& v_md, Rating rating);

		void set_album_rating(const Album& album, Rating rating);

		void merge_artists(const Util::Set<Id>& artist_ids, ArtistId target_artist_id);
		void merge_albums(const Util::Set<Id>& albums_ids, AlbumId target_album_id);

		void add_genre(const IdSet ids, const Genre& genre);
		void delete_genre(const Genre& genre);
		void rename_genre(const Genre& genre, const Genre& new_genre);
		void add_genre_to_md(const MetaDataList& v_md, const Genre& genre);

	private:
		Editor* create_editor();
		void run_editor(Editor* editor);
	};
}

#endif // USERTAGGINGOPERATIONS_H
