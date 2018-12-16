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
		{"trackId", md.id}
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
