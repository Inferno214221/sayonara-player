/* Session.cpp */

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



#include "Session.h"
#include "Query.h"
#include "Utils/MetaData/MetaData.h"

using DB::Module;

DB::Session::Session(const QString& connection_name, DbId db_id) :
	Module(connection_name, db_id)
{
	QString create_query =
		"CREATE TABLE IF NOT EXISTS Session "
			"("
			"    id INTEGER DEFAULT 0 PRIMARY KEY, "
			"    sessionId VARCHAR(32), "
			"    date INTEGER, "
			"    artistName VARCHAR(128), "
			"    trackId INTEGER DEFAULT -1 REFERENCES Tracks(trackID) ON DELETE SET DEFAULT"
			");";

	this->run_query(create_query, "Cannot create table Sessions");
}

DB::Session::~Session() {}

PairList<uint64_t, TrackID> DB::Session::get_sessions(uint64_t beginning)
{
	PairList<uint64_t, TrackID> ret;
	QString query
	(
		"SELECT Session.date, Session.trackId FROM Session, Tracks "
		"WHERE Session.trackId = tracks.trackId AND Session.date > :beggining;"
	);

	QPair<QString, QVariant> binding(":beggining", QVariant::fromValue<uint64_t>(beginning));

	DB::Query q = this->run_query(query, binding, "Cannot fetch sessions");
	if(q.has_error())
	{
		return ret;
	}

	q.show_query();

	while(q.next())
	{
		uint64_t date =			q.value(0).value<uint64_t>();
		TrackID track_id =		q.value(1).toInt();

		ret.push_back(QPair<uint64_t, TrackID>(date, track_id));
	}

	return ret;
}

bool DB::Session::add_track(const QString& session_id, uint64_t current_date_time, const MetaData &md)
{
	QMap<QString, QVariant> bindings
	{
		{"sessionId", session_id},
		{"date", QVariant::fromValue<uint64_t>(current_date_time)},
		{"artistName", md.artist()},
		{"trackId", md.id()}
	};

	DB::Query q = insert("Session", bindings, "Cannot add track to session");

	return q.has_error();
}

void DB::Session::clear()
{
	DB::Query q(this);
	q.prepare("DELETE FROM Session;");
	q.exec();
}
