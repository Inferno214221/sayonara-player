/*
 * Copyright (C) 2011-2023 Michael Lugmair
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
#include "Common/FileSystemMock.h"
#include "Common/TaggingMocks.h"

#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Parser/PLSParser.h"

#include <QList>

// access working directory with Test::Base::tempPath("somefile.txt");

namespace
{
	struct PlaylistEntry
	{
		QString path;
		QString title;
		MilliSeconds duration;
	};

	QString createPlaylist(const QList<PlaylistEntry>& entries)
	{
		auto convertedEntries = QStringList {};
		int index = 1;
		for(const auto& entry: entries)
		{
			convertedEntries << QString {"File%1=%2"}.arg(index).arg(entry.path);
			convertedEntries << QString {"Title%1=%2"}.arg(index).arg(entry.title);
			convertedEntries << QString {"Length%1=%2"}.arg(index).arg(entry.duration);
			convertedEntries << QString {};
			index++;
		}

		return QString {
			"[playlist]\n\n"
			"NumberOfEntries=%1\n\n"
			"%2\n"
			"Version=2"
		}.arg(entries.count()).arg(convertedEntries.join("\n"));
	}

	class TagReaderMock :
		public Tagging::TagReader
	{
		public:
			~TagReaderMock() override = default;

			std::optional<MetaData> readMetadata(const QString& /*filepath*/) override
			{
				return std::nullopt;
			}
	};

	class VerboseTagReaderMock :
		public Tagging::TagReader
	{
		public:
			VerboseTagReaderMock(QString title, const MilliSeconds lengthMs) :
				m_title {std::move(title)},
				m_lengthMs {lengthMs} {}

			~VerboseTagReaderMock() override = default;

			std::optional<MetaData> readMetadata(const QString& filepath) override
			{
				auto track = MetaData {filepath};
				track.setTitle(m_title);
				track.setDurationMs(m_lengthMs);

				return track;
			}

		private:
			QString m_title;
			MilliSeconds m_lengthMs {0};
	};
}

