/* Session.cpp */

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

#include "Session.h"

#include "Components/PlayManager/PlayManager.h"
#include "Database/Connector.h"
#include "Database/LibraryDatabase.h"
#include "Database/Session.h"
#include "Utils/Algorithm.h"
#include "Utils/Logger/Logger.h"
#include "Utils/MetaData/MetaData.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Set.h"
#include "Utils/Utils.h"

#include <limits>

using Session::Manager;

struct Manager::Private
{
	PlayManager* playManager;
	Session::Id sessionId;
	QList<Session::Id> sessionIds;
	QList<Session::Timecode> sessionDays;
	bool playtimeResetted;

	Private(PlayManager* playManager) :
		playManager(playManager),
		playtimeResetted(true)
	{
		auto* db = DB::Connector::instance();
		DB::Session* sessionConnector = db->sessionConnector();

		sessionId = sessionConnector->createNewSession();

		sessionIds = sessionConnector->getSessionKeys();
		sessionIds.prepend(sessionId);

		Util::Set<Session::Timecode> days;
		for(Session::Timecode timecode : sessionIds)
		{
			Session::Timecode timecode_begin = Session::dayBegin(timecode);
			days.insert(timecode_begin);
		}

		sessionDays = days.toList();

		Util::Algorithm::sort(sessionIds, [](auto key1, auto key2){
			return (key1 > key2);
		});

		Util::Algorithm::sort(sessionDays, [](auto key1, auto key2){
			return (key1 > key2);
		});
	}
};

Manager::Manager(PlayManager* playManager)
{
	m = Pimpl::make<Private>(playManager);
	connect(m->playManager, &PlayManager::sigPositionChangedMs, this, &Manager::positionChanged);
}

Manager::~Manager() = default;

void Manager::positionChanged(MilliSeconds ms)
{
	Q_UNUSED(ms)
	static const MilliSeconds MinTime=5000;

	MilliSeconds playtime = m->playManager->currentTrackPlaytimeMs();

	if(playtime > MinTime && m->playtimeResetted)
	{
		spLog(Log::Debug, this) << "Adding track to Session " << m->sessionId;
		auto* db = DB::Connector::instance();
		DB::Session* sessionConnector = db->sessionConnector();

		sessionConnector->addTrack(m->sessionId, m->playManager->currentTrack());

		emit sigSessionChanged(m->sessionId);
	}

	m->playtimeResetted = (playtime <= MinTime);
}

Session::EntryListMap Manager::history(const QDateTime& dtBegin, const QDateTime& dtEnd)
{
	auto* db = DB::Connector::instance();
	DB::Session* sessionConnector = db->sessionConnector();

	return sessionConnector->getSessions(dtBegin, dtEnd);
}

Session::EntryListMap Manager::historyForDay(const QDateTime& dt)
{
	QDateTime dt_min(dt.toUTC());
	dt_min.setTime(QTime(0, 0));

	QDateTime dt_max(dt.toUTC());
	dt_max.setTime(QTime(23, 59, 59));

	return history(dt_min, dt_max);
}

Session::EntryListMap Manager::historyEntries(int dayIndex, int count)
{
	if(dayIndex > m->sessionDays.count() - 1)
	{
		return Session::EntryListMap();
	}

	int minIndex = std::min(dayIndex * count, m->sessionDays.count() - 1);
	int maxIndex = std::min((dayIndex + 1) * count - 1, m->sessionDays.count() - 1);

	Session::Timecode minKey = dayBegin(m->sessionDays[maxIndex]);
	Session::Timecode maxKey = dayEnd(m->sessionDays[minIndex]);

	auto* db = DB::Connector::instance();
	DB::Session* sessionConnector = db->sessionConnector();

	EntryListMap history = sessionConnector->getSessions(Util::intToDate(minKey), Util::intToDate(maxKey));
	if(history.isEmpty() && dayIndex == 0){
		history[Session::dayBegin(Session::now())] = EntryList();
	}

	return history;
}

bool Manager::isEmpty() const
{
	auto* db = DB::Connector::instance();
	DB::Session* sessionConnector = db->sessionConnector();

	return (sessionConnector->getSessionKeys().isEmpty());
}
