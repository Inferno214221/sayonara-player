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

#include "Common/SayonaraTest.h"
#include "Common/PlayManagerMock.h"
#include "Common/FileSystemMock.h"

#include "Components/Playlist/PlaylistLoader.h"
#include "Components/Playlist/PlaylistSaver.h"
#include "Components/Playlist/Playlist.h"
#include "Database/Connector.h"
#include "Database/Playlist.h"
#include "Database/Query.h"
#include "Utils/MetaData/MetaData.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Playlist/CustomPlaylist.h"

// access working directory with Test::Base::tempPath("somefile.txt");

namespace
{
	constexpr const auto Temporary = true;
	constexpr const auto Permanent = false;

	void cleanup()
	{
		const auto q1 = DB::Connector::instance()->runQuery("DELETE FROM playlistToTracks",
		                                                    "Cannot delete from playlistToTracks");
		const auto q2 = DB::Connector::instance()->runQuery("DELETE FROM playlists",
		                                                    "Cannot delete from playlists");

		QVERIFY(!DB::hasError(q1));
		QVERIFY(!DB::hasError(q2));

		auto* playlistConnector = DB::Connector::instance()->playlistConnector();
		auto allPlaylists = playlistConnector->getAllPlaylists(Playlist::StoreType::TemporaryAndPermanent, false);

		QVERIFY(allPlaylists.isEmpty());
	}

	struct TestEnv
	{
		TestEnv()
		{
			cleanup();

			SetSetting(Set::PL_LoadTemporaryPlaylists, false);
			SetSetting(Set::PL_LoadSavedPlaylists, false);
			SetSetting(Set::PL_LoadRecentPlaylists, false);
			SetSetting(Set::PL_LastPlaylist, -1);
			SetSetting(Set::PL_LastTrack, -1);
			SetSetting(Set::PL_LoadLastTrack, false);
			SetSetting(Set::PL_RecentPlaylists, {});
		}

		~TestEnv()
		{
			cleanup();
		}

		DB::Playlist* db {DB::Connector::instance()->playlistConnector()};
	};

	int createPlaylist(
		const int index, const QString& name, const bool temporary, PlayManager* playManager)
	{
		constexpr const auto TracksInPlaylist = 3;

		auto playlist =
			std::make_shared<Playlist::Playlist>(index, name, playManager,
			                                     std::make_shared<Test::AllFilesAvailableFileSystem>());
		auto tracks = MetaDataList {};
		for(int i = 0; i < TracksInPlaylist; i++)
		{
			const auto filepath = QString("/artist/album/title%1.mp3").arg(i);
			tracks << MetaData {filepath};
		}

		playlist->createPlaylist(tracks);
		playlist->setTemporary(temporary);
		playlist->save();

		return playlist->id();
	}
}

