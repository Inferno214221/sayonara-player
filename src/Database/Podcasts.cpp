/* DatabasePodcasts.cpp */

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

#include "Database/Query.h"
#include "Database/Podcasts.h"

#include "Utils/Streams/Station.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Utils.h"

using DB::Podcasts;
using DB::Query;

Podcasts::Podcasts(const QString& connection_name, DbId databaseId) :
	DB::Module(connection_name, databaseId) {}

Podcasts::~Podcasts() = default;

bool Podcasts::getAllPodcasts(QList<Podcast>& podcasts)
{
	podcasts.clear();

	auto q = runQuery("SELECT name, url, reversed FROM savedpodcasts;", "Cannot fetch podcasts");
	if(hasError(q))
	{
		return false;
	}

	while(q.next())
	{
		const auto name = q.value(0).toString();
		const auto url = q.value(1).toString();
		const auto reversed = q.value(2).toBool();

		podcasts << Podcast(name, url, reversed);
	}

	return true;
}

bool Podcasts::deletePodcast(const QString& name)
{
	auto q = runQuery(
		"DELETE FROM savedpodcasts WHERE name = :name;",
		{
			{":name", Util::convertNotNull(name)},
		},
		QString("Could not delete podcast %1").arg(name));

	return !hasError(q);
}

bool Podcasts::addPodcast(const Podcast& podcast)
{
	auto q = insert("savedpodcasts",
	                {
		                {"name",     Util::convertNotNull(podcast.name())},
		                {"url",      Util::convertNotNull(podcast.url())},
		                {"reversed", podcast.reversed()}
	                }, QString("Could not add podcast: %1, %2").arg(podcast.name(), podcast.url()));

	return !hasError(q);
}

bool Podcasts::updatePodcast(const QString& name, const Podcast& podcast)
{
	const auto q = update("savedpodcasts",
	                      {
		                      {"name",     Util::convertNotNull(podcast.name())},
		                      {"url",      Util::convertNotNull(podcast.url())},
		                      {"reversed", podcast.reversed()}
	                      },
	                      {"name", Util::convertNotNull(name)},
	                      QString("Could not update podcast url %1").arg(name));

	return wasUpdateSuccessful(q);
}

Podcast Podcasts::getPodcast(const QString& name)
{
	auto q = runQuery(
		"SELECT name, url, reversed FROM savedpodcasts WHERE name = :name;",
		{":name", name},
		QString("Cannot fetch podcast %1").arg(name));

	if(!hasError(q) && q.next())
	{
		Podcast podcast;
		podcast.setName(q.value(0).toString());
		podcast.setUrl(q.value(1).toString());
		podcast.setReversed(q.value(2).toBool());
		return podcast;
	}

	return {};
}



