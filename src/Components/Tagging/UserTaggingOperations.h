#ifndef USERTAGGINGOPERATIONS_H
#define USERTAGGINGOPERATIONS_H

#include <QObject>
#include "Utils/Pimpl.h"

class Genre;

namespace Tagging
{
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

		void merge_artists(const Util::Set<Id>& artist_ids, ArtistId target_artist_id);
		void merge_albums(const Util::Set<Id>& albums_ids, AlbumId target_album_id);

		void add_genre(const IdSet ids, const Genre& genre);
		void delete_genre(const Genre& genre);
		void rename_genre(const Genre& genre, const Genre& new_genre);
		void add_genre_to_md(const MetaDataList& v_md, const Genre& genre);
	};
}

#endif // USERTAGGINGOPERATIONS_H
