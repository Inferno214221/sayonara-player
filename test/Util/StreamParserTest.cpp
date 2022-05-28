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

#include "Common/SayonaraTest.h"
#include "Common/TestWebClientFactory.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Parser/StreamParser.h"

#include <QSignalSpy>

// access working directory with Test::Base::tempPath("somefile.txt");

namespace
{
	constexpr const auto* stationName = "Station";
	constexpr const auto* stationUrl = "https://mySuperAwesomeRadio.de";
	constexpr const auto SpyTimeout = 50;
}

class StreamParserTest :
	public Test::Base
{
	Q_OBJECT

	public:
		StreamParserTest() :
			Test::Base("StreamParserTest") {}

	private slots:
		[[maybe_unused]] void testNoData();
		[[maybe_unused]] void testOneAudioFile();
		[[maybe_unused]] void testRelativeAudioFile();
		[[maybe_unused]] void testMultipleAudioFiles();
		[[maybe_unused]] void testPlaylistFile();
		[[maybe_unused]] void testStream();
};

[[maybe_unused]] void StreamParserTest::testNoData()
{
	auto testWebClientFactory = std::make_shared<TestWebClientFactory>();
	auto streamParser = StreamParser(testWebClientFactory, this);
	auto spy = QSignalSpy(&streamParser, &StreamParser::sigFinished);

	streamParser.parse(stationName, stationUrl);
	auto* client = testWebClientFactory->clients().first();
	client->fireData("Nothing", WebClient::Status::GotData);

	spy.wait(SpyTimeout);

	QVERIFY(spy.count() == 1);
	QVERIFY(streamParser.tracks().isEmpty());
}

[[maybe_unused]] void StreamParserTest::testOneAudioFile()
{
	auto testWebClientFactory = std::make_shared<TestWebClientFactory>();
	auto streamParser = StreamParser(testWebClientFactory, this);
	auto spy = QSignalSpy(&streamParser, &StreamParser::sigFinished);

	streamParser.parse(stationName, stationUrl);
	auto* client = testWebClientFactory->clients().first();
	const auto* data = R"(
<http>
    <a href="https://path/to/track.mp3">Hallo</a>
</http>)";

	client->fireData(data, WebClient::Status::GotData);

	spy.wait(SpyTimeout);

	QVERIFY(spy.count() == 1);
	QVERIFY(streamParser.tracks().size() == 1);
	const auto track = streamParser.tracks().first();

	QVERIFY(track.filepath() == "https://path/to/track.mp3");
	QVERIFY(track.radioMode() == RadioMode::Station);
	QVERIFY(track.radioStation() == stationUrl);
	QVERIFY(track.radioStationName() == stationName);
}

[[maybe_unused]] void StreamParserTest::testRelativeAudioFile()
{
	auto testWebClientFactory = std::make_shared<TestWebClientFactory>();
	auto streamParser = StreamParser(testWebClientFactory, this);
	auto spy = QSignalSpy(&streamParser, &StreamParser::sigFinished);

	streamParser.parse(stationName, stationUrl);
	auto* client = testWebClientFactory->clients().first();
	const auto* data = R"(
<http>
    <a href="/path/to/relativeTrack.mp3">Hallo</a>
</http>)";

	client->fireData(data, WebClient::Status::GotData);

	spy.wait(SpyTimeout);

	QVERIFY(streamParser.tracks().size() == 1);
	const auto track = streamParser.tracks().first();

	const auto expectedPath = QString("%1/path/to/relativeTrack.mp3").arg(stationUrl);
	QVERIFY(track.filepath().toLower() == expectedPath.toLower());
	QVERIFY(track.radioMode() == RadioMode::Station);
	QVERIFY(track.radioStation() == stationUrl);
	QVERIFY(track.radioStationName() == stationName);
}

[[maybe_unused]] void StreamParserTest::testMultipleAudioFiles()
{
	auto testWebClientFactory = std::make_shared<TestWebClientFactory>();
	auto streamParser = StreamParser(testWebClientFactory, this);
	auto spy = QSignalSpy(&streamParser, &StreamParser::sigFinished);

	streamParser.parse(stationName, stationUrl);

	auto* client = testWebClientFactory->clients().first();
	const auto* data = R"(
<http>
    <a href="https://path/to/track.mp3">Hallo</a>
    <a href="https://path/to/windows.wma">Hallo</a>
    <a href="https://path/to/vorbis.ogg">Hallo</a>
    <a href="https://path/to/invalid.xyz">Hallo</a> <!-- invalid -->
    <a href="https://path/to/track.mp3">Hallo</a> <!-- double entry -->
</http>)";
	client->fireData(data, WebClient::Status::GotData);

	spy.wait(SpyTimeout);

	QVERIFY(streamParser.tracks().size() == 3);
	QVERIFY(streamParser.tracks()[0].filepath() == "https://path/to/track.mp3");
	QVERIFY(streamParser.tracks()[1].filepath() == "https://path/to/windows.wma");
	QVERIFY(streamParser.tracks()[2].filepath() == "https://path/to/vorbis.ogg");
}

[[maybe_unused]] void StreamParserTest::testPlaylistFile()
{
	auto testWebClientFactory = std::make_shared<TestWebClientFactory>();
	auto streamParser = StreamParser(testWebClientFactory, this);
	auto spy = QSignalSpy(&streamParser, &StreamParser::sigFinished);

	streamParser.parse(stationName, stationUrl);

	auto* client = testWebClientFactory->clients().first();
	const auto* data = R"(
<http>
    <a href="https://path/to/playlist.m3u">Hallo</a>
</http>)";
	client->fireData(data, WebClient::Status::GotData);

	auto* playlistClient = testWebClientFactory->clientByUrl("https://path/to/playlist.m3u");
	const auto* playlistFile = R"(
https://path/to/file1.mp3
https://path/to/file2.mp3)";
	playlistClient->fireData(playlistFile, WebClient::Status::GotData);

	spy.wait(SpyTimeout);

	QVERIFY(streamParser.tracks().size() == 2);
	QVERIFY(streamParser.tracks()[0].filepath() == "https://path/to/file1.mp3");
	QVERIFY(streamParser.tracks()[1].filepath() == "https://path/to/file2.mp3");
}

[[maybe_unused]] void StreamParserTest::testStream()
{
	auto testWebClientFactory = std::make_shared<TestWebClientFactory>();
	auto streamParser = StreamParser(testWebClientFactory, this);
	auto spy = QSignalSpy(&streamParser, &StreamParser::sigFinished);

	streamParser.parse(stationName, stationUrl);
	auto* client = testWebClientFactory->clients().first();

	client->fireData("Audio DATA", WebClient::Status::AudioStream);

	spy.wait(SpyTimeout);

	QVERIFY(streamParser.tracks().size() == 1);
	QVERIFY(streamParser.tracks()[0].filepath() == "https://mySuperAwesomeRadio.de");
}

QTEST_GUILESS_MAIN(StreamParserTest)

#include "StreamParserTest.moc"
