/* PlaylistHandlerTest.cpp
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

#include "test/Common/SayonaraTest.h"
#include "test/Common/PlayManagerMock.h"
#include "test/Common/PlaylistMocks.h"
#include "test/Common/TestTracks.h"

#include "Components/PlayManager/PlayManager.h"
#include "Components/Playlist/LocalPathPlaylistCreator.h"
#include "Components/Playlist/Playlist.h"
#include "Components/Playlist/PlaylistHandler.h"
#include "Components/Playlist/PlaylistModifiers.h"
#include "Utils/FileUtils.h"
#include "Utils/MetaData/MetaData.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Settings/Settings.h"

#include <QSignalSpy>

#include <memory>
#include <utility>

// access working directory with Test::Base::tempPath("somefile.txt");

namespace
{
	class PlaylistFromPathCreatorMock :
		public Playlist::LocalPathPlaylistCreator
	{
		public:
			PlaylistFromPathCreatorMock(PlaylistCreator* creator, MetaDataList tracks) :
				m_creator {creator},
				m_tracks {std::move(tracks)} {}

			int createPlaylists(const QStringList& /*paths*/, const QString& name, bool temporary) override
			{
				const auto index = m_creator->createPlaylist(m_tracks, name, temporary);
				emit sigAllPlaylistsCreated(index);

				return index;
			}

		private:
			PlaylistCreator* m_creator;
			MetaDataList m_tracks;
	};

	Playlist::LocalPathPlaylistCreator*
	makePlaylistFromPathCreator(PlaylistCreator* creator, const MetaDataList& tracks)
	{
		return new PlaylistFromPathCreatorMock(creator, tracks);
	}

	void distributeTracksOnPlaylists(std::shared_ptr<Playlist::Handler>& plh, const int playlistCount)
	{
		const auto tracks = Test::createTracks();
		for(auto i = 0; i < playlistCount - 1; i++)
		{
			plh->createEmptyPlaylist();
		}

		auto i = 0;
		for(const auto& track: tracks)
		{
			auto& playlist = *plh->playlist(i);
			Playlist::appendTracks(playlist, MetaDataList {track}, Playlist::Reason::Undefined);

			i = (i + 1) % playlistCount;
		}
	}
}

class PlaylistHandlerTest :
	public Test::Base
{
	Q_OBJECT

	public:
		PlaylistHandlerTest() :
			Test::Base("PlaylistHandlerTest"),
			m_playManager(new PlayManagerMock()) {}

	private:
		PlayManager* m_playManager;

		std::shared_ptr<Playlist::Handler> createHandler()
		{
			return std::make_shared<Playlist::Handler>(m_playManager, std::make_shared<PlaylistLoaderMock>());
		}

	private slots: // NOLINT(readability-redundant-access-specifiers)
		[[maybe_unused]] void createTest();
		[[maybe_unused]] void closeTest();
		[[maybe_unused]] void currentIndexTest();
		[[maybe_unused]] void activeIndexTest();
		[[maybe_unused]] void createPlaylistFromFiles();
		[[maybe_unused]] void createCommandLinePlaylistSettings();
		[[maybe_unused]] void createCommandLinePlaylist();
		[[maybe_unused]] void testEmptyPlaylistDeletion();
		[[maybe_unused]] void testEmptyPlaylistsDeletedOnShutdown();

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void testIfClosedSignalIsSentBeforeNewPlaylistIsAdded()
		{
			auto plh = createHandler();

			auto isPlaylistAdded = false;
			auto isPlaylistClosed = false;

			connect(plh.get(), &Playlist::Handler::sigNewPlaylistAdded, this, [&](const auto& /*i*/) {
				isPlaylistAdded = true;
				QVERIFY(isPlaylistClosed);
			});

			connect(plh.get(), &Playlist::Handler::sigPlaylistClosed, this, [&]() {
				isPlaylistClosed = true;
				QVERIFY(!isPlaylistAdded);
			});

			plh->closePlaylist(0);
		}
};

