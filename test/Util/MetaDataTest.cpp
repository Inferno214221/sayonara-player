/* MetaDataTest.cpp
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

#include "Utils/MetaData/MetaData.h"
#include "Utils/MetaData/Genre.h"
#include "Utils/Set.h"

#include <algorithm>
#include <array>
#include <utility>

namespace
{
	MetaData createTrack()
	{
		auto track = MetaData("/path/to/my/file.mp3");
		track.setTitle("Title");
		track.setArtist("Artist");
		track.setAlbum("Album");
		track.setDurationMs(100000); // NOLINT(readability-magic-numbers)
		track.setFilesize(1234567); // NOLINT(readability-magic-numbers)
		track.setId(5); // NOLINT(readability-magic-numbers)
		track.setArtistId(6); // NOLINT(readability-magic-numbers)
		track.setAlbumId(7); // NOLINT(readability-magic-numbers)
		track.setBitrate(320000); // NOLINT(readability-magic-numbers)
		track.setTrackNumber(17); // NOLINT(readability-magic-numbers)
		track.setYear(2014); // NOLINT(readability-magic-numbers)
		track.setExtern(true);
		track.setDisabled(true);
		track.setRating(Rating::Four);
		track.setDiscnumber(2);
		track.setDiscCount(5); // NOLINT(readability-magic-numbers)
		track.setLibraryid(2);
		track.setDisabled(true);

		track.addGenre(Genre("Metal"));
		track.addGenre(Genre("Rock"));
		track.setAlbumArtist("Album artist", 14); // NOLINT(readability-magic-numbers)

		return track;
	}
}

class MetaDataTest :
	public Test::Base
{
	Q_OBJECT

	public:
		MetaDataTest() :
			Test::Base("MetaDataTest") {}

		~MetaDataTest() override = default;

	private slots:
		[[maybe_unused]] void copyTest();
		[[maybe_unused]] void genreTest();
		[[maybe_unused]] void privateTest();
		[[maybe_unused]] void streamTest();
		[[maybe_unused]] void moveTest();
		[[maybe_unused]] void setRadioStationTest();
		[[maybe_unused]] void testRadioModeConversion();

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void testFilepathHash()
		{
			struct TestCase
			{
				QString filepath1;
				QString filepath2;
				bool expectEqual {false};
			};

			const auto testCases = std::array {
				TestCase {"/path/to/a", "/path/to/b", false},
				TestCase {"/path/to/a", "/path/to/a", true},
				TestCase {"/path/to/a", "/path/to/./a", true},
				TestCase {"/path/to/a", "/path/to/../to/a", true}
			};

			for(const auto& testCase: testCases)
			{
				const auto track1 = MetaData {testCase.filepath1};
				const auto track2 = MetaData {testCase.filepath2};
				const auto isEqual = track1.filepathHash() == track2.filepathHash();
				
				QVERIFY(isEqual == testCase.expectEqual);
				QVERIFY(track1.isEqual(track2) == testCase.expectEqual);
			}
		}
};

[[maybe_unused]] void MetaDataTest::copyTest() // NOLINT(readability-convert-member-functions-to-static)
{
	const auto track = createTrack();

	const auto track2 = track; // NOLINT(performance-unnecessary-copy-initialization)
	QVERIFY(track2.isEqual(createTrack()));
	QVERIFY(track2.isEqualDeep(createTrack()));
	QVERIFY(track2.uniqueId() != track.uniqueId());

	const MetaData track3(track); // NOLINT(performance-unnecessary-copy-initialization)
	QVERIFY(track3.isEqual(createTrack()));
	QVERIFY(track3.isEqualDeep(createTrack()));
	QVERIFY(track3.uniqueId() != track.uniqueId());
}

[[maybe_unused]] void MetaDataTest::moveTest() // NOLINT(readability-convert-member-functions-to-static)
{
	const auto originalTrack = createTrack();

	auto track = originalTrack;
	auto uniqueId = track.uniqueId();

	MetaData track2(std::move(track));
	QVERIFY(track2.isEqualDeep(originalTrack));
	QVERIFY(track2.uniqueId() == uniqueId);

	track = originalTrack;
	uniqueId = track.uniqueId();

	const auto track3 = std::move(track);
	QVERIFY(track3.isEqualDeep(originalTrack));
	QVERIFY(track3.uniqueId() == uniqueId);

	track = track3; // test reassignment
	QVERIFY(track.isEqualDeep(originalTrack));
	QVERIFY(track.uniqueId() != uniqueId);
}

[[maybe_unused]] void MetaDataTest::genreTest() // NOLINT(readability-convert-member-functions-to-static)
{
	const auto track = createTrack();
	QVERIFY(track.hasGenre(Genre("Metal")));
	QVERIFY(track.hasGenre(Genre("Rock")));
}

[[maybe_unused]] void MetaDataTest::privateTest() // NOLINT(readability-convert-member-functions-to-static)
{
	const auto track = createTrack();
	QVERIFY(track.albumArtistId() == 14);
	QVERIFY(track.albumArtist().compare("Album artist") == 0);
}

[[maybe_unused]] void MetaDataTest::streamTest() // NOLINT(readability-convert-member-functions-to-static)
{
	auto track = createTrack();
	QVERIFY(track.radioMode() == RadioMode::Off);

	track.setFilepath("http://path.to/my/stream");
	QVERIFY(track.radioMode() == RadioMode::Station);
}

struct Result
{
	QString station;
	QString stationName;
};

[[maybe_unused]] void
MetaDataTest::setRadioStationTest() // NOLINT(readability-convert-member-functions-to-static,readability-function-cognitive-complexity)
{
	const auto testCases = std::array {
		std::tuple {"", "", Result {"", ""}},
		std::tuple {"http://path.to/stream", "", Result {"http://path.to/stream", "path.to"}},
		std::tuple {"http://path.to/stream", "Name", Result {"http://path.to/stream", "Name"}},
		std::tuple {"path.to/stream", "", Result {"path.to/stream", "path.to/stream"}},
		std::tuple {"path.to/stream", "Name", Result {"path.to/stream", "Name"}}
	};

	for(const auto& testCase: testCases)
	{
		const auto& [url, stationName, result] = testCase;
		const auto& [expectedStation, expectedStationName] = result;

		auto track = MetaData {};
		track.setRadioStation(url, stationName);

		const auto station = track.radioStation();
		const auto radioStationName = track.radioStationName();

		QVERIFY(track.radioStation() == expectedStation);
		QVERIFY(track.radioStationName() == expectedStationName);

		QVERIFY(track.artist() == track.radioStation());
		QVERIFY(track.title() == track.radioStationName());
		QVERIFY(track.album() == track.title());
	}
}

void MetaDataTest::testRadioModeConversion()
{
	constexpr const auto radioModes = std::array {
		RadioMode::Off,
		RadioMode::Podcast,
		RadioMode::Soundcloud,
		RadioMode::Station,
		RadioMode::Undefined
	};

	for(const auto& radioMode: radioModes)
	{
		const auto variant = QVariant::fromValue(radioMode);
		QVERIFY(variant.value<RadioMode>() == radioMode);
	}
}

QTEST_GUILESS_MAIN(MetaDataTest)

#include "MetaDataTest.moc"