class PlaylistLoaderTest :
	public Test::Base
{
	Q_OBJECT

	public:
		PlaylistLoaderTest() :
			Test::Base("PlaylistLoaderTest") {}

	private:
		std::shared_ptr<PlayManager> m_playManager {std::make_shared<PlayManagerMock>()};

	private slots: // NOLINT(*-redundant-access-specifiers)

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void testLoadTemporaryAndSavedPlaylists()
		{
			struct TestCase
			{
				bool loadTemporaryPlaylists;
				bool loadPermanentPlaylists;
				QList<bool> isTemporary;
				int expectedCount;
			};

			const auto testCases = std::array {
				TestCase {true, true, {}, 0},
				TestCase {false, true, {}, 0},

				TestCase {true, true, {Temporary, Temporary}, 2},
				TestCase {true, false, {Temporary, Temporary}, 2},
				TestCase {false, true, {Temporary, Temporary}, 0},
				TestCase {false, false, {Temporary, Temporary}, 0},

				TestCase {true, true, {Permanent, Permanent}, 2},
				TestCase {true, false, {Permanent, Permanent}, 0},
				TestCase {false, true, {Permanent, Permanent}, 2},
				TestCase {false, false, {Permanent, Permanent}, 0},
			};

			for(const auto& testCase: testCases)
			{
				[[maybe_unused]] auto env = TestEnv {};

				SetSetting(Set::PL_LoadTemporaryPlaylists, testCase.loadTemporaryPlaylists);
				SetSetting(Set::PL_LoadSavedPlaylists, testCase.loadPermanentPlaylists);

				auto i = 1;
				for(const auto& isTemporary: testCase.isTemporary)
				{
					const auto name = isTemporary ? QString("T%1").arg(i) : QString("P%1").arg(i);
					createPlaylist(i, name, isTemporary, m_playManager.get());

					i++;
				}

				auto loader = Playlist::LoaderImpl();
				const auto& fetchedPlaylists = loader.playlists();

				QCOMPARE(fetchedPlaylists.count(), testCase.expectedCount);
			}
		}

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void testLoadRecentPlaylists()
		{
			struct TestCase
			{
				bool loadRecentPlaylists;
				QList<bool> isTemporary;
				int expectedCount;
			};

			const auto testCases = std::array {
				TestCase {true, {}, 0},
				TestCase {false, {}, 0},

				TestCase {true, {Temporary, Temporary}, 2},
				TestCase {true, {Temporary, Permanent}, 2},
				TestCase {true, {Permanent, Permanent}, 2},

				TestCase {false, {Temporary, Temporary}, 0},
				TestCase {false, {Temporary, Permanent}, 0},
				TestCase {false, {Permanent, Permanent}, 0},
			};

			for(const auto& testCase: testCases)
			{
				[[maybe_unused]] auto env = TestEnv {};

				SetSetting(Set::PL_LoadRecentPlaylists, testCase.loadRecentPlaylists);

				auto session = QList<int> {};
				auto i = 1;
				for(const auto& isTemporary: testCase.isTemporary)
				{
					const auto name = isTemporary ? QString("T%1").arg(i) : QString("P%1").arg(i);
					const auto id = createPlaylist(i, name, isTemporary, m_playManager.get());
					session << id;

					i++;
				}

				SetSetting(Set::PL_RecentPlaylists, session);

				auto loader = Playlist::LoaderImpl();
				const auto& fetchedPlaylists = loader.playlists();

				QCOMPARE(fetchedPlaylists.count(), testCase.expectedCount);
			}
		}

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void testEvaluateSessionParameter()
		{
			struct TestCase
			{
				QList<bool> inSession;
				int expectedCount;
			};

			const auto testCases = std::array {
				TestCase {{}, 0},
				TestCase {{}, 0},

				TestCase {{true, true, true}, 3},
				TestCase {{true, true, false}, 2},
				TestCase {{true, false, true}, 2},
				TestCase {{false, true, false}, 1},
				TestCase {{false, false, false}, 0},
			};

			for(const auto& testCase: testCases)
			{
				[[maybe_unused]] auto env = TestEnv {};
				SetSetting(Set::PL_LoadRecentPlaylists, true);

				auto session = QList<int> {};
				auto i = 1;
				for(const auto& inSession: testCase.inSession)
				{
					const auto name = QString("P%1").arg(i);
					const auto id = createPlaylist(i, name, Temporary, m_playManager.get());
					if(inSession)
					{
						session << id;
					}

					i++;
				}

				SetSetting(Set::PL_RecentPlaylists, session);

				auto loader = Playlist::LoaderImpl();
				const auto& fetchedPlaylists = loader.playlists();

				QCOMPARE(fetchedPlaylists.count(), testCase.expectedCount);
				for(auto sessionIndex = 0; sessionIndex < session.count(); sessionIndex++)
				{
					QCOMPARE(fetchedPlaylists[sessionIndex].id(), session[sessionIndex]);
				}
			}
		}

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void testInvalidIndexesAreNotFetched()
		{
			[[maybe_unused]] auto env = TestEnv {};

			const auto recentPlaylist = QList<int> {-1, 5, 6, 8};
			SetSetting(Set::PL_LoadRecentPlaylists, true);
			SetSetting(Set::PL_RecentPlaylists, recentPlaylist);

			auto loader = Playlist::LoaderImpl();
			const auto& fetchedPlaylists = loader.playlists();
			QVERIFY(fetchedPlaylists.isEmpty());
		}

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void testPlaylistSaverOnlySavesCorrectPlaylists()
		{
			const auto fileSystem = std::make_shared<Test::AllFilesAvailableFileSystem>();

			struct TestCase
			{
				bool loadRecent;
				bool loadTemporary;
				bool loadPermanent;

				QList<int> expectedSession;
			};

			const auto testCases = std::array {
				TestCase {false, false, false, {}},
				TestCase {true, false, false, {0, 1}},
				TestCase {false, true, false, {0}},
				TestCase {false, false, true, {1}},
				TestCase {false, true, true, {0, 1}}
			};

			for(const auto& testCase: testCases)
			{
				[[maybe_unused]] auto env = TestEnv {};

				SetSetting(Set::PL_LoadRecentPlaylists, testCase.loadRecent);
				SetSetting(Set::PL_LoadTemporaryPlaylists, testCase.loadTemporary);
				SetSetting(Set::PL_LoadSavedPlaylists, testCase.loadPermanent);

				auto temporary = std::make_shared<Playlist::Playlist>(0, "new1", m_playManager.get(), fileSystem);
				temporary->setTemporary(true);
				temporary->setId(0);

				auto permanent = std::make_shared<Playlist::Playlist>(1, "new2", m_playManager.get(), fileSystem);
				permanent->setTemporary(false);
				permanent->setId(1);

				Playlist::saveCurrentPlaylists({temporary, permanent});

				const auto session = GetSetting(Set::PL_RecentPlaylists);
				QCOMPARE(session, testCase.expectedSession);
			}
		}
};

QTEST_GUILESS_MAIN(PlaylistLoaderTest)

#include "PlaylistLoaderTest.moc"
