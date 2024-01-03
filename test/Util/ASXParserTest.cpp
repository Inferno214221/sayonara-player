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
#include "Common/FileSystemMock.h"
#include "Common/TaggingMocks.h"

#include "Utils/Parser/ASXParser.h"
#include "Utils/MetaData/MetaDataList.h"

// access working directory with Test::Base::tempPath("somefile.txt");

class AsxParserTest :
	public Test::Base
{
	Q_OBJECT

	public:
		AsxParserTest() :
			Test::Base("ASXParserTest") {}

	private slots:
		[[maybe_unused]] void testParserPlaylist();
};

// I can't create this function inline because the moc parser crashes when reading "R(...)" literals, I can't even
// move it to the anon namespace QTBUG-62632
[[maybe_unused]] void AsxParserTest::testParserPlaylist() // NOLINT(readability-convert-member-functions-to-static)
{
	constexpr const auto* SamplePlaylist = R"asx(<asx version="3.0">
  <title>Sample presentation</title>
  <author>Tobias Kuenkel</author>
  <entry>
    <title>Sample clip no 1</title>
    <author>Peter Meier</author>
    <ref href="https://server.de/pfad/datei1.wmv"/>
  </entry>
  <entry>
    <title>Sample clip no 2</title>
    <ref href="https://server.de/pfad/datei2.wmv"/>
  </entry>
</asx>)asx";

	struct TestCase
	{
		bool withTagReader {false};
	};

	const auto testCases = {
		TestCase {true},
		TestCase {false}
	};

	for(const auto& testCase: testCases)
	{
		auto fileSystem = std::make_shared<Test::FileSystemMock>(
			QMap<QString, QStringList> {
				{"/", {"playlist.asx"}}
			});

		fileSystem->writeFile(SamplePlaylist, "/playlist.asx");

		auto parser = testCase.withTagReader
		              ? ASXParser("/playlist.asx", fileSystem, std::make_shared<Test::TagReaderMock>())
		              : ASXParser("/playlist.asx", fileSystem, nullptr);

		const auto tracks = parser.tracks();

		QVERIFY(tracks[0].title() == "Sample clip no 1");
		QVERIFY(tracks[0].artist() == "Peter Meier");
		QVERIFY(tracks[0].albumArtist() == "Tobias Kuenkel");
		QVERIFY(tracks[0].filepath() == "https://server.de/pfad/datei1.wmv");

		QVERIFY(tracks[1].title() == "Sample clip no 2");
		QVERIFY(tracks[1].artist() == "Tobias Kuenkel");
		QVERIFY(tracks[1].albumArtist() == tracks[1].artist());
		QVERIFY(tracks[1].filepath() == "https://server.de/pfad/datei2.wmv");
	}
}

QTEST_GUILESS_MAIN(AsxParserTest)

#include "ASXParserTest.moc"
