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
#include "Utils/Set.h"
#include "Utils/Algorithm.h"
#include "Utils/Logger/Logger.h"

#include <limits>

using Session::Manager;

struct Manager::Private
{
	Session::Id session_id;
	QList<Session::Id> session_ids;
	QList<Session::Timecode> session_days;
	bool playtime_resetted;

	Private() :
		playtime_resetted(true)
	{
		auto* db = DB::Connector::instance();
		DB::Session* session_connector = db->session_connector();

		session_id = session_connector->create_new_session();

		session_ids = session_connector->get_session_keys();
		session_ids.prepend(session_id);

		Util::Set<Session::Timecode> days;
		for(Session::Timecode timecode : session_ids)
		{
			Session::Timecode timecode_begin = Session::day_begin(timecode);
			days.insert(timecode_begin);
		}

		session_days = days.toList();

		Util::Algorithm::sort(session_ids, [](auto key1, auto key2){
			return (key1 > key2);
		});

		Util::Algorithm::sort(session_days, [](auto key1, auto key2){
			return (key1 > key2);
		});

	}
};

Manager::Manager()
{
	m = Pimpl::make<Private>();

	connect(PlayManager::instance(), &PlayManager::sig_position_changed_ms, this, &Manager::position_changed);
}

Manager::~Manager() = default;

void Manager::position_changed(MilliSeconds ms)
{
	Q_UNUSED(ms)
	const static MilliSeconds MinTime=5000;

	auto* pm = PlayManager::instance();
	MilliSeconds playtime = pm->current_track_playtime_ms();

	if(playtime > MinTime && m->playtime_resetted)
	{
		sp_log(Log::Debug, this) << "Adding track to Session " << m->session_id;
		auto* db = DB::Connector::instance();
		DB::Session* session_connector = db->session_connector();

		session_connector->add_track(m->session_id, pm->current_track());

		emit sig_changed(m->session_id);
	}

	m->playtime_resetted = (playtime <= MinTime);
}

Session::EntryListMap Manager::history(const QDateTime& dt_begin, const QDateTime& dt_end)
{
	auto* db = DB::Connector::instance();
	DB::Session* session_connector = db->session_connector();

	return session_connector->get_sessions(dt_begin, dt_end);
}

Session::EntryListMap Manager::history_for_day(const QDateTime& dt)
{
	QDateTime dt_min(dt.toUTC());
	dt_min.setTime(QTime(0, 0));

	QDateTime dt_max(dt.toUTC());
	dt_max.setTime(QTime(23, 59, 59));

	return history(dt_min, dt_max);
}


Session::EntryListMap Manager::history_entries(int day_index, int count)
{
	if(day_index >= m->session_days.count() - 1)
	{
		return Session::EntryListMap();
	}

	int max_index = std::min(day_index + count + 1, m->session_days.count() - 1);

	Session::Timecode min_key = m->session_days[max_index];
	Session::Timecode max_key = m->session_days[day_index];

	auto* db = DB::Connector::instance();
	DB::Session* session_connector = db->session_connector();

	return session_connector->get_sessions(Util::int_to_date(min_key), Util::int_to_date(max_key));
}
