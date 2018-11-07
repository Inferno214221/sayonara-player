/* GenreFetcher.cpp */

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

#include "GenreFetcher.h"

#include "Components/Library/LocalLibrary.h"
#include "Components/Tagging/ChangeNotifier.h"
#include "Components/Tagging/UserTaggingOperations.h"

#include "Database/Connector.h"
#include "Database/LibraryDatabase.h"

#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Set.h"

#include <QTimer>

struct GenreFetcher::Private
{
	LocalLibrary*					local_library=nullptr;
	Util::Set<Genre>				genres;
	Util::Set<Genre>				additional_genres; // empty genres that are inserted
	Tagging::UserOperations* uto=nullptr;

	Private(GenreFetcher* parent)
	{
		uto = new Tagging::UserOperations(-1, parent);
	}

	DB::LibraryDatabase* get_local_library_db()
	{
		if(!local_library){
			return nullptr;
		}

		LibraryId library_id = local_library->library_id();

		DB::Connector* db = DB::Connector::instance();
		DB::LibraryDatabase* lib_db = db->library_db(library_id, 0);

		return lib_db;
	}
};

GenreFetcher::GenreFetcher(QObject* parent) :
	QObject(parent)
{
	m = Pimpl::make<Private>(this);

	Tagging::ChangeNotifier* mcn = Tagging::ChangeNotifier::instance();

	connect(mcn, &Tagging::ChangeNotifier::sig_metadata_changed, this, &GenreFetcher::metadata_changed);
	connect(mcn, &Tagging::ChangeNotifier::sig_metadata_deleted, this, &GenreFetcher::metadata_deleted);

	connect(m->uto, &Tagging::UserOperations::sig_progress, this, &GenreFetcher::sig_progress);
	connect(m->uto, &Tagging::UserOperations::sig_finished, this, &GenreFetcher::sig_finished);
}

GenreFetcher::~GenreFetcher() {}

void GenreFetcher::reload_genres()
{
	DB::LibraryDatabase* db = m->get_local_library_db();
	if(!db){
		return;
	}

	m->genres = db->getAllGenres();

	emit sig_genres_fetched();
}

Util::Set<Genre> GenreFetcher::genres() const
{
	Util::Set<Genre> genres(m->genres);
	for(const Genre& genre : m->additional_genres)
	{
		genres.insert(genre);
	}

	return genres;
}

void GenreFetcher::metadata_changed(const MetaDataList& v_md_old, const MetaDataList& v_md_new)
{
	Q_UNUSED(v_md_old)
	Q_UNUSED(v_md_new)

	reload_genres();
}

void GenreFetcher::metadata_deleted(const MetaDataList& v_md_deleted)
{
	Q_UNUSED(v_md_deleted)

	reload_genres();
}

void GenreFetcher::create_genre(const Genre& genre)
{
	m->additional_genres << genre;
	emit sig_genres_fetched();
}

void GenreFetcher::add_genre_to_md(const MetaDataList& v_md, const Genre& genre)
{
	m->uto->add_genre_to_md(v_md, genre);
}

void GenreFetcher::delete_genre(const Genre& genre)
{
	m->uto->delete_genre(genre);
}

void GenreFetcher::rename_genre(const Genre& old_genre, const Genre& new_genre)
{
	m->uto->rename_genre(old_genre, new_genre);
}

void GenreFetcher::set_local_library(LocalLibrary* local_library)
{
	m->local_library = local_library;
	connect(m->local_library, &LocalLibrary::sig_reloading_library_finished,
			this, &GenreFetcher::reload_genres);

	QTimer::singleShot(200, this, SLOT(reload_genres()));
}
