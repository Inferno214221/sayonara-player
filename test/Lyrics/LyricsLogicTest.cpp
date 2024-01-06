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

#include "test/Common/SayonaraTest.h"
#include "test/Common/TaggingMocks.h"

#include "Components/Lyrics/Lyrics.h"
#include "Utils/MetaData/MetaData.h"

#include <array>

// access working directory with Test::Base::tempPath("somefile.txt");

namespace
{
	struct Result
	{
		QString artist;
		QString title;
	};

	class LyricsTagReaderWriter :
		public Test::TagReaderMock,
		public Test::TagWriterMock
	{
		public:
			void setLyrics(const QString& lyrics)
			{
				m_lyrics = lyrics;
			}

			[[nodiscard]] bool isLyricsSupported(const QString& /*filepath*/) const override
			{
				return !m_lyrics.isEmpty();
			}

			[[nodiscard]] std::optional<QString> extractLyrics(const MetaData& /*track*/) const override
			{
				return m_lyrics.isEmpty()
				       ? std::nullopt
				       : std::optional {m_lyrics};
			}

		private:
			QString m_lyrics;
	};

	MetaData
	createStandardTrack(const QString& artist, const QString& album, const QString& albumArtist, const QString& title)
	{
		MetaData track;
		track.setArtist(artist);
		track.setAlbum(album);
		track.setAlbumArtist(albumArtist);
		track.setTitle(title);

		return track;
	}

	MetaData createRadioTrack(const QString& url, const QString& stationName, const QString& title = QString())
	{
		auto track = MetaData {};

		track.setRadioStation(url, stationName);
		track.setFilepath(url, RadioMode::Station);
		if(!title.isEmpty())
		{
			track.setTitle(title);
		}

		return track;
	}
}

class LyricsLogicTest :
	public Test::Base
{
	Q_OBJECT

	public:
		LyricsLogicTest() :
			Test::Base("LyricsLogicTest") {}

	private slots:

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void testFetchesLyricsCorrectly()
		{
			struct TestCase
			{
				QString lyrics;
				bool expectedSupport;
			};

			const auto testCases = std::array {
				TestCase {{}, false},
				TestCase {{"lalalala"}, true},
			};

			for(const auto& testCase: testCases)
			{
				const auto tagAccessor = std::make_shared<LyricsTagReaderWriter>();
				tagAccessor->setLyrics(testCase.lyrics);

				auto lyrics = Lyrics::Lyrics(tagAccessor, tagAccessor);
				QVERIFY(lyrics.isLyricTagSupported() == testCase.expectedSupport);

				lyrics.setMetadata(MetaData {"/path/to/file.mp3"});
				QVERIFY(lyrics.lyrics().isEmpty());
				QVERIFY(lyrics.localLyrics() == testCase.lyrics);
			}
		}

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void testStandardTrack()
		{
			const auto artist = "Artist";
			const auto album = "Album";
			const auto albumArtist = "AlbumArtist";
			const auto title = "Title";
			const auto track = createStandardTrack(artist, album, albumArtist, title);

			const auto tagAccessor = std::make_shared<LyricsTagReaderWriter>();
			auto lyrics = Lyrics::Lyrics(tagAccessor, tagAccessor);
			lyrics.setMetadata(track);

			QVERIFY(lyrics.artist() == artist);
			QVERIFY(lyrics.title() == title);
		}

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void testRadio()
		{
			const auto stationName = "My Radio";
			const auto testCases = std::array {
				std::pair {"", Result {"", stationName}},
				std::pair {"Some Title", Result {"", "Some Title"}},
				std::pair {"Metallica - Battery", Result {"Metallica", "Battery"}},
				std::pair {"Metallica - Master of Puppets - Battery",
				           Result {"Metallica", "Master of Puppets - Battery"}},
				std::pair {"Metallica: Battery", Result {"Metallica", "Battery"}},
				std::pair {"Metallica: Master of Puppets - Battery",
				           Result {"Metallica", "Master of Puppets - Battery"}},
				std::pair {"Metallica - Master of Puppets: Battery",
				           Result {"Metallica - Master of Puppets", "Battery"}},
			};

			for(const auto& testCase: testCases)
			{
				const auto& title = testCase.first;
				const auto& [expectedArtist, expectedTitle] = testCase.second;

				const auto track = createRadioTrack("https://path-to-url.mp3", stationName, title);

				const auto tagAccessor = std::make_shared<LyricsTagReaderWriter>();
				auto lyrics = Lyrics::Lyrics(tagAccessor, tagAccessor);
				lyrics.setMetadata(track);

				QVERIFY(lyrics.artist() == expectedArtist);
				QVERIFY(lyrics.title() == expectedTitle);
			}
		}
};

QTEST_GUILESS_MAIN(LyricsLogicTest)

#include "LyricsLogicTest.moc"
