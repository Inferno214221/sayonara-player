/* GenreFetcher.cpp */

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

#include "GenreFetcher.h"

#include "Components/Library/LocalLibrary.h"
#include "Components/Tagging/ChangeNotifier.h"
#include "Components/Tagging/UserTaggingOperations.h"

#include "Database/Connector.h"
#include "Database/LibraryDatabase.h"

#include "Utils/Library/LibraryInfo.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Set.h"

#include <QTimer>

struct GenreFetcher::Private
{
	LocalLibrary*					localLibrary=nullptr;
	Util::Set<Genre>				genres;
	Util::Set<Genre>				additionalGenres; // empty genres that are inserted
	Tagging::UserOperations*		uto=nullptr;
};

GenreFetcher::GenreFetcher(QObject* parent) :
	QObject(parent)
{
	m = Pimpl::make<Private>();

	Tagging::ChangeNotifier* mcn = Tagging::ChangeNotifier::instance();

	connect(mcn, &Tagging::ChangeNotifier::sigMetadataChanged, this, &GenreFetcher::reloadGenres);
	connect(mcn, &Tagging::ChangeNotifier::sigMetadataDeleted, this, &GenreFetcher::reloadGenres);
}

Tagging::UserOperations* GenreFetcher::initTagging()
{
	if(!m->uto)
	{
		m->uto = new Tagging::UserOperations(-1, this);
		connect(m->uto, &Tagging::UserOperations::sigProgress, this, &GenreFetcher::sigProgress);
		connect(m->uto, &Tagging::UserOperations::sigFinished, this, &GenreFetcher::sigFinished);
	}

	return m->uto;
}

GenreFetcher::~GenreFetcher() = default;

void GenreFetcher::reloadGenres()
{
	if(!m->localLibrary){
		return;
	}

	LibraryId libraryId = m->localLibrary->info().id();

	DB::LibraryDatabase* lib_db = DB::Connector::instance()->libraryDatabase(libraryId, 0);
	m->genres = lib_db->getAllGenres();

	emit sigGenresFetched();
}

Util::Set<Genre> GenreFetcher::genres() const
{
	Util::Set<Genre> genres(m->genres);
	for(const Genre& genre : m->additionalGenres)
	{
		genres.insert(genre);
	}

	return genres;
}

void GenreFetcher::createGenre(const Genre& genre)
{
	m->additionalGenres << genre;
	emit sigGenresFetched();
}

void GenreFetcher::applyGenreToMetadata(const MetaDataList& v_md, const Genre& genre)
{
	Tagging::UserOperations* uto = initTagging();
	uto->applyGenreToMetadata(v_md, genre);
}

void GenreFetcher::deleteGenre(const Genre& genre)
{
	Tagging::UserOperations* uto = initTagging();
	uto->deleteGenre(genre);
}

void GenreFetcher::deleteGenres(const Util::Set<Genre>& genres)
{
	for(const Genre& genre : genres)
	{
		deleteGenre(genre);
	}
}

void GenreFetcher::renameGenre(const Genre& oldGenre, const Genre& newGenre)
{
	Tagging::UserOperations* uto = initTagging();
	uto->renameGenre(oldGenre, newGenre);
}

void GenreFetcher::set_local_library(LocalLibrary* local_library)
{
	m->localLibrary = local_library;
	connect(m->localLibrary, &LocalLibrary::sigReloadingLibraryFinished,
			this, &GenreFetcher::reloadGenres);

	QTimer::singleShot(200, this, SLOT(reloadGenres()));
}
