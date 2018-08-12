/* DatabaseStreams.cpp */

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

#include "Database/SayonaraQuery.h"
#include "Database/DatabaseStreams.h"
#include "Utils/Utils.h"

using DB::Streams;
using DB::Query;

Streams::Streams(const QString& connection_name, DbId db_id) :
	Module(connection_name, db_id) {}

Streams::~Streams() {}

bool Streams::getAllStreams(QMap<QString, QString>& streams)
{
	streams.clear();

	Query q(this);
	q.prepare("SELECT name, url FROM savedstreams;");

	if (!q.exec()){
		q.show_error("Cannot get all streams");
		return false;
	}

	while(q.next()) {
		QString name = q.value(0).toString();
		QString url = q.value(1).toString().trimmed();

		streams[name] = url;
	}

	return true;
}


bool Streams::deleteStream(const QString& name)
{
	Query q(this);
	q.prepare("DELETE FROM savedstreams WHERE name = :name;" );
	q.bindValue(":name", Util::cvt_not_null(name));

	if(!q.exec()) {
		q.show_error(QString("Could not delete stream ") + name);
		return false;
	}

	return true;
}


bool Streams::addStream(const QString& name, const QString& url)
{
	Query q(this);

	q.prepare("INSERT INTO savedstreams (name, url) VALUES (:name, :url); " );
	q.bindValue(":name",	Util::cvt_not_null(name));
	q.bindValue(":url",		Util::cvt_not_null(url));

	if(!q.exec()) {
		q.show_error(QString("Could not add stream ") + name);
		return false;
	}

	return true;
}


bool Streams::updateStreamUrl(const QString& name, const QString& url)
{
	Query q(this);

	q.prepare("UPDATE savedstreams SET url=:url WHERE name=:name;");
	q.bindValue(":name",	Util::cvt_not_null(name));
	q.bindValue(":url",		Util::cvt_not_null(url));

	if(!q.exec()) {
		q.show_error(QString("Could not update stream url ") + name);
		return false;
	}

	return true;
}