class PlsParserTest :
	public Test::Base
{
	Q_OBJECT

	public:
		PlsParserTest() :
			Test::Base("PlsParserTest") {}

	private slots:

		[[maybe_unused]] void testAbsoluteAndRelativePaths() // NOLINT(readability-convert-member-functions-to-static)
		{
			const auto fileSystem = std::make_shared<Test::FileSystemMock>(
				QMap<QString, QStringList> {
					{"/path/to/playlist",        {"playlist.pls", "file1.mp3"}},
					{"/path/to/playlist/subdir", {"file2.mp3"}},
					{"/path/to",                 {"file3.mp3"}}});

			const auto samplePlaylist = createPlaylist(
				QList<PlaylistEntry> {
					{"file1.mp3",                          "Some track1", 1},
					{"/path/to/playlist/file1.mp3",        "Some track1", 1},
					{"subdir/file2.mp3",                   "Some track2", 1},
					{"/path/to/playlist/subdir/file2.mp3", "Some track2", 1},
					{"../file3.mp3",                       "Some track3", 1},
					{"/path/to/file3.mp3",                 "Some track3", 1}});

			fileSystem->writeFile(samplePlaylist.toLocal8Bit(), "/path/to/playlist/playlist.pls");

			auto parser = PLSParser("/path/to/playlist/playlist.pls", fileSystem, std::make_shared<TagReaderMock>());
			const auto tracks = parser.tracks();

			QVERIFY(tracks[0].filepath() == "/path/to/playlist/file1.mp3");
			QVERIFY(tracks[1].filepath() == "/path/to/playlist/file1.mp3");
			QVERIFY(tracks[2].filepath() == "/path/to/playlist/subdir/file2.mp3");
			QVERIFY(tracks[3].filepath() == "/path/to/playlist/subdir/file2.mp3");
			QVERIFY(tracks[4].filepath() == "/path/to/file3.mp3");
			QVERIFY(tracks[5].filepath() == "/path/to/file3.mp3");
		}

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static,readability-function-cognitive-complexity)
		[[maybe_unused]] void testPlaylistWith3Files()
		{
			const auto samplePlaylist = createPlaylist(
				QList<PlaylistEntry> {
					{"http://streamexample.com:80", "Online Radio", -1},
					{"http://example.com/song.mp3", "Online Song",  286},
					{"/home/root/album.flac",       "Local track",  3487}});

			for(const auto& enableTagReader: {true, false})
			{
				const auto fileSystem = std::make_shared<Test::FileSystemMock>(
					QMap<QString, QStringList> {
						{"/",          {"playlist.pls"}},
						{"/home/root", {"album.flac"}}
					});

				fileSystem->writeFile(samplePlaylist.toLocal8Bit(), "/playlist.pls");

				auto parser = enableTagReader
				              ? PLSParser("/playlist.pls", fileSystem, std::make_shared<TagReaderMock>())
				              : PLSParser("/playlist.pls", fileSystem, nullptr);

				const auto tracks = parser.tracks();

				QVERIFY(tracks[0].title() == "Online Radio");
				QVERIFY(tracks[0].durationMs() == 0);
				QVERIFY(tracks[1].title() == "Online Song");
				QVERIFY(tracks[1].durationMs() == 286'000);
				QVERIFY(tracks[2].title() == "Local track");
				QVERIFY(tracks[2].durationMs() == 3'487'000);
			}
		}

		[[maybe_unused]] void
		testNonExistentTracksAreSkipped() // NOLINT(readability-convert-member-functions-to-static)
		{
			const auto samplePlaylist = createPlaylist(
				QList<PlaylistEntry> {
					{"/file1.mp3", "Track 1",            1000},
					{"/file2.mp3", "Non-existent track", 286},
					{"/file3.mp3", "Track 2",            3487}});

			for(const auto& enableTagReader: {true, false})
			{
				const auto fileSystem = std::make_shared<Test::FileSystemMock>(
					QMap<QString, QStringList> {
						{"/", {"playlist.pls", "file1.mp3", "file3.mp3"}}
					});

				fileSystem->writeFile(samplePlaylist.toLocal8Bit(), "/playlist.pls");

				auto parser = enableTagReader
				              ? PLSParser("/playlist.pls", fileSystem, std::make_shared<TagReaderMock>())
				              : PLSParser("/playlist.pls", fileSystem, nullptr);
				const auto tracks = parser.tracks();

				QVERIFY(tracks[0].filepath() == "/file1.mp3");
				QVERIFY(tracks[1].filepath() == "/file3.mp3");
			}
		}

		[[maybe_unused]] void testTagReaderOverridesTags() // NOLINT(readability-convert-member-functions-to-static)
		{
			const auto samplePlaylist = createPlaylist(
				QList<PlaylistEntry> {
					{"/file1.mp3", "Track 1", 1000},
					{"/file2.mp3", "Track 2", 3487}});

			const auto fileSystem = std::make_shared<Test::FileSystemMock>(
				QMap<QString, QStringList> {
					{"/", {"playlist.pls", "file1.mp3", "file2.mp3"}}
				});

			fileSystem->writeFile(samplePlaylist.toLocal8Bit(), "/playlist.pls");

			const auto genericTitle = QString {"Hallo"};
			const auto genericDuration = 12345;

			auto parser = PLSParser("/playlist.pls",
			                        fileSystem,
			                        std::make_shared<VerboseTagReaderMock>(genericTitle, genericDuration));

			const auto tracks = parser.tracks();

			QVERIFY(tracks[0].title() == genericTitle);
			QVERIFY(tracks[0].durationMs() == genericDuration);
			QVERIFY(tracks[1].title() == genericTitle);
			QVERIFY(tracks[1].durationMs() == genericDuration);
		}
};

QTEST_GUILESS_MAIN(PlsParserTest)

#include "PlsParserTest.moc"
