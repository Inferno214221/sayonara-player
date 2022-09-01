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

#include "Components/Playlist/ExternTracksPlaylistGenerator.h"
#include "Components/Playlist/Playlist.h"
#include "Components/Playlist/PlaylistModifiers.h"
#include "Interfaces/PlaylistInterface.h"
#include "Utils/Algorithm.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Parser/M3UParser.h"
#include "Utils/Settings/Settings.h"

#include <QStringList>
#include <QSignalSpy>

#include <functional> // bad_function_call

namespace
{
	class PlaylistCreatorMock :
		public PlaylistCreator
	{
		public:
			PlaylistCreatorMock(const Test::Playlist::PathTrackMap& pathTrackMap, int trackCount) :
				m_playManager {PlayManagerMock {}},
				m_playlist {std::make_shared<Playlist::Playlist>(0, "Playlist", &m_playManager)}
			{
				auto tracks = MetaDataList {};
				for(auto i = 0; i < trackCount; i++)
				{
					tracks << pathTrackMap[i].second;
				}

				Playlist::appendTracks(*m_playlist, tracks);
			}

			~PlaylistCreatorMock() override = default;

			PlaylistPtr playlist(int /*playlistIndex*/) override { return m_playlist; }

			PlaylistPtr playlistById(int /*playlistId*/) override { throw std::bad_function_call {}; }

			[[nodiscard]] QString
			requestNewPlaylistName(const QString& /*prefix*/) const override { throw std::bad_function_call {}; }

			int createPlaylist(const MetaDataList& /*tracks*/, const QString& /*name*/,
			                   bool /*temporary*/) override { throw std::bad_function_call {}; }

			int createPlaylist(const QStringList& /*pathList*/, const QString& /*name*/,
			                   bool /*temporary*/) override { throw std::bad_function_call {}; }

			int createPlaylist(const CustomPlaylist& /*customPlaylist*/) override { throw std::bad_function_call {}; }

			int createEmptyPlaylist(bool /*override*/) override { throw std::bad_function_call {}; }

			int createCommandLinePlaylist(const QStringList& /*pathList*/) override { throw std::bad_function_call {}; }

		private:
			PlayManagerMock m_playManager;
			PlaylistPtr m_playlist;
	};

	bool checkPlayList(const PlaylistPtr& playlist, const QStringList& paths)
	{
		if(::Playlist::count(*playlist) != paths.count())
		{
			return false;
		}

		const auto& tracks = playlist->tracks();
		for(auto i = 0; i < tracks.count(); i++)
		{
			const auto& track = tracks[i];
			if(track.filepath() != paths[i])
			{
				return false;
			}
		}

		return true;
	}
}

// access working directory with Test::Base::tempPath("somefile.txt");

class ExternTracksPlaylistGeneratorTest :
	public Test::Base
{
	Q_OBJECT

	public:
		ExternTracksPlaylistGeneratorTest() :
			Test::Base("ExternTracksPlaylistGeneratorTest"),
			m_pathTrackMap {Test::Playlist::createTrackFiles(Test::Base::tempPath())} {}

	private slots:
		void testInsertFiles();
		void testAddFiles();
		void testAddFilesWithAppend();
		void testAddFilesWithSingleDir();
		void testWithPlaylistFile();

	private:
		static void wait(ExternTracksPlaylistGenerator* generator);
		const Test::Playlist::PathTrackMap m_pathTrackMap;
};

void ExternTracksPlaylistGeneratorTest::testInsertFiles()
{
	auto* playlistCreator = new PlaylistCreatorMock(m_pathTrackMap, 5);
	auto playlist = playlistCreator->playlist(0);
	auto pathList = QStringList {};

	for(auto i = 5; i < m_pathTrackMap.count(); i++)
	{
		pathList << m_pathTrackMap[i].first;
	}

	auto externTracksPlaylistGenerator = ExternTracksPlaylistGenerator(playlistCreator, playlist);
	externTracksPlaylistGenerator.insertPaths(pathList, 2);

	wait(&externTracksPlaylistGenerator);

	const auto expectedPaths = QStringList()
		<< m_pathTrackMap[0].first
		<< m_pathTrackMap[1].first
		<< pathList
		<< m_pathTrackMap[2].first
		<< m_pathTrackMap[3].first
		<< m_pathTrackMap[4].first;

	QVERIFY(checkPlayList(playlist, expectedPaths));
}

