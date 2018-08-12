/* DatabaseBookmarks.cpp */

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

#include "Database/DatabaseBookmarks.h"
#include "Database/SayonaraQuery.h"
#include "Utils/Utils.h"

using DB::Bookmarks;
using DB::Query;

Bookmarks::Bookmarks(const QString& connection_name, DbId db_id) :
	DB::Module(connection_name, db_id) {}

Bookmarks::~Bookmarks() {}

bool Bookmarks::searchBookmarks(int track_id, QMap<Seconds, QString>& bookmarks)
{
	bookmarks.clear();

	Query q(this);
	q.prepare("SELECT name, timeidx FROM savedbookmarks WHERE trackid=:trackid;");
	q.bindValue(":trackid", track_id);

	if (!q.exec()){
		q.show_error( QString("Could not fetch bookmarks for track ") + QString::number(track_id));
		return false;
	}

	while(q.next()) {
		QString name = q.value(0).toString();
		Seconds bm = q.value(1).toUInt();

		bookmarks.insert(bm, name);
	}

	return true;
}


bool Bookmarks::insertBookmark(int track_id, Seconds time, const QString& name)
{
	Query q(this);
	q.prepare("INSERT INTO savedbookmarks (trackid, name, timeidx) VALUES(:trackid, :name, :timeidx);");
	q.bindValue(":trackid",	track_id);
	q.bindValue(":name",	Util::cvt_not_null(name));
	q.bindValue(":timeidx",	time);

	if (!q.exec()){
		q.show_error("Cannot insert bookmarks");
		return false;
	}

	return true;
}


bool Bookmarks::removeBookmark(int track_id, Seconds time)
{
	Query q(this);
	q.prepare("DELETE FROM savedbookmarks WHERE trackid=:trackid AND timeidx=:timeidx;");
	q.bindValue(":trackid", track_id);
	q.bindValue(":timeidx", time);

	if (!q.exec()){
		q.show_error("Cannot remove bookmark");
		return false;
	}

	return true;
}


bool Bookmarks::removeAllBookmarks(int track_id)
{
	Query q(this);
	q.prepare("DELETE FROM savedbookmarks WHERE trackid=:trackid;");
	q.bindValue(":trackid", track_id);

	if (!q.exec()){
		q.show_error("Cannot remove all bookmarks");
		return false;
	}

	return true;
}