[[maybe_unused]] void PlaylistHandlerTest::createTest() // NOLINT(readability-function-cognitive-complexity)
{
	auto plh = createHandler();
	QVERIFY(plh->count() == 1);
	QVERIFY(plh->activeIndex() == 0);

	auto index = plh->createEmptyPlaylist(false);
	QVERIFY(plh->playlist(index)->index() == index);
	QVERIFY(index == 1);
	QVERIFY(plh->count() == 2);

	index = plh->createEmptyPlaylist(false);
	QVERIFY(plh->playlist(index)->index() == index);
	QVERIFY(index == 2);
	QVERIFY(plh->count() == 3);

	index = plh->createEmptyPlaylist(true);
	QVERIFY(plh->playlist(index)->index() == index);
	QVERIFY(index == 2);
	QVERIFY(plh->count() == 3);

	plh->shutdown();
}

[[maybe_unused]] void PlaylistHandlerTest::closeTest() // NOLINT(readability-function-cognitive-complexity)
{
	auto plh = createHandler();
	plh->createEmptyPlaylist(false);
	QVERIFY(plh->count() == 2);

	plh->closePlaylist(-4);
	QVERIFY(plh->count() == 2);

	plh->closePlaylist(0);
	QVERIFY(plh->count() == 1);
	QVERIFY(plh->playlist(0)->index() == 0);
	QVERIFY(plh->activeIndex() == 0);

	constexpr const auto MaxPlaylistCount = 50;
	for(auto i = 0; i < MaxPlaylistCount; i++)
	{
		plh->closePlaylist(0);
		QVERIFY(plh->count() == 1);
		QVERIFY(plh->playlist(0)->index() == 0);
		QVERIFY(plh->activeIndex() == 0);
	}
}

[[maybe_unused]] void PlaylistHandlerTest::currentIndexTest()
{
	auto plh = createHandler();

	QVERIFY(plh->currentIndex() == 0); // one playlist
	plh->setCurrentIndex(20); // NOLINT(readability-magic-numbers)
	QVERIFY(plh->currentIndex() == 0);

	plh->createEmptyPlaylist(false); // two playlists
	QVERIFY(plh->currentIndex() == 1);

	plh->createEmptyPlaylist(false); // three playlists
	QVERIFY(plh->currentIndex() == 2);

	constexpr const auto InvalidIndex = 5;
	plh->setCurrentIndex(InvalidIndex);
	QVERIFY(plh->currentIndex() == 2);

	constexpr const auto ValidCurrentIndex = 0;
	plh->setCurrentIndex(ValidCurrentIndex);
	QVERIFY(plh->currentIndex() == 0);

	constexpr const auto NewCurrentIndex = 2;
	plh->setCurrentIndex(NewCurrentIndex);
	plh->closePlaylist(NewCurrentIndex);
	QVERIFY(plh->currentIndex() == 1);

	constexpr const auto LastPlaylistIndex = 0;
	plh->closePlaylist(LastPlaylistIndex);
	QVERIFY(plh->currentIndex() == 0);
}

[[maybe_unused]] void PlaylistHandlerTest::activeIndexTest() // NOLINT(readability-function-cognitive-complexity)
{
	auto plh = createHandler();
	QVERIFY(plh->activeIndex() == 0); // one playlist

	plh->createEmptyPlaylist();
	QVERIFY(plh->activeIndex() == 1); // two playlist

	plh->createEmptyPlaylist();
	QVERIFY(plh->activeIndex() == 2); // three playlists

	plh->closePlaylist(0);
	QVERIFY(plh->activeIndex() == 1); // two playlists

	MetaDataList tracks;
	constexpr const auto TrackCount = 10;
	for(int i = 0; i < TrackCount; i++)
	{
		// file must exist
		const auto filename = Test::Base::tempPath(QString("file%1.mp3").arg(i));
		Util::File::writeFile(QByteArray {}, filename);

		tracks << MetaData {filename};
	}

	auto index = plh->createPlaylist(tracks, "new-playlist"); // index = 2

	plh->setCurrentIndex(0);
	QVERIFY(plh->activeIndex() == 0);

	auto playlist = plh->playlist(index);
	auto success = playlist->changeTrack(4, 0);
	QVERIFY(success);

	playlist->play();
	QVERIFY(plh->currentIndex() == 0);
	QVERIFY(plh->activeIndex() == 2);

	playlist->stop();
	QVERIFY(plh->activeIndex() == plh->currentIndex());
}