void ExternTracksPlaylistGeneratorTest::testAddFiles()
{
	auto* playlistCreator = new PlaylistCreatorMock(m_pathTrackMap, 0);
	auto playlist = playlistCreator->playlist(0);
	auto pathList = QStringList {};

	for(const auto& [filepath, metadata]: m_pathTrackMap)
	{
		pathList << filepath;
	}

	auto externTracksPlaylistGenerator = ExternTracksPlaylistGenerator(playlistCreator, playlist);
	externTracksPlaylistGenerator.addPaths(pathList);

	wait(&externTracksPlaylistGenerator);

	QVERIFY(checkPlayList(playlist, pathList));
}

void ExternTracksPlaylistGeneratorTest::testAddFilesWithAppend()
{
	auto* playlistCreator = new PlaylistCreatorMock(m_pathTrackMap, 3);
	auto playlist = playlistCreator->playlist(0);
	auto pathList = QStringList {};

	Playlist::Mode mode;
	mode.setAppend(Playlist::Mode::On);
	playlist->setMode(mode);

	for(const auto& [filepath, metadata]: m_pathTrackMap)
	{
		pathList << filepath;
	}

	auto externTracksPlaylistGenerator = ExternTracksPlaylistGenerator(playlistCreator, playlist);
	externTracksPlaylistGenerator.addPaths(pathList);

	wait(&externTracksPlaylistGenerator);

	const auto expectedPaths = QStringList()
		<< m_pathTrackMap[0].first
		<< m_pathTrackMap[1].first
		<< m_pathTrackMap[2].first
		<< pathList;

	QVERIFY(checkPlayList(playlist, expectedPaths));
}

void ExternTracksPlaylistGeneratorTest::testAddFilesWithSingleDir()
{
	auto* playlistCreator = new PlaylistCreatorMock(m_pathTrackMap, 3);
	auto playlist = playlistCreator->playlist(0);
	auto pathList = QStringList {};

	for(const auto& [filepath, metadata]: m_pathTrackMap)
	{
		pathList << filepath;
	}

	pathList.sort();

	auto externTracksPlaylistGenerator = ExternTracksPlaylistGenerator(playlistCreator, playlist);
	externTracksPlaylistGenerator.addPaths({Test::Base::tempPath()});

	wait(&externTracksPlaylistGenerator);
	auto playlistFiles = QStringList {};
	Util::Algorithm::transform(playlist->tracks(), playlistFiles, [](const auto& track){
		return track.filepath();
	});

	playlistFiles.sort();

	QVERIFY(pathList == playlistFiles);
}

void ExternTracksPlaylistGeneratorTest::testWithPlaylistFile()
{
	auto* playlistCreator = new PlaylistCreatorMock(m_pathTrackMap, 3);
	auto playlist = playlistCreator->playlist(0);
	auto pathList = QStringList {};
	auto tracks = MetaDataList {};

	for(const auto& [filepath, track]: m_pathTrackMap)
	{
		pathList << filepath;
		tracks << track;
	}

	Util::Algorithm::sort(tracks, [](const auto& track1, const auto track2) {
		return (track1.filepath() < track2.filepath());
	});

	const auto playlistFilename = Test::Base::tempPath("bla.m3u");
	M3UParser::saveM3UPlaylist(playlistFilename, tracks, false);

	auto externTracksPlaylistGenerator = ExternTracksPlaylistGenerator(playlistCreator, playlist);
	externTracksPlaylistGenerator.addPaths({playlistFilename});

	wait(&externTracksPlaylistGenerator);

	pathList.sort();
	QVERIFY(checkPlayList(playlist, pathList));
}

void ExternTracksPlaylistGeneratorTest::wait(ExternTracksPlaylistGenerator* generator)
{
	auto spy = QSignalSpy(generator, &ExternTracksPlaylistGenerator::sigFinished);

	QVERIFY(spy.wait(1000));
	QCOMPARE(spy.count(), 1);
}

QTEST_GUILESS_MAIN(ExternTracksPlaylistGeneratorTest)

#include "ExternTracksPlaylistGeneratorTest.moc"
