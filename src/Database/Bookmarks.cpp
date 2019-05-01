/* DatabaseBookmarks.cpp */

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

#include "Database/Bookmarks.h"
#include "Database/Query.h"
#include "Utils/Utils.h"

using DB::Bookmarks;
using DB::Query;

Bookmarks::Bookmarks(const QString& connection_name, DbId db_id) :
	DB::Module(connection_name, db_id) {}

Bookmarks::~Bookmarks() {}

bool Bookmarks::searchBookmarks(int track_id, QMap<Seconds, QString>& bookmarks)
{
	bookmarks.clear();

	DB::Query q = this->run_query
	(
		"SELECT name, timeidx FROM savedbookmarks WHERE trackid=:trackid;",
		{
			{":trackid", track_id}
		},
		QString("Cannot find bookmark for Track %1").arg(track_id)
	);

	if (q.has_error()){
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


bool Bookmarks::insertBookmark(int track_id, Seconds time, const QString& name)
{
	DB::Query q = insert("savedbookmarks",
	{
		{"trackid",	track_id},
		{"name",	Util::cvt_not_null(name)},
		{"timeidx",	time}
	}, "Cannot insert bookmarks");

	return (!q.has_error());
}


bool Bookmarks::removeBookmark(int track_id, Seconds time)
{
	DB::Query q = run_query
	(
		"DELETE FROM savedbookmarks WHERE trackid=:trackid AND timeidx=:timeidx;",
		{
			{":trackid", track_id},
			{":timeidx", time}
		},
		"Cannot remove bookmark"
	);

	return (!q.has_error());
}


bool Bookmarks::removeAllBookmarks(int track_id)
{
	DB::Query q = run_query
	(
		"DELETE FROM savedbookmarks WHERE trackid=:trackid;",
		{
			{":trackid", track_id}
		},
		"Cannot remove all bookmarks"
	);

	return (!q.has_error());
}