[[maybe_unused]] void PlaylistHandlerTest::createPlaylistFromFiles()
{
	const auto paths = QStringList() << "path2.m3u" << "path1.mp3" << "path3.pls";
	auto plh = createHandler();
	QVERIFY(plh->count() == 1); // empty playlist

	auto spy = QSignalSpy(plh.get(), &Playlist::Handler::sigNewPlaylistAdded);
	plh->createPlaylist(paths, "Test");

	QVERIFY(spy.count() == 3);
	QVERIFY(plh->count() == 4);

	const auto names = QStringList {"path2", "path3", "Test"};
	auto playlistNames = QStringList {
		plh->playlist(1)->name(),
		plh->playlist(2)->name(),
		plh->playlist(3)->name()
	};

	playlistNames.sort(Qt::CaseInsensitive);
	QVERIFY(names == playlistNames);
}

[[maybe_unused]] void PlaylistHandlerTest::createCommandLinePlaylistSettings()
{
	struct TestCase
	{
		bool createExtraPlaylist {false};
		bool setPlaylistName {false};
		QString playlistName;
		int callCount {0};
		int expectedPlaylists {0};
	};

	const auto testCases = {
		TestCase {false, false, QString {}, 3, 3},
		TestCase {true, false, QString {}, 3, 1},
		TestCase {true, true, "Extra", 3, 1}
	};

	for(const auto& testCase: testCases)
	{
		SetSetting(Set::PL_CreateFilesystemPlaylist, testCase.createExtraPlaylist);
		SetSetting(Set::PL_SpecifyFileystemPlaylistName, testCase.setPlaylistName);
		SetSetting(Set::PL_FilesystemPlaylistName, testCase.playlistName);

		auto plh = createHandler();

		for(int i = 0; i < testCase.callCount; i++)
		{
			plh->createCommandLinePlaylist({}, makePlaylistFromPathCreator(plh.get(), Test::createTracks()));
		}

		QVERIFY(plh->count() == testCase.expectedPlaylists);
	}
}

[[maybe_unused]] void PlaylistHandlerTest::createCommandLinePlaylist()
{
	auto plh = createHandler();

	plh->createEmptyPlaylist();
	plh->createEmptyPlaylist();

	QVERIFY(plh->count() == 3);

	auto spy = QSignalSpy(plh.get(), &Playlist::Handler::sigNewPlaylistAdded);
	const auto paths = QStringList() << "path2.m3u" << "path1.mp3" << "path3.pls";
	plh->createCommandLinePlaylist(paths, nullptr);
	QVERIFY(spy.count() == 3);
	QVERIFY(plh->count() == 6);
}

[[maybe_unused]] void PlaylistHandlerTest::testEmptyPlaylistDeletion()
{
	auto plh = createHandler();

	QVERIFY(plh->count() == 1);
	const auto firstPlaylist = plh->playlist(0);
	QVERIFY(firstPlaylist->tracks().isEmpty());

	const auto playlistName = firstPlaylist->name();
	const auto playlistId = firstPlaylist->id();
	const auto tracks = Test::createTracks();

	const auto index = plh->createCommandLinePlaylist({}, makePlaylistFromPathCreator(plh.get(), tracks));
	const auto newPlaylist = plh->playlist(index);

	QVERIFY(plh->count() == 1);
	QVERIFY(index == 0);
	QVERIFY(newPlaylist->name() != playlistName);
	QVERIFY(newPlaylist->id() != playlistId);
	QVERIFY(newPlaylist->count() == tracks.count());
}

[[maybe_unused]] void PlaylistHandlerTest::testEmptyPlaylistsDeletedOnShutdown()
{
	constexpr const auto PlaylistCount = 5;

	auto plh = createHandler();

	distributeTracksOnPlaylists(plh, PlaylistCount);

	QVERIFY(plh->count() == PlaylistCount);

	Playlist::clear(*plh->playlist(0), Playlist::Reason::Undefined);
	Playlist::clear(*plh->playlist(2), Playlist::Reason::Undefined);
	Playlist::clear(*plh->playlist(4), Playlist::Reason::Undefined);

	auto spy = QSignalSpy(plh.get(), &Playlist::Handler::sigPlaylistClosed);

	plh->shutdown();

	QVERIFY(spy.count() == 3);
}

QTEST_GUILESS_MAIN(PlaylistHandlerTest)

#include "PlaylistHandlerTest.moc"
