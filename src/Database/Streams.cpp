/* DatabaseStreams.cpp */

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

#include "Database/Streams.h"
#include "Database/Query.h"

#include "Utils/Utils.h"

using DB::Streams;
using DB::Query;

Streams::Streams(const QString& connection_name, DbId db_id) :
	Module(connection_name, db_id) {}

Streams::~Streams() {}

bool Streams::getAllStreams(QMap<QString, QString>& streams)
{
	streams.clear();

	Query q = run_query("SELECT name, url FROM savedstreams;", "Cannot fetch streams");

	if(q.has_error()){
		return false;
	}

	while(q.next())
	{
		QString name = q.value(0).toString();
		QString url = q.value(1).toString();

		streams[name] = url;
	}

	return true;
}


bool Streams::deleteStream(const QString& name)
{
	Query q = run_query
	(
		"DELETE FROM savedstreams WHERE name = :name;",
		{
			{":name", Util::cvt_not_null(name)},
		},
		QString("Could not delete stream %1").arg(name)
	);

	return (!q.has_error());
}


bool Streams::addStream(const QString& name, const QString& url)
{
	Query q = insert("savedstreams",
	{
		{"name", Util::cvt_not_null(name)},
		{"url", Util::cvt_not_null(url)}
	}, QString("Could not add stream: %1, %2").arg(name, url));

	return (!q.has_error());
}


bool Streams::updateStreamUrl(const QString& name, const QString& url)
{
	Query q = update("savedstreams",
		{{"url", Util::cvt_not_null(url)}},
		{"name", Util::cvt_not_null(name)},
		QString("Could not update stream url %1").arg(name)
	);

	return (!q.has_error());
}


bool DB::Streams::renameStream(const QString& old_name, const QString& new_name)
{
	Query q = update("savedstreams",
		{{"name", Util::cvt_not_null(new_name)}},
		{"name", Util::cvt_not_null(old_name)},
		QString("Could not update stream name %1").arg(old_name)
	);

	return (!q.has_error());
}
