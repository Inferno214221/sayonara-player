/* PlaylistTest.cpp
 *
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

#include "PlaylistTestUtils.h"

#include "test/Common/SayonaraTest.h"
#include "test/Common/PlayManagerMock.h"
#include "test/Common/FileSystemMock.h"

#include "Components/Playlist/Playlist.h"
#include "Components/Playlist/PlaylistModifiers.h"
#include "Utils/Algorithm.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Set.h"
#include "Utils/Utils.h"
#include "Utils/Settings/Settings.h"

namespace
{
	template<typename FileSystem_t>
	struct TestEnv
	{
		static constexpr const auto PlaylistIndex = 1;
		static constexpr const auto PlaylistName = "empty";

		explicit TestEnv(const int count) :
			playlist {Playlist::Playlist {PlaylistIndex, PlaylistName, &playManager, fileSystem}}
		{
			auto tracks = MetaDataList {};
			for(int i = 0; i < count; i++)
			{
				auto track = MetaData {QString("abc%1.mp3").arg(i)};
				track.setId(i);
				tracks << track;
			}

			playlist.createPlaylist(tracks);
		}

		PlayManagerMock playManager;
		std::shared_ptr<Util::FileSystem> fileSystem {std::make_shared<FileSystem_t>()};
		Playlist::Playlist playlist;
	};
	
	IndexSet toSet(const QList<int>& lst)
	{
		auto result = IndexSet {};
		for(const auto item: lst)
		{
			result << item;
		}

		return result;
	}
}

class PlaylistTest :
	public Test::Base
{
	Q_OBJECT

	public:
		PlaylistTest() :
			Test::Base("PlaylistTest"),
			m_playManager {new PlayManagerMock()} {}

		~PlaylistTest() override
		{
			delete m_playManager;
		}

	private:
		PlayManager* m_playManager;
		Util::FileSystemPtr m_fileSystem = std::make_shared<Test::AllFilesAvailableFileSystem>();

	private slots:

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void testRunningTime()
		{
			struct TestCase
			{
				QList<MilliSeconds> runningTimes;
				MicroSeconds expectedTime;
			};

			const auto testCases = std::array {
				TestCase {{}, 0},
				TestCase {{0, 0, 0, 0}, 0},
				TestCase {{-1, -1}, 0},
				TestCase {{100, 200, 300, 400}, 1000},
			};

			for(const auto& testCase: testCases)
			{
				auto testEnv = TestEnv<Test::AllFilesAvailableFileSystem> {0};
				for(const auto t: testCase.runningTimes)
				{
					auto track = MetaData {"abc"};
					track.setDurationMs(t);
					appendTracks(testEnv.playlist, MetaDataList {track}, Playlist::Reason::Undefined);
				}

				QVERIFY(Playlist::runningTime(testEnv.playlist) == testCase.expectedTime);
			}
		}

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void testCurrentIndexAfterTrackChange()
		{
			constexpr const auto TrackCount = 10;
			struct TestCase
			{
				int trackCount;
				int targetIndex;
				bool expectedSuccess;
				int expectedCurrentIndex;
			};

			const auto testCases = std::array {
				TestCase {0, 0, false, -1},
				TestCase {TrackCount, -1, false, -1},
				TestCase {TrackCount, 100, false, -1},
				TestCase {TrackCount, 0, true, 0},
				TestCase {TrackCount, 4, true, 4},
			};

			for(const auto& testCase: testCases)
			{
				auto testEnv = TestEnv<Test::AllFilesAvailableFileSystem>(testCase.trackCount);
				auto& playlist = testEnv.playlist;

				QVERIFY(playlist.currentTrackIndex() == -1);

				const auto success = playlist.changeTrack(testCase.targetIndex);
				QVERIFY(success == testCase.expectedSuccess);
				QVERIFY(playlist.currentTrackIndex() == testCase.expectedCurrentIndex);
			}
		}

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void testFwdAndBwd()
		{
			constexpr const auto TrackCount = 10;
			enum Direction
			{
				Fwd,
				Bwd
			};
			struct TestCase
			{
				int trackCount;
				int currentIndex;
				Direction dir;
				bool repeatAll;
				int expectedCurrentIndex;
			};

			const auto testCases = std::array {
				TestCase {0, -1, Fwd, false, -1},
				TestCase {TrackCount, -1, Fwd, false, 0},
				TestCase {TrackCount, 4, Fwd, false, 5},
				TestCase {TrackCount, TrackCount - 1, Fwd, false, -1},
				TestCase {TrackCount, TrackCount - 1, Fwd, true, 0},
				TestCase {0, -1, Bwd, false, -1},
				TestCase {TrackCount, 0, Bwd, false, -1},
				TestCase {TrackCount, 1, Bwd, false, 0},
				TestCase {TrackCount, 4, Bwd, false, 3},
				TestCase {TrackCount, TrackCount - 1, Bwd, false, TrackCount - 2},
			};

			for(const auto& testCase: testCases)
			{
				auto testEnv = TestEnv<Test::AllFilesAvailableFileSystem>(testCase.trackCount);
				auto& playlist = testEnv.playlist;
				playlist.changeTrack(testCase.currentIndex);
				QVERIFY(playlist.currentTrackIndex() == testCase.currentIndex);

				auto playlistMode = GetSetting(Set::PL_Mode);
				playlistMode.setRepAll(testCase.repeatAll, testCase.repeatAll);
				SetSetting(Set::PL_Mode, playlistMode);

				if(testCase.dir == Fwd)
				{
					playlist.fwd();
				}
				else
				{
					playlist.bwd();
				}

				QVERIFY(playlist.currentTrackIndex() == testCase.expectedCurrentIndex);
			}
		}

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void testStop()
		{
			constexpr const auto TrackCount = 10;

			struct TestCase
			{
				int trackCount;
				int currentIndex;
				int expectedCurrentIndex;
			};

			const auto testCases = std::array {
				TestCase {0, -1, -1},
				TestCase {TrackCount, -1, -1},
				TestCase {TrackCount, 4, -1},
			};

			for(const auto& testCase: testCases)
			{
				auto testEnv = TestEnv<Test::AllFilesAvailableFileSystem>(testCase.trackCount);
				auto& playlist = testEnv.playlist;

				playlist.changeTrack(testCase.currentIndex);
				playlist.stop();

				QVERIFY(playlist.currentTrackIndex() == testCase.expectedCurrentIndex);
			}
		}

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void testMoveTracks()
		{
			constexpr const auto TrackCount = 8;

			struct TestCase
			{
				int trackCount;
				QList<int> tracksToMove;
				int targetIndex;
			};

			const auto testCases = std::array {
				TestCase {TrackCount, {}, 4},
				TestCase {TrackCount, {}, -1},
				TestCase {TrackCount, {}, TrackCount},
				TestCase {TrackCount, {1}, 4},
				TestCase {TrackCount, {1}, -1},
				TestCase {TrackCount, {1}, TrackCount},
				TestCase {TrackCount, {1}, TrackCount + 2},
				TestCase {TrackCount, {1, 2}, 4},
				TestCase {TrackCount, {1, 3, 5}, 6},
				TestCase {TrackCount, {4, 5, 6}, 2},
				TestCase {TrackCount, {4, 5, 6}, 0},
				TestCase {TrackCount, {4, 5, 6}, -1},
				TestCase {TrackCount, {4, 5, 6}, TrackCount},
				TestCase {TrackCount, {4, 5, 6}, TrackCount + 2},
				TestCase {TrackCount, {0, 1, 3, 5, 7}, 3},
			};

			for(const auto& testCase: testCases)
			{
				auto testEnv = TestEnv<Test::AllFilesAvailableFileSystem>(testCase.trackCount);
				auto& playlist = testEnv.playlist;
				auto tracks = playlist.tracks();

				moveTracks(playlist, toSet(testCase.tracksToMove), testCase.targetIndex, Playlist::Reason::Undefined);
				tracks.moveTracks(toSet(testCase.tracksToMove), testCase.targetIndex);

				const auto newTracks = playlist.tracks();
				const auto equal =
					std::equal(newTracks.begin(), newTracks.end(), tracks.begin(), [](const auto& t1, const auto& t2) {
						return t1.id() == t2.id();
					});

				QVERIFY(equal);
			}
		}

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void testRemoveTracks()
		{
			struct TestCase
			{
				int trackCount;
				QList<int> indexesToRemove;
				QList<int> expectedIds;
			};

			const auto testCases = std::array {
				TestCase {0, {0}, {}},
				TestCase {2, {}, {0, 1}},
				TestCase {2, {-1}, {0, 1}},
				TestCase {2, {3}, {0, 1}},
				TestCase {2, {1}, {0}},
				TestCase {1, {0}, {}},
				TestCase {3, {0, 1}, {2}},
				TestCase {3, {0, 5}, {1, 2}},
				TestCase {3, {0, 1, 2}, {}},
				TestCase {5, {0, 2, 4}, {1, 3}}
			};

			for(const auto& testCase: testCases)
			{
				auto testEnv = TestEnv<Test::AllFilesAvailableFileSystem>(testCase.trackCount);
				auto& playlist = testEnv.playlist;

				removeTracks(playlist, toSet(testCase.indexesToRemove), Playlist::Reason::Undefined);

				QVERIFY(playlist.count() == testCase.expectedIds.count());

				for(int i = 0; i < testCase.expectedIds.count(); i++)
				{
					const auto& track = playlist.tracks().at(i);
					QVERIFY(track.id() == testCase.expectedIds[i]);
				}
			}
		}

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void testInsert()
		{
			struct TestCase
			{
				int trackCount;
				QList<int> tracksToInsert;
				int targetIndex;
				QList<int> expectedIds;
			};

			const auto testCases = std::array {
				TestCase {0, {}, -1, {}},
				TestCase {0, {}, 1, {}},
				TestCase {3, {}, 2, {0, 1, 2}},
				TestCase {0, {10, 20}, -1, {10, 20}},
				TestCase {0, {10, 20}, 0, {10, 20}},
				TestCase {3, {10}, -1, {10, 0, 1, 2}},
				TestCase {3, {10}, 0, {10, 0, 1, 2}},
				TestCase {3, {10, 20}, 2, {0, 1, 10, 20, 2}},
				TestCase {3, {10, 20}, 3, {0, 1, 2, 10, 20}},
				TestCase {3, {10, 20}, 5, {0, 1, 2, 10, 20}}
			};

			for(const auto& testCase: testCases)
			{
				auto testEnv = TestEnv<Test::AllFilesAvailableFileSystem>(testCase.trackCount);
				auto& playlist = testEnv.playlist;
				auto tracks = MetaDataList {};
				for(const auto& i: testCase.tracksToInsert)
				{
					auto track = MetaData {QString("abc%1.mp3").arg(i)};
					track.setId(i);
					tracks << track;
				}

				insertTracks(playlist, tracks, testCase.targetIndex, Playlist::Reason::Undefined);

				QVERIFY(playlist.count() == testCase.expectedIds.count());
				for(int i = 0; i < testCase.expectedIds.count(); i++)
				{
					const auto& track = playlist.tracks().at(i);
					QVERIFY(track.id() == testCase.expectedIds[i]);
				}
			}
		}

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void testIndexWithoutDisabled()
		{
			struct TestCase
			{
				int trackCount;
				int currentTrack;
				QList<int> disabledTracks;
				int expectedIndex;
			};

			const auto testCases = std::array {
				TestCase {0, -1, {}, -1},
				TestCase {5, -1, {}, -1},
				TestCase {5, 3, {}, 3},
				TestCase {5, -1, {1, 2}, -1},
				TestCase {5, 0, {1, 2}, 0},
				TestCase {5, 3, {1, 2}, 1},
				TestCase {5, 0, {1, 2, 3, 4}, 0},
				TestCase {5, 3, {0, 1, 2}, 0},
			};

			for(const auto& testCase: testCases)
			{
				auto testEnv = TestEnv<Test::AllFilesAvailableFileSystem> {0};
				for(int i = 0; i < testCase.trackCount; i++)
				{
					auto track = MetaData {"abc"};
					track.setId(i);
					track.setDisabled(testCase.disabledTracks.contains(i));

					appendTracks(testEnv.playlist, MetaDataList {track}, Playlist::Reason::Undefined);
				}

				auto& playlist = testEnv.playlist;
				playlist.changeTrack(testCase.currentTrack);

				QVERIFY(Playlist::currentTrackWithoutDisabled(playlist) == testCase.expectedIndex);
			}
		}
};

QTEST_GUILESS_MAIN(PlaylistTest)

#include "PlaylistTest.moc"

