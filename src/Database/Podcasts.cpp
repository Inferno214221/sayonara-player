/* DatabasePodcasts.cpp */

/* Copyright (C) 2011-2020  Lucio Carreras
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

#include "Utils/Streams/Podcast.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Utils.h"

using DB::Podcasts;
using DB::Query;

Podcasts::Podcasts(const QString& connection_name, DbId db_id) :
	DB::Module(connection_name, db_id) {}

Podcasts::~Podcasts() = default;

bool Podcasts::getAllPodcasts(QList<Podcast>& podcasts)
{
	podcasts.clear();

	Query q = run_query("SELECT name, url, reversed FROM savedpodcasts;", "Cannot fetch podcasts");

	if(q.has_error()){
		return false;
	}

	while(q.next())
	{
		QString name = q.value(0).toString();
		QString url = q.value(1).toString();
		bool reversed = q.value(2).toBool();

		podcasts << Podcast(name, url, reversed);
	}

	return true;
}


bool Podcasts::deletePodcast(const QString& name)
{
	Query q = run_query
	(
		"DELETE FROM savedpodcasts WHERE name = :name;",
		{
			{":name", Util::cvt_not_null(name)},
		},
		QString("Could not delete podcast %1").arg(name)
	);

	return (!q.has_error());
}


bool Podcasts::addPodcast(const Podcast& podcast)
{
	Query q = insert("savedpodcasts",
	{
		{"name", Util::cvt_not_null(podcast.name())},
		{"url", Util::cvt_not_null(podcast.url())},
		{"reversed", podcast.reversed()}
	}, QString("Could not add podcast: %1, %2").arg(podcast.name(), podcast.url()));

	return (!q.has_error());
}


bool Podcasts::updatePodcastUrl(const QString& name, const QString& url)
{
	Query q = update("savedpodcasts",
		{{"url", Util::cvt_not_null(url)}},
		{"name", Util::cvt_not_null(name)},
		QString("Could not update podcast url %1").arg(name)
	);

	return (!q.has_error());
}

bool Podcasts::renamePodcast(const QString& old_name, const QString& new_name)
{
	Query q = update("savedpodcasts",
		{{"name", Util::cvt_not_null(new_name)}},
		{"name", Util::cvt_not_null(old_name)},
		QString("Could not update podcast name %1").arg(old_name)
	);

	q.show_query();
	sp_log(Log::Debug, this) << "Affected rows = " << q.numRowsAffected();
	return (!q.has_error());
}

Podcast Podcasts::getPodcast(const QString& name)
{
	Query q = run_query
	(
		"SELECT name, url, reversed FROM savedpodcasts WHERE name = :name;",
		{":name", name},
		QString("Cannot fetch podcast %1").arg(name)
	);

	if(!q.has_error() && q.next())
	{
		Podcast podcast;
		podcast.set_name(q.value(0).toString());
		podcast.set_url(q.value(1).toString());
		podcast.set_reversed(q.value(2).toBool());
		return podcast;
	}

	return Podcast();
}



