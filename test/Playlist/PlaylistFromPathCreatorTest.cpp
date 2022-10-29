/*
 * Copyright (C) 2011-2022 Michael Lugmair
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
#include "test/Playlist/PlaylistTestUtils.h"

#include "Components/Playlist/PlaylistFromPathCreator.h"
#include "Components/Playlist/Playlist.h"
#include "Components/Playlist/PlaylistModifiers.h"
#include "Interfaces/PlaylistInterface.h"
#include "Utils/Parser/M3UParser.h"
#include "Utils/MetaData/MetaDataList.h"

#include "Utils/Algorithm.h"
#include "Utils/Logger/Logger.h"

#include <QSignalSpy>

#include <functional> // bad_function_call

// access working directory with Test::Base::tempPath("somefile.txt");

using Playlist::PlaylistFromPathCreator;

class PlaylistCreatorMock :
	public PlaylistCreator
{
	public:
		PlaylistCreatorMock() :
			m_playManager {PlayManagerMock {}} {}

		~PlaylistCreatorMock() override = default;

		PlaylistPtr playlist(int playlistIndex) override { return m_playlists[playlistIndex]; }

		[[nodiscard]] QString
		requestNewPlaylistName(const QString& /*prefix*/) const override
		{
			return QString("%1").arg(m_playlists.count() + 1);
		}

		int createPlaylist(const MetaDataList& /*tracks*/, const QString& name,
		                   bool /*temporary*/) override
		{
			int index = m_playlists.count();
			auto playlist =
				std::make_shared<::Playlist::Playlist>(index, name, &m_playManager);

			playlist->setId(index);
			m_playlists << playlist;

			return index;
		}

		PlaylistPtr playlistById(int /*playlistId*/) override { throw std::bad_function_call {}; }

		int createPlaylist(const QStringList& /*pathList*/, const QString& /*name*/, bool /*temporary*/,
		                   PlaylistFromPathCreator* /*creator*/) override
		{
			throw std::bad_function_call {};
		}

		int createPlaylist(const CustomPlaylist& /*customPlaylist*/) override { throw std::bad_function_call {}; }

		int createEmptyPlaylist(bool /*override*/) override { throw std::bad_function_call {}; }

		int createCommandLinePlaylist(const QStringList& /*pathList*/,
		                              PlaylistFromPathCreator* /*creator*/) override
		{
			throw std::bad_function_call {};
		}

		[[nodiscard]] int count() const
		{
			return m_playlists.count();
		}

		[[nodiscard]] QList<PlaylistPtr> playlists() const
		{
			return m_playlists;
		}

	private:
		PlayManagerMock m_playManager;
		QList<PlaylistPtr> m_playlists;
};

class PlaylistFromPathCreatorTest :
	public Test::Base
{
	Q_OBJECT

	public:
		PlaylistFromPathCreatorTest() :
			Test::Base("PlaylistFromPathCreatorTest"),
			m_pathTrackMap {Test::Playlist::createTrackFiles(Test::Base::tempPath())} {}

	private slots:
		[[maybe_unused]] void testSinglePlaylist();
		[[maybe_unused]] void testPlaylistWithPlaylistFile();

	private: // NOLINT(readability-redundant-access-specifiers)
		static void wait(PlaylistFromPathCreator* creator);
		const Test::Playlist::PathTrackMap m_pathTrackMap;
};

[[maybe_unused]] void PlaylistFromPathCreatorTest::testSinglePlaylist()
{
	auto playlistCreator = PlaylistCreatorMock();
	auto* playlistFromPathCreator = PlaylistFromPathCreator::create(&playlistCreator);

	auto pathList = QStringList {};

	for(const auto& [filepath, track]: m_pathTrackMap)
	{
		pathList << filepath;
	}

	playlistFromPathCreator->createPlaylists({Test::Base::tempPath()}, "Some Playlist", true);

	wait(playlistFromPathCreator);

	QVERIFY(playlistCreator.count() == 1);

	const auto playlist = playlistCreator.playlists()[0];
	QVERIFY(Playlist::count(*playlist) == m_pathTrackMap.count());
}

[[maybe_unused]] void PlaylistFromPathCreatorTest::testPlaylistWithPlaylistFile()
{
	auto playlistCreator = PlaylistCreatorMock();
	auto* playlistFromPathCreator = PlaylistFromPathCreator::create(&playlistCreator);

	auto pathList = QStringList {};
	auto tracks = MetaDataList {};

	for(const auto& [filepath, track]: m_pathTrackMap)
	{
		pathList << filepath;
		tracks << track;
	}

	constexpr const auto indexToRemove = 5;
	tracks.removeTracks(indexToRemove, tracks.count() - 1);

	M3UParser::saveM3UPlaylist(Test::Base::tempPath("bla.m3u"), tracks, false);

	const auto paths = QStringList {pathList.first(), Test::Base::tempPath("bla.m3u")};
	playlistFromPathCreator->createPlaylists(paths, "Some Playlist", true);

	wait(playlistFromPathCreator);

	const auto playlist = playlistCreator.playlists()[1];
	QVERIFY(playlistCreator.count() == 2);
	QVERIFY(Playlist::count(*playlist) == 5);
}

[[maybe_unused]] void PlaylistFromPathCreatorTest::wait(PlaylistFromPathCreator* creator)
{
	auto spy = QSignalSpy(creator, &PlaylistFromPathCreator::sigAllPlaylistsCreated);

	QVERIFY(spy.wait(1000000));
	QCOMPARE(spy.count(), 1);
}

QTEST_GUILESS_MAIN(PlaylistFromPathCreatorTest)

#include "PlaylistFromPathCreatorTest.moc"
