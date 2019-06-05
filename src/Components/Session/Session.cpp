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

#include "Components/PlayManager/PlayManager.h"

#include "Database/Connector.h"
#include "Database/Session.h"
#include "Database/LibraryDatabase.h"

#include "Utils/MetaData/MetaData.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Utils.h"
#include "Utils/Logger/Logger.h"


struct Session::Private
{
	QString session_id;

	Private()
	{
		session_id = Util::random_string(32);
	}
};

Session::Session(QObject* parent) :
	QObject(parent)
{
	m = Pimpl::make<Private>();

	PlayManager* pm = PlayManager::instance();

	connect(pm, &PlayManager::sig_track_changed, this, &Session::track_changed);
}

Session::~Session() {}

void Session::track_changed(const MetaData& md)
{
	uint64_t cur_date = Util::current_date_to_int();

	DB::Connector* db = DB::Connector::instance();
	DB::Session* session_connector = db->session_connector();

	session_connector->add_track(m->session_id, cur_date, md);
}

QMap<QDateTime, MetaDataList> Session::get_history(QDateTime beginning)
{
	DB::Connector* db = DB::Connector::instance();
	DB::Session* session_connector = db->session_connector();
	DB::LibraryDatabase* track_connector = db->library_db(-1, db->db_id());

	QMap<QDateTime, MetaDataList> ret;

	uint64_t start = Util::date_to_int(beginning);
	if(!beginning.isValid()){
		start = 0;
	}

	PairList<uint64_t, TrackID> history = session_connector->get_sessions(start);

	QList<TrackID> track_ids;
	for(auto it=history.begin(); it != history.end(); it++)
	{
		track_ids << it->second;
	}

	MetaDataList v_md;
    track_connector->getTracksByIds(track_ids, v_md);

	for(auto it=v_md.begin(); it != v_md.end(); it++)
	{
		sp_log(Log::Debug, "Session") << "History Filepath: " << it->filepath();
	}


	return ret;
}
