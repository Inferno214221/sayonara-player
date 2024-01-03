/* PlaylistFileTest.cpp
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
#include "test/Common/FileSystemMock.h"

#include "Utils/Parser/M3UParser.h"
#include "Utils/FileUtils.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Utils.h"
#include "Utils/Tagging/TagReader.h"


// access working directory with Test::Base::tempPath("somefile.txt");

using Test::Playlist::PathTrackMap;

namespace
{
	constexpr const auto ExpectedPlaylist = R"(#EXTM3U
#EXTINF:100,Artist1 - Title1
%1path/mp3test.mp3

#EXTINF:200,Artist2 - Title2
%1path/to/mp3test.mp3

#EXTINF:300,Artist3 - Title3
%1path/to/somewhere/mp3test.mp3

#EXTINF:400,Artist4 - Title4
%1path/to/somewhere/else/mp3test.mp3

#EXTINF:500,Artist5 - Title5
%1path/to/another/mp3test.mp3

#EXTINF:600,Artist6 - Title6
%1path/to/another/dir/mp3test.mp3
)";
}

class PlaylistFileTest :
	public Test::Base
{
	Q_OBJECT

	public:
		PlaylistFileTest() :
			Test::Base("PlaylistFileTest")
		{
			const auto basePath = Test::Base::tempPath();
			m_tracks = Test::Playlist::createTrackFiles(basePath);
		}

	private:
		class TagReader :
			public Tagging::TagReader
		{
			public:
				explicit TagReader(Util::FileSystemPtr fileSystem) :
					m_fileSystem {std::move(fileSystem)} {}

				~TagReader() override = default;

				std::optional<MetaData> readMetadata(const QString& filepath) override
				{
					if(!m_fileSystem->exists(filepath))
					{
						return std::nullopt;
					}

					auto track = MetaData {};
					track.setFilepath(filepath);
					track.setTitle(filepath);
					track.setAlbum(filepath);
					track.setArtist(filepath);

					return {track};
				}

				[[nodiscard]] bool isCoverSupported(const QString& /*filepath*/) const override { return false; }

			private:
				Util::FileSystemPtr m_fileSystem;
		};

	private slots:
		void parseStandardM3U();
		void parseExtendedM3U();
		void writeM3uAbsolute();
		void writeM3uRelative();

	private:
		PathTrackMap m_tracks;
};

void PlaylistFileTest::parseStandardM3U()
{
	const QString content = R"m3u(
	file1.mp3

	file2.ogg
	/non/existent/path
	/path/to/file.flac
    # don't parse this
	/other/path/some/file.aac
	)m3u";

	constexpr const auto* PlaylistName = "/current/playlist.m3u";

	auto fileSystem = std::make_shared<Test::FileSystemMock>(
		QMap<QString, QStringList> {
			{"/current",         {"playlist.m3u", "file1.mp3", "file2.ogg"}},
			{"/path/to",         {"file.flac"}},
			{"/other/path/some", {"file.aac"}},
		}
	);

	fileSystem->writeFile(content.toLocal8Bit(), PlaylistName);

	auto tagReader = std::make_shared<PlaylistFileTest::TagReader>(fileSystem);

	auto parser = M3UParser(PlaylistName, fileSystem, tagReader);
	const auto tracks = parser.tracks();

	QVERIFY(tracks.size() == 4);
	QVERIFY(tracks[0].filepath() == "/current/file1.mp3");
	QVERIFY(tracks[1].filepath() == "/current/file2.ogg");
	QVERIFY(tracks[2].filepath() == "/path/to/file.flac");
	QVERIFY(tracks[3].filepath() == "/other/path/some/file.aac");
}

void PlaylistFileTest::parseExtendedM3U()
{
	const QString content = R"m3u(
	#EXTM3U

	#EXTINF:221,Queen - Bohemian Rhapsody
	file1.mp3

	#EXTINF:473, Dire Straits - Walk Of Life
	file2.ogg
	# This is a comment
	#EXTINF:264 ,冨田勲 - Boléro
	/path/to/bolero.flac
	#EXTINF: 504,  Bob Marley - Buffalo Soldier

/some/path/to/buffalo soldier.aac
	)m3u";

	auto fileSystem = std::make_shared<Test::FileSystemMock>(
		QMap<QString, QStringList> {
			{"/current",      {"playlist.m3u", "file1.mp3", "file2.ogg"}},
			{"/path/to",      {"bolero.flac"}},
			{"/some/path/to", {"buffalo soldier.aac"}},
		}
	);

	constexpr const auto* PlaylistName = "/current/playlist.m3u";

	fileSystem->writeFile(content.toLocal8Bit(), PlaylistName);
	auto tagReader = std::make_shared<PlaylistFileTest::TagReader>(fileSystem);

	auto parser = M3UParser(PlaylistName, fileSystem, nullptr);
	const auto tracks = parser.tracks();

	QVERIFY(tracks.size() == 4);

	QVERIFY(tracks[0].durationMs() == 221'000);
	QVERIFY(tracks[0].artist() == "Queen");
	QVERIFY(tracks[0].title() == "Bohemian Rhapsody");
	QVERIFY(tracks[0].filepath() == "/current/file1.mp3");

	QVERIFY(tracks[1].durationMs() == 473'000);
	QVERIFY(tracks[1].artist() == "Dire Straits");
	QVERIFY(tracks[1].title() == "Walk Of Life");
	QVERIFY(tracks[1].filepath() == "/current/file2.ogg");

	QVERIFY(tracks[2].durationMs() == 264'000);
	QVERIFY(tracks[2].artist() == QString::fromUtf8("冨田勲"));
	QVERIFY(tracks[2].title() == QString::fromUtf8("Boléro"));
	QVERIFY(tracks[2].filepath() == "/path/to/bolero.flac");

	QVERIFY(tracks[3].durationMs() == 504'000);
	QVERIFY(tracks[3].artist() == "Bob Marley");
	QVERIFY(tracks[3].title() == "Buffalo Soldier");
	QVERIFY(tracks[3].filepath() == "/some/path/to/buffalo soldier.aac");
}

void PlaylistFileTest::writeM3uAbsolute()
{
	MetaDataList tracks;
	for(const auto& [path, track]: m_tracks)
	{
		tracks << track;
	}

	const auto playlistName = Base::tempPath("absolutePlaylist.m3u");
	M3UParser::saveM3UPlaylist(playlistName, tracks, false);

	QString content;
	Util::File::readFileIntoString(playlistName, content);

	const auto absoluteExpectedPlaylist = QString(ExpectedPlaylist).arg(Base::tempPath() + '/');
	QVERIFY(content == absoluteExpectedPlaylist);
}

void PlaylistFileTest::writeM3uRelative()
{
	MetaDataList tracks;
	for(const auto& [path, track]: m_tracks)
	{
		tracks << track;
	}

	const auto playlistName = Base::tempPath("relativePlaylist.m3u");
	M3UParser::saveM3UPlaylist(playlistName, tracks, true);

	QString content;
	Util::File::readFileIntoString(playlistName, content);

	const auto relativeExpectedPlaylist = QString(ExpectedPlaylist).arg("");
	QVERIFY(content == relativeExpectedPlaylist);
}

QTEST_GUILESS_MAIN(PlaylistFileTest)

#include "PlaylistFileTest.moc"
