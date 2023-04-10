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

#include <QDateTime>

#include <limits>

using Session::Manager;

namespace
{
	DB::Session* sessionConnector()
	{
		return DB::Connector::instance()->sessionConnector();
	}
}

struct Manager::Private
{
	PlayManager* playManager;
	Session::Id sessionId;
	QList<Session::Id> sessionIds;
	QList<Session::Timecode> sessionDays;
	bool playtimeResetted {true};

	explicit Private(PlayManager* playManager) :
		playManager(playManager),
		sessionId {sessionConnector()->createNewSession()},
		sessionIds {sessionConnector()->getSessionKeys()}
	{
		qRegisterMetaType<Session::Id>("SessionId");

		sessionIds.prepend(sessionId);

		Util::Set<Session::Timecode> days;
		for(const auto timecode: sessionIds)
		{
			const auto timecodeBegin = Session::dayBegin(timecode);
			days.insert(timecodeBegin);
		}

		Util::Algorithm::sort(sessionIds, [](const auto& key1, const auto& key2) {
			return (key1 > key2);
		});

		sessionDays = days.toList();
		Util::Algorithm::sort(sessionDays, [](const auto& key1, const auto& key2) {
			return (key1 > key2);
		});
	}
};

Manager::Manager(PlayManager* playManager) :
	m {Pimpl::make<Private>(playManager)}
{
	connect(m->playManager, &PlayManager::sigPositionChangedMs, this, &Manager::positionChanged);
}

Manager::~Manager() = default;

void Manager::positionChanged(const MilliSeconds ms)
{
	constexpr const MilliSeconds MinTime = 5000;

	if(ms > MinTime && m->playtimeResetted)
	{
		spLog(Log::Debug, this) << "Adding track to Session " << m->sessionId;

		m->sessionConnector->addTrack(m->sessionId, m->playManager->currentTrack(), QDateTime::currentDateTime());

		emit sigSessionChanged(m->sessionId);
	}

	m->playtimeResetted = (ms <= MinTime);
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
Session::EntryListMap Manager::history(const QDateTime& dtBegin, const QDateTime& dtEnd)
{
	return sessionConnector()->getSessions(dtBegin, dtEnd);
}

Session::EntryListMap Manager::historyForDay(const QDateTime& dt)
{
	auto dateTimeMin(dt.toUTC());
	dateTimeMin.setTime(QTime(0, 0));

	auto dateTimeMax(dt.toUTC());
	dateTimeMax.setTime(QTime(23, 59, 59)); // NOLINT(readability-magic-numbers)

	return history(dateTimeMin, dateTimeMax);
}

Session::EntryListMap Manager::historyEntries(int dayIndex, int count)
{
	if(dayIndex > m->sessionDays.count() - 1)
	{
		return {};
	}

	const auto minIndex = std::min(dayIndex * count, m->sessionDays.count() - 1);
	const auto maxIndex = std::min((dayIndex + 1) * count - 1, m->sessionDays.count() - 1);

	const auto minKey = dayBegin(m->sessionDays[maxIndex]);
	const auto maxKey = dayEnd(m->sessionDays[minIndex]);

	auto history = sessionConnector()->getSessions(Util::intToDate(minKey), Util::intToDate(maxKey));
	if(history.isEmpty() && dayIndex == 0)
	{
		history[Session::dayBegin(Session::now())] = EntryList();
	}

	return history;
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
bool Manager::isEmpty() const
{
	return (m->sessionConnector->getSessionKeys().isEmpty());
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
void Session::Manager::clearAllHistory()
{
	m->sessionConnector->clear();

	for(const auto& sessionId: m->sessionIds)
	{
		emit sigSessionDeleted(sessionId);
	}

	m->sessionIds = QList<Session::Id> {m->sessionId};
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
void Session::Manager::clearAllHistoryBefore(const QDateTime& dt)
{
	const auto oldSessionIds = std::move(m->sessionIds);

	m->sessionConnector->clearBefore(dt);
	m->sessionIds = m->sessionConnector->getSessionKeys();
	m->sessionIds.prepend(m->sessionId);

	for(const auto& oldSessionId: oldSessionIds)
	{
		if(!m->sessionIds.contains(oldSessionId))
		{
			emit sigSessionDeleted(oldSessionId);
		}
	}
}
