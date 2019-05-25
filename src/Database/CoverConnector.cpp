/* CoverConnector.cpp */

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

#include "CoverConnector.h"
#include "Query.h"
#include "Utils/Utils.h"
#include "Utils/Set.h"

using DB::Query;

DB::Covers::Covers(const QString& connection_name, DbId db_id) :
	DB::Module(connection_name, db_id)
{}

DB::Covers::~Covers() {}

bool DB::Covers::exists(const QString& hash)
{
	Query q = run_query
	(
		"SELECT hash FROM covers WHERE hash = :hash;",
		{
			{":hash", hash}
		},
		"Cannot check cover"
	);

	if(q.has_error()){
		return false;
	}

	return q.next();
}

bool DB::Covers::get_cover(const QString& hash, QPixmap& pm)
{
	Query q = run_query
	(
		"SELECT data FROM covers WHERE hash = :hash;",
		{
			{":hash", hash}
		},
		"Cannot fetch cover"
	);

	if(q.has_error()){
		return false;
	}

	if(q.next())
	{
		QByteArray data = q.value(0).toByteArray();
		pm = ::Util::cvt_bytearray_to_pixmap(data);

		return true;
	}

	return false;
}

bool DB::Covers::set_cover(const QString& hash, const QPixmap& pm)
{
	if(hash.isEmpty() || pm.isNull()){
		return false;
	}

	if(this->exists(hash))
	{
		return update_cover(hash, pm);
	}

	else
	{
		return insert_cover(hash, pm);
	}
}

bool DB::Covers::update_cover(const QString& hash, const QPixmap& pm)
{
	QByteArray data = ::Util::cvt_pixmap_to_bytearray(pm);

	Query q = update("covers",
		{{"data", data}},
		{"hash", hash},
		"Cannot update cover"
	);

	return (!q.has_error());
}

bool DB::Covers::insert_cover(const QString& hash, const QPixmap& pm)
{
	QByteArray data = ::Util::cvt_pixmap_to_bytearray(pm);

	Query q = insert("covers",
	{
		{"data", data},
		{"hash", hash}
	}, "Cannot insert cover");

	return (!q.has_error());
}

Util::Set<QString> DB::Covers::get_all_hashes()
{
	Query q = run_query("SELECT hash FROM covers;", "Cannot fetch all hashes");
	if(q.has_error()){
		return Util::Set<QString>();
	}

	Util::Set<QString> ret;
	while(q.next())
	{
		ret.insert(q.value(0).toString());
	}

	return ret;
}

bool DB::Covers::get_all_covers(QMap<QString, QPixmap>& covers)
{
	covers.clear();

	Query q = run_query("SELECT hash, data FROM covers;", "Cannot fetch all covers");
	if(q.has_error()){
		return false;
	}

	while(q.next())
	{
		QString hash = q.value(0).toString();
		QByteArray data = q.value(1).toByteArray();

		covers[hash] = ::Util::cvt_bytearray_to_pixmap(data);
	}

	return true;
}

void DB::Covers::clear()
{
	run_query("DELETE FROM covers;", "Cannot drop all covers");
}
