/* DatabaseBookmarks.cpp */

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

#include "Database/Bookmarks.h"
#include "Database/Query.h"
#include "Utils/Utils.h"

using DB::Bookmarks;

Bookmarks::Bookmarks(const QString& connection_name, DbId databaseId) :
	DB::Module(connection_name, databaseId) {}

DB::Bookmarks::~Bookmarks() = default;

bool Bookmarks::searchBookmarks(int trackId, QMap<Seconds, QString>& bookmarks)
{
	bookmarks.clear();

	auto q = runQuery(
		"SELECT name, timeidx FROM savedbookmarks WHERE trackid=:trackid;",
		{
			{":trackid", trackId}
		},
		QString("Cannot find bookmark for Track %1").arg(trackId));

	if(hasError(q))
	{
		return false;
	}

	while(q.next())
	{
		QString name = q.value(0).toString();
		Seconds bm = q.value(1).toUInt();

		bookmarks.insert(bm, name);
	}

	return true;
}

bool Bookmarks::insertBookmark(int trackId, Seconds time, const QString& name)
{
	auto q = insert("savedbookmarks",
	                {
		                {"trackid", trackId},
		                {"name",    Util::convertNotNull(name)},
		                {"timeidx", time}
	                }, "Cannot insert bookmarks");

	return !hasError(q);
}

bool Bookmarks::removeBookmark(int trackId, Seconds time)
{
	auto q = runQuery(
		"DELETE FROM savedbookmarks WHERE trackid=:trackid AND timeidx=:timeidx;",
		{
			{":trackid", trackId},
			{":timeidx", time}
		},
		"Cannot remove bookmark");

	return !hasError(q);
}

bool Bookmarks::removeAllBookmarks(int trackId)
{
	const auto q = runQuery(
		"DELETE FROM savedbookmarks WHERE trackid=:trackid;",
		{
			{":trackid", trackId}
		},
		"Cannot remove all bookmarks");

	return !hasError(q);
}

