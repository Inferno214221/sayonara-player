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

#include "Components/DynamicPlayback/LfmSimiliarArtistsParser.h"

#include "Utils/Algorithm.h"
#include "Utils/FileUtils.h"

// access working directory with Test::Base::tempPath("somefile.txt");

namespace
{
	QByteArray getSimilarArtistData()
	{
		constexpr const auto* filename = ":/test/metallica.similar_artists";
		auto data = QByteArray {};
		Util::File::readFileIntoByteArray(filename, data);

		return data;
	}

	QByteArray createDoc(const QString& artistName, const QList<DynamicPlayback::ArtistMatch::Entry>& entries)
	{
		auto entryStrings = QStringList {};
		Util::Algorithm::transform(entries, entryStrings, [](const auto& entry) {
			return QString("<artist><name>%1</name><mbid>%2</mbid><match>%3</match></artist>")
				.arg(entry.artist)
				.arg(entry.mbid)
				.arg(entry.similarity);
		});

		return QString("<lfm><similarartists artist=\"%1\">%2</similarartists></lfm>")
			.arg(artistName)
			.arg(entryStrings.join("\n")).toLocal8Bit();
	}
}

class LfmSimilarArtistParserTest :
	public Test::Base
{
	Q_OBJECT

	public:
		LfmSimilarArtistParserTest() :
			Test::Base("LfmSimilarArtistParserTest") {}

	private slots:

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void testResultError()
		{
			struct TestCase
			{
				QByteArray data;
				bool expectedSuccess;
			};

			const auto testCases = std::array {
				TestCase {getSimilarArtistData(), true},
				TestCase {"some kind of monster", false}
			};

			for(const auto& testCase: testCases)
			{
				const auto result = DynamicPlayback::parseLastFMAnswer(testCase.data);
				QCOMPARE(result.hasError, !testCase.expectedSuccess);
			}
		}

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void testValidity()
		{
			struct TestCase
			{
				QByteArray data;
				bool expectedValid;
			};

			const auto testCases = std::array {
				TestCase {{}, false},
				TestCase {createDoc("Doors", {}), false},
				TestCase {getSimilarArtistData(), true}
			};

			for(const auto& testCase: testCases)
			{
				const auto result = DynamicPlayback::parseLastFMAnswer(testCase.data);
				const auto match = result.artistMatch;

				QCOMPARE(match.isValid(), testCase.expectedValid);
			}
		}

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void testData()
		{
			using Quality = DynamicPlayback::ArtistMatch::Quality;
			using Entry = DynamicPlayback::ArtistMatch::Entry;

			constexpr const auto* artistName = "Metallica";
			const auto excellentEntries = QList<Entry> {
				{"Megadeth", "mbid-megadeth", 0.99},
				{"Slayer",   "mbid-slayer",   0.9},
				{"Pantera",  "mbid-pantera",  0.7},
			};

			const auto veryGoodEntries = QList<Entry> {
				{"Pantera",     "mbid-pantera",     0.7},
				{"Iron Maiden", "mbid-iron-maiden", 0.59}
			};

			const auto data = createDoc(artistName, excellentEntries + veryGoodEntries);
			const auto result = DynamicPlayback::parseLastFMAnswer(data);
			const auto match = result.artistMatch;

			const auto qualities = QList {
				std::pair {Quality::VeryGood, std::reference_wrapper(veryGoodEntries)},
				std::pair {Quality::Excellent, std::reference_wrapper(excellentEntries)}
			};

			for(const auto& [quality, entries]: qualities)
			{
				const auto parsedEntries = match.get(quality);
				for(const auto& parsedEntry: parsedEntries)
				{
					const auto contains = Util::Algorithm::contains(entries.get(), [&](const auto& entry) {
						return (parsedEntry == entry);
					});

					QVERIFY(contains);
				}
			}
		}
};

QTEST_GUILESS_MAIN(LfmSimilarArtistParserTest)

#include "LfmSimilarArtistParserTest.moc"
