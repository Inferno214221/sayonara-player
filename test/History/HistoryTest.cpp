/*
 * Copyright (C) 2011-2024 Michael Lugmair
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

#include "Common/PlayManagerMock.h"
#include "Common/SayonaraTest.h"
#include "Common/TestTracks.h"

#include "Components/Session/Session.h"
#include "Database/Connector.h"
#include "Database/Session.h"
#include "Utils/MetaData/MetaData.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Utils.h"

#include <QSignalSpy>

#include <utility>

namespace
{
	class HistoryPlayManager :
		public PlayManagerMock
	{
		public:
			HistoryPlayManager() = default;

			void setCurrentPositionMs(const MilliSeconds ms) override
			{
				m_currentPlaytime = ms;
				emit sigPositionChangedMs(ms);
			}

			[[nodiscard]] MilliSeconds currentTrackPlaytimeMs() const override
			{
				return m_currentPlaytime;
			}

			[[nodiscard]] const MetaData& currentTrack() const override
			{
				return m_currentTrack;
			}

			void setCurrentTrack(const MetaData& track)
			{
				m_currentTrack = track;
			}

		private:
			MetaData m_currentTrack;
			MilliSeconds m_currentPlaytime {0};
	};

	int createHistory(DB::Session* db)
	{
		auto tracks = Test::createTracks();

		int count = 0;
		for(int i = 3; i <= 7; i++, count++)
		{
			const auto date = QDate(2020, i, 1);
			db->addTrack(Util::dateToInt(date.startOfDay()),
			             tracks[count],
			             date.startOfDay()); // NOLINT(readability-magic-numbers)

		}

		return count;
	}

	std::pair<QDateTime, QDateTime> minMaxDate()
	{
		return {QDate(1, 1, 1).startOfDay(), QDate(2100, 1, 1).startOfDay()}; // NOLINT(readability-magic-numbers)
	}

	QMap<Session::Id, QVector<Session::Entry>> getAllSessions(DB::Session* db)
	{
		const auto [start, end] = minMaxDate();
		return db->getSessions(start, end);
	}

	[[maybe_unused]] void
	emulatePlayback(HistoryPlayManager& playManager, const MetaData& track, const MilliSeconds playtime)
	{
		playManager.setCurrentTrack(track);

		for(MilliSeconds i = 0; i < playtime; i += 1'000) // NOLINT(readability-magic-numbers)
		{
			playManager.setCurrentPositionMs(i);
		}
	}

	void cleanup()
	{
		auto playManager = HistoryPlayManager();
		auto sessionManager = Session::Manager(&playManager);
		sessionManager.clearAllHistory();
	}
}

class HistoryTest :
	public Test::Base
{
	Q_OBJECT

	public:
		HistoryTest() :
			Test::Base("HistoryTest") {}

	private slots:

		[[maybe_unused]] void testCreateHistory()  // NOLINT(readability-convert-member-functions-to-static)
		{
			auto playManager = PlayManagerMock();
			auto sessionManager = Session::Manager(&playManager);

			auto* db = DB::Connector::instance()->sessionConnector();
			const auto createdEntries = createHistory(db);
			const auto [start, end] = minMaxDate();

			QVERIFY(sessionManager.history(start, end).count() == createdEntries);
			QVERIFY(db->getSessionKeys().count() == createdEntries);
			QVERIFY(db->getSessions(QDate(1, 1, 1).startOfDay(), QDate(2100, 1, 1).startOfDay()).count() == 5);

			cleanup();
		}

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static,readability-function-cognitive-complexity)
		[[maybe_unused]] void testAddTrack()
		{
			const auto tracks = QList<MetaData> {
				Test::createTrack(1, "t1", "al1", "ar1"),
				Test::createTrack(1, "t2", "al2", "ar2"),
				Test::createTrack(1, "t3", "al3", "ar3")
			};

			auto playManager = HistoryPlayManager();
			auto sessionManager = Session::Manager(&playManager);

			QVERIFY(sessionManager.historyForDay(QDate::currentDate().startOfDay()).isEmpty());

			auto currentTrackIndex = 0;
			for(const auto& track: tracks)
			{
				emulatePlayback(playManager, track, 10'000); // NOLINT(readability-magic-numbers)

				const auto historyForToday = sessionManager.historyForDay(QDateTime::currentDateTime());
				QVERIFY(historyForToday.count() == 1);

				const auto sessionKey = historyForToday.keys()[0];
				const auto entries = historyForToday[sessionKey];
				const auto& currentEntry = entries[currentTrackIndex];

				QVERIFY(entries.count() == currentTrackIndex + 1);
				QVERIFY(currentEntry.track.id() == track.id());
				QVERIFY(currentEntry.track.title() == track.title());
				QVERIFY(currentEntry.track.album() == track.album());
				QVERIFY(currentEntry.track.artist() == track.artist());

				++currentTrackIndex;
			}

			cleanup();
		}

		[[maybe_unused]] void testDeleteAllHistory() // NOLINT(readability-convert-member-functions-to-static)
		{
			auto* db = DB::Connector::instance()->sessionConnector();
			createHistory(db);

			auto playManager = PlayManagerMock();
			auto sessionManager = Session::Manager(&playManager);

			sessionManager.clearAllHistory();
			QVERIFY(db->getSessionKeys().count() == 0);
			QVERIFY(getAllSessions(db).isEmpty());

			const auto [start, end] = minMaxDate();
			QVERIFY(sessionManager.isEmpty());
			QVERIFY(sessionManager.history(start, end).isEmpty());
			QVERIFY(sessionManager.historyForDay(QDate(2020, 5, 1).startOfDay()).isEmpty());

			cleanup();
		};

		[[maybe_unused]] void testDeleteOldHistory() // NOLINT(readability-convert-member-functions-to-static)
		{
			auto* db = DB::Connector::instance()->sessionConnector();
			createHistory(db);

			struct TestCase
			{
				QDate date;
				int expectedCount;
			};

			const auto testCases = {
				TestCase {QDate(2019, 1, 1), 5},
				TestCase {QDate(2020, 3, 2), 4},
				TestCase {QDate(2020, 5, 15), 2},
				TestCase {QDate(2020, 7, 1), 0}
			};

			const auto [start, end] = minMaxDate();

			for(const auto& testCase: testCases)
			{
				auto playManager = PlayManagerMock();
				auto sessionManager = Session::Manager(&playManager);

				sessionManager.clearAllHistoryBefore(testCase.date.startOfDay());

				const auto sessionKeys = getAllSessions(db).keys();
				QVERIFY(sessionManager.history(start, end).count() == sessionKeys.count());
				QVERIFY(sessionKeys.count() == testCase.expectedCount);
			}

			cleanup();
		}

		[[maybe_unused]] void
		testSignalIsEmittedWhenChangingTrack() // NOLINT(readability-convert-member-functions-to-static)
		{
			const auto tracks = QList<MetaData> {
				Test::createTrack(1, "t1", "al1", "ar1"),
				Test::createTrack(1, "t2", "al2", "ar2"),
				Test::createTrack(1, "t3", "al3", "ar3")
			};

			auto playManager = HistoryPlayManager();
			auto sessionManager = Session::Manager(&playManager);

			for(const auto& track: tracks)
			{
				auto spy = QSignalSpy(&sessionManager, &Session::Manager::sigSessionChanged);
				emulatePlayback(playManager, track, 10'000); // NOLINT(readability-magic-numbers)

				auto currentSessionId = sessionManager.historyForDay(QDateTime::currentDateTime()).keys()[0];
				QVERIFY(spy.count() == 1);
				const auto arguments = spy.takeFirst();
				QVERIFY(arguments.at(0).value<uint64_t>() == currentSessionId);
			}

			cleanup();
		}

		[[maybe_unused]] void
		testSignalIsEmittedWhenHistoryIsCleared() // NOLINT(readability-convert-member-functions-to-static)
		{
			const auto sessions = createHistory(DB::Connector::instance()->sessionConnector());

			auto playManager = HistoryPlayManager();
			auto sessionManager = Session::Manager(&playManager);

			auto spy = QSignalSpy(&sessionManager, &Session::Manager::sigSessionDeleted);
			sessionManager.clearAllHistory();
			QVERIFY(spy.count() == sessions + 1);

			cleanup();
		}

		[[maybe_unused]] void
		testSignalsAreEmittedWhenHistoryIsPartlyCleared() // NOLINT(readability-convert-member-functions-to-static)
		{
			createHistory(DB::Connector::instance()->sessionConnector());

			const auto cutDate = QDate(2020, 5, 15);
			auto playManager = HistoryPlayManager();
			auto sessionManager = Session::Manager(&playManager);

			auto spy = QSignalSpy(&sessionManager, &Session::Manager::sigSessionDeleted);
			sessionManager.clearAllHistoryBefore(cutDate.startOfDay());

			QVERIFY(spy.count() == 3);
			for(int i = 0; i < spy.count(); i++)
			{
				const auto sessionId = spy.at(i).at(0).value<Session::Id>();
				const auto date = Util::intToDate(sessionId);

				QVERIFY(date.date() < cutDate);
			}

			cleanup();
		}
};

QTEST_GUILESS_MAIN(HistoryTest)

#include "HistoryTest.moc"
