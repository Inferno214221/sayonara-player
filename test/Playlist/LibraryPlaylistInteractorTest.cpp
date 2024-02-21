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
#include "Common/PlaylistMocks.h"
#include "Components/Playlist/LibraryPlaylistInteractor.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Set.h"

// access working directory with Test::Base::tempPath("somefile.txt");
namespace
{
	enum PlayBehavior
	{
		PlayOnDoubleClickIfStopped,
		PlayOnDoubleClickImmediately,
		DontPlayOnDoubleClick
	};
}

class LibraryPlaylistInteractorTest :
	public Test::Base
{
	Q_OBJECT

	public:
		LibraryPlaylistInteractorTest() :
			Test::Base("LibraryPlaylistInteractorTest") {}

	private slots:

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void testPlaystateIsCorrectAfterDoubleClick()
		{
			struct TestCase
			{
				PlayBehavior playBehavior;
				PlayState initialState;
				PlayState expectedState;
			};

			const auto testCases = std::array {
				TestCase {DontPlayOnDoubleClick, PlayState::Stopped, PlayState::Stopped},
				TestCase {PlayOnDoubleClickIfStopped, PlayState::Stopped, PlayState::Playing},
				TestCase {PlayOnDoubleClickImmediately, PlayState::Stopped, PlayState::Playing},
				TestCase {PlayOnDoubleClickImmediately, PlayState::Paused, PlayState::Playing},
			};

			// needs to be https otherwise the file check will fail
			const auto tracks = QStringList {"https://a.mp3", "https://b.mp3"};

			for(const auto& testCase: testCases)
			{
				auto playManager = std::make_shared<PlayManagerMock>();
				playManager->setPlaystate(testCase.initialState);

				auto playlistHandler = PlaylistHandlerMock(playManager);
				auto* libPlaylistInteractor =
					LibraryPlaylistInteractor::create(&playlistHandler, &playlistHandler, playManager.get());

				SetSetting(Set::Lib_DC_PlayIfStopped, testCase.playBehavior == PlayOnDoubleClickIfStopped);
				SetSetting(Set::Lib_DC_PlayImmediately, testCase.playBehavior == PlayOnDoubleClickImmediately);
				SetSetting(Set::Lib_DC_DoNothing, testCase.playBehavior == DontPlayOnDoubleClick);

				// double click
				libPlaylistInteractor->createPlaylist(tracks, true);

				QVERIFY(playManager->playstate() == testCase.expectedState);

				delete libPlaylistInteractor;
			}
		}

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void testCurrentTrackIsCorrect()
		{
			const auto playlistTrack = MetaData {"https://a.mp3"};
			const auto initialTrack = MetaData {"https://sound.mp3"};

			struct TestCase
			{
				PlayBehavior playBehavior;
				PlayState initialState;
				MetaData expectedTrack;
			};

			const auto testCases = std::array {
				TestCase {DontPlayOnDoubleClick, PlayState::Stopped, initialTrack},
				TestCase {DontPlayOnDoubleClick, PlayState::Playing, initialTrack},
				TestCase {PlayOnDoubleClickIfStopped, PlayState::Stopped, playlistTrack},
				TestCase {PlayOnDoubleClickIfStopped, PlayState::Paused, playlistTrack},
				TestCase {PlayOnDoubleClickIfStopped, PlayState::Playing, initialTrack},
				TestCase {PlayOnDoubleClickImmediately, PlayState::Stopped, playlistTrack},
				TestCase {PlayOnDoubleClickImmediately, PlayState::Playing, playlistTrack},
				TestCase {PlayOnDoubleClickImmediately, PlayState::Paused, playlistTrack},
			};

			for(const auto& testCase: testCases)
			{
				auto playManager = std::make_shared<PlayManagerMock>();
				playManager->changeCurrentTrack(initialTrack, 0);
				playManager->setPlaystate(testCase.initialState);

				auto playlistHandler = PlaylistHandlerMock(playManager);
				auto* libPlaylistInteractor =
					LibraryPlaylistInteractor::create(&playlistHandler, &playlistHandler, playManager.get());

				SetSetting(Set::Lib_DC_PlayIfStopped, testCase.playBehavior == PlayOnDoubleClickIfStopped);
				SetSetting(Set::Lib_DC_PlayImmediately, testCase.playBehavior == PlayOnDoubleClickImmediately);
				SetSetting(Set::Lib_DC_DoNothing, testCase.playBehavior == DontPlayOnDoubleClick);

				// double click
				libPlaylistInteractor->createPlaylist(MetaDataList {playlistTrack}, true);

				QVERIFY(playManager->currentTrack() == testCase.expectedTrack);

				delete libPlaylistInteractor;
			}
		}

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void testRandomTrackIsChosen()
		{
			constexpr const auto trackCount = 1000;
			constexpr const auto numTries = 100;

			auto playlistTracks = MetaDataList {};
			for(int i = 0; i < trackCount; i++)
			{
				playlistTracks << MetaData {QString("https://%1.mp3").arg(i)};
			}

			const auto initialTrack = MetaData {"https://sound.mp3"};

			struct TestCase
			{
				bool isShuffle;
				bool startRandomTrackOnShuffle;
				int exepctedSamplesMin;
				int exepctedSamplesMax;
			};

			const auto testCases = std::array {
				TestCase {false, false, 1, 1},
				TestCase {false, true, 1, 1},
				TestCase {true, false, 1, 1},
				TestCase {true, true, 2, numTries},
			};

			for(const auto& testCase: testCases)
			{
				Util::Set<int> initialTracks;

				for(int i = 0; i < numTries; i++)
				{
					auto playManager = std::make_shared<PlayManagerMock>();
					playManager->changeCurrentTrack(initialTrack, 0);
					playManager->setPlaystate(PlayState::Stopped);

					auto playlistHandler = PlaylistHandlerMock(playManager);
					auto* libPlaylistInteractor =
						LibraryPlaylistInteractor::create(&playlistHandler, &playlistHandler, playManager.get());

					SetSetting(Set::Lib_DC_PlayIfStopped, true);

					auto playlistMode = PlaylistMode {};
					playlistMode.setShuffle(testCase.isShuffle, testCase.isShuffle);
					SetSetting(Set::PL_Mode, playlistMode);
					SetSetting(Set::PL_StartAtRandomTrackOnShuffle, testCase.startRandomTrackOnShuffle);

					// double click
					libPlaylistInteractor->createPlaylist(playlistTracks, true);
					auto playlist = playlistHandler.playlist(playlistHandler.currentIndex());
					initialTracks << playlist->currentTrackIndex();

					delete libPlaylistInteractor;
				}

				QVERIFY(initialTracks.count() >= testCase.exepctedSamplesMin);
				QVERIFY(initialTracks.count() <= testCase.exepctedSamplesMax);
			}
		}
};

QTEST_GUILESS_MAIN(LibraryPlaylistInteractorTest)

#include "LibraryPlaylistInteractorTest.moc"
