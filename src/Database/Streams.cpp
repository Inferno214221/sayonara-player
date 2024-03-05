/* DatabaseStreams.cpp */

/* Copyright (C) 2011-2024 Michael Lugmair (Lucio Carreras)
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
#include "Utils/Streams/Station.h"

using DB::Streams;

Streams::Streams(const QString& connectionName, DbId databaseId) :
	Module(connectionName, databaseId) {}

Streams::~Streams() = default;

bool Streams::getAllStreams(QList<Stream>& streams)
{
	streams.clear();

	auto q = runQuery("SELECT name, url, isUpdatable, userAgent FROM savedstreams;", "Cannot fetch streams");
	if(hasError(q))
	{
		return false;
	}

	while(q.next())
	{
		streams << Stream {
			q.value(0).toString(),
			q.value(1).toString(),
			q.value(2).toBool(),
			q.value(3).toString()
		};
	}

	return true;
}

bool Streams::deleteStream(const QString& name)
{
	auto q = runQuery(
		"DELETE FROM savedstreams WHERE name = :name;",
		{
			{":name", Util::convertNotNull(name)},
		},
		QString("Could not delete stream %1").arg(name));

	return !hasError(q);
}

bool Streams::addStream(const Stream& stream)
{
	const auto q = insert("savedstreams",
	                      {
		                      {"name",        Util::convertNotNull(stream.name())},
		                      {"url",         Util::convertNotNull(stream.url())},
		                      {"isUpdatable", stream.isUpdatable()},
		                      {"userAgent",   Util::convertNotNull(stream.userAgent())}
	                      }, QString("Could not add stream: %1, %2").arg(stream.name(), stream.url()));

	return !hasError(q);
}

bool DB::Streams::updateStream(const QString& old_name, const Stream& stream)
{
	const auto q = update("savedstreams",
	                      {
		                      {"name",        Util::convertNotNull(stream.name())},
		                      {"url",         Util::convertNotNull(stream.url())},
		                      {"isUpdatable", stream.isUpdatable()},
		                      {"userAgent",   Util::convertNotNull(stream.userAgent())}
	                      },
	                      {"name", Util::convertNotNull(old_name)},
	                      QString("Could not update stream name %1").arg(old_name));

	return wasUpdateSuccessful(q);
}

Stream Streams::getStream(const QString& name)
{
	auto q = runQuery(
		"SELECT name, url, isUpdatable, userAgent FROM savedstreams WHERE name = :name;",
		{":name", name},
		QString("Cannot fetch stream %1").arg(name));

	if(!hasError(q) && q.next())
	{
		return {
			q.value(0).toString(),
			q.value(1).toString(),
			q.value(2).toBool(),
			q.value(3).toString()
		};
	}

	return {};
}
