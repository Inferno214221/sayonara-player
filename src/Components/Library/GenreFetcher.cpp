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
	LocalLibrary* localLibrary = nullptr;
	Util::Set<Genre> genres;
	Util::Set<Genre> additionalGenres; // empty genres that are inserted
	bool taggingInProgress {false};
};

GenreFetcher::GenreFetcher(QObject* parent) :
	QObject(parent),
	m {Pimpl::make<Private>()}
{
	auto* tagChangeNotifier = Tagging::ChangeNotifier::instance();

	connect(tagChangeNotifier, &Tagging::ChangeNotifier::sigMetadataChanged, this, &GenreFetcher::reloadGenres);
	connect(tagChangeNotifier, &Tagging::ChangeNotifier::sigMetadataDeleted, this, &GenreFetcher::reloadGenres);
}

Tagging::UserOperations* GenreFetcher::initTagging()
{
	m->taggingInProgress = true;

	auto* userOperation = new Tagging::UserOperations(-1, this);

	connect(userOperation, &Tagging::UserOperations::sigProgress, this, &GenreFetcher::sigProgress);
	connect(userOperation, &Tagging::UserOperations::sigFinished, this, &GenreFetcher::taggingOperationsFinished);

	return userOperation;
}

void GenreFetcher::taggingOperationsFinished()
{
	m->taggingInProgress = false;

	emit sigFinished();

	sender()->deleteLater();
}

GenreFetcher::~GenreFetcher() = default;

void GenreFetcher::reloadGenres()
{
	if(m->localLibrary)
	{
		const auto libraryId = m->localLibrary->info().id();
		auto* libraryDatabase = DB::Connector::instance()->libraryDatabase(libraryId, 0);
		m->genres = libraryDatabase->getAllGenres();

		emit sigGenresFetched();
	}
}

Util::Set<Genre> GenreFetcher::genres() const
{
	auto genres = Util::Set<Genre> {m->genres};
	for(const auto& genre: m->additionalGenres)
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

void GenreFetcher::applyGenreToMetadata(const MetaDataList& tracks, const Genre& genre)
{
	if(!m->taggingInProgress)
	{
		auto* userTaggingOperation = initTagging();
		userTaggingOperation->applyGenreToMetadata(tracks, genre);
	}
}

void GenreFetcher::deleteGenres(const Util::Set<Genre>& genres)
{
	if(!m->taggingInProgress)
	{
		auto* userTaggingOperation = initTagging();
		userTaggingOperation->deleteGenres(genres);
	}
}

void GenreFetcher::renameGenre(const Genre& oldGenre, const Genre& newGenre)
{
	if(!m->taggingInProgress)
	{
		auto* userTaggingOperation = initTagging();
		userTaggingOperation->renameGenre(oldGenre, newGenre);
	}
}

void GenreFetcher::setLocalLibrary(LocalLibrary* localLibrary)
{
	m->localLibrary = localLibrary;
	connect(m->localLibrary, &LocalLibrary::sigReloadingLibraryFinished, this, &GenreFetcher::reloadGenres);

	QTimer::singleShot(200, this, SLOT(reloadGenres())); // NOLINT(*-magic-numbers)
}
