/* CoverConnector.cpp */

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



#include "CoverConnector.h"
#include "Query.h"
#include "Utils/Utils.h"

using DB::Query;

DB::Covers::Covers(const QString& connection_name, DbId db_id) :
	DB::Module(connection_name, db_id)
{}

DB::Covers::~Covers() {}

bool DB::Covers::exists(const QString& hash)
{
	Query q_check(this);
	QString query_check("SELECT hash FROM covers WHERE hash = :hash;");
	q_check.prepare(query_check);
	q_check.bindValue(":hash", hash);
	if(!q_check.exec())
	{
		q_check.show_error("Cannot check cover");
		return false;
	}

	return q_check.next();
}

bool DB::Covers::get_cover(const QString& hash, QPixmap& pm)
{
	Query q(this);
	QString query = "SELECT data FROM covers WHERE hash = :hash;";
	q.prepare(query);
	q.bindValue(":hash", hash);
	if(!q.exec()){
		q.show_error("Cannot fetch cover");
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

	QByteArray data = ::Util::cvt_pixmap_to_bytearray(pm);

	if(this->exists(hash))
	{
		Query q(this);
		QString query("UPDATE covers SET data=:data WHERE hash=:hash;");
		q.prepare(query);
		q.bindValue(":data", data);
		q.bindValue(":hash", hash);

		if(!q.exec()){
			q.show_error("Cannot update cover");
			return false;
		}

		return true;
	}

	else
	{
		Query q(this);
		QString query("INSERT INTO covers (hash, data) VALUES (:hash, :data)");
		q.prepare(query);
		q.bindValue(":data", data);
		q.bindValue(":hash", hash);

		if(!q.exec()){
			q.show_error("Cannot insert cover");
			return false;
		}

		return true;
	}
}

bool DB::Covers::get_all_covers(QMap<QString, QPixmap>& covers)
{
	covers.clear();

	Query q(this);
	QString query = "SELECT hash, data FROM covers;";
	q.prepare(query);
	if(!q.exec()){
		q.show_error("Cannot fetch all covers");
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
	Query q(this);
	QString query = "DELETE FROM covers;";
	q.prepare(query);
	if(!q.exec()){
		q.show_error("Cannot drop all covers");
	}
}
