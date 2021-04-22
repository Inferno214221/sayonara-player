/* DatabaseStreams.cpp */

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

#include "Database/Streams.h"
#include "Database/Query.h"

#include "Utils/Utils.h"
#include "Utils/Streams/Station.h"

using DB::Streams;
using DB::Query;

Streams::Streams(const QString& connection_name, DbId databaseId) :
	Module(connection_name, databaseId) {}

Streams::~Streams() = default;

bool Streams::getAllStreams(QList<Stream>& streams)
{
	streams.clear();

	Query q = runQuery("SELECT name, url FROM savedstreams;", "Cannot fetch streams");

	if(q.hasError()){
		return false;
	}

	while(q.next())
	{
		QString name = q.value(0).toString();
		QString url = q.value(1).toString();

		streams << Stream(name, url);
	}

	return true;
}

bool Streams::deleteStream(const QString& name)
{
	Query q = runQuery
	(
		"DELETE FROM savedstreams WHERE name = :name;",
		{
			{":name", Util::convertNotNull(name)},
		},
		QString("Could not delete stream %1").arg(name)
	);

	return (!q.hasError());
}

bool Streams::addStream(const Stream& stream)
{
	Query q = insert("savedstreams",
	{
		{"name", Util::convertNotNull(stream.name())},
		{"url", Util::convertNotNull(stream.url())}
	}, QString("Could not add stream: %1, %2").arg(stream.name(), stream.url()));

	return (!q.hasError());
}

bool DB::Streams::updateStream(const QString& old_name, const Stream& stream)
{
	Query q = update("savedstreams",
		{
			{"name", Util::convertNotNull(stream.name())},
			{"url", Util::convertNotNull(stream.url())}
		},
		{"name", Util::convertNotNull(old_name)},
		QString("Could not update stream name %1").arg(old_name)
	);

	return (!q.hasError());
}

Stream Streams::getStream(const QString& name)
{
	QString query = "SELECT name, url FROM savedstreams WHERE name = :name;";
	Query q = runQuery
	(
		query,
		{":name", name},
		QString("Cannot fetch stream %1").arg(name)
	);

	if(!q.hasError() && q.next())
	{
		Stream stream;
		stream.setName(q.value(0).toString());
		stream.setUrl(q.value(1).toString());
		return stream;
	}

	return Stream();
}
