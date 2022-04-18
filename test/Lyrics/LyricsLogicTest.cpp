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

#include "Components/Lyrics/Lyrics.h"
#include "Utils/MetaData/MetaData.h"

#include <array>

// access working directory with Test::Base::tempPath("somefile.txt");

namespace
{
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
		auto track = MetaData{};

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
		void testStandardTrack();
		void testRadio();
};

void LyricsLogicTest::testStandardTrack()
{
	const auto artist = "Artist";
	const auto album = "Album";
	const auto albumArtist = "AlbumArtist";
	const auto title = "Title";
	const auto track = createStandardTrack(artist, album, albumArtist, title);

	auto lyrics = Lyrics::Lyrics();
	lyrics.setMetadata(track);

	QVERIFY(lyrics.artist() == artist);
	QVERIFY(lyrics.title() == title);
}

struct Result
{
	QString artist;
	QString title;
};

void LyricsLogicTest::testRadio()
{
	const auto stationName = "My Radio";
	const auto testCases = std::array {
		std::pair {"", Result {"", stationName}},
		std::pair {"Some Title", Result {"", "Some Title"}},
		std::pair {"Metallica - Battery", Result {"Metallica", "Battery"}},
		std::pair {"Metallica - Master of Puppets - Battery", Result {"Metallica", "Master of Puppets - Battery"}},
		std::pair {"Metallica: Battery", Result {"Metallica", "Battery"}},
		std::pair {"Metallica: Master of Puppets - Battery", Result {"Metallica", "Master of Puppets - Battery"}},
		std::pair {"Metallica - Master of Puppets: Battery", Result {"Metallica - Master of Puppets", "Battery"}},
	};

	for(const auto& testCase: testCases)
	{
		const auto& title = testCase.first;
		const auto&[expectedArtist, expectedTitle] = testCase.second;

		const auto track = createRadioTrack("https://path-to-url.mp3", stationName, title);

		auto lyrics = Lyrics::Lyrics();
		lyrics.setMetadata(track);

		QVERIFY(lyrics.artist() == expectedArtist);
		QVERIFY(lyrics.title() == expectedTitle);
	}
}

QTEST_GUILESS_MAIN(LyricsLogicTest)

#include "LyricsLogicTest.moc"
