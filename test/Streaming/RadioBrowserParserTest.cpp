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
#include "Components/Streaming/StationSearcher/RadioBrowserSearcher.h"
#include "Components/Streaming/StationSearcher/RadioStation.h"

#include "Utils/FileUtils.h"

#include <QList>

class RadioBrowserParserTest :
	public Test::Base
{
	Q_OBJECT

	public:
		RadioBrowserParserTest() :
			Test::Base("RadioBrowserParserTest") {}

	private slots:

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void testParsing()
		{
			auto data = QByteArray {};
			Util::File::readFileIntoByteArray(":/test/radio-browser.info.json", data);

			const auto parser = RadioBrowserParser();
			const auto radioStations = parser.parse(data);

			QVERIFY(radioStations[0].name == "Kings Radio Davao");
			QVERIFY(radioStations[0].home_url == "https://www.amfmph.com/");
			QVERIFY(radioStations[0].location == "The Philippines");
			QVERIFY(radioStations[0].image ==
			        "https://play-lh.googleusercontent.com/NRdZhS26PUEXfVgnR6ww1AdYJa2AQLL4KF8jSOjFsR2R4oii9Gi1s28sXRP2i9CacKlu=w480-h960-rw");
			QVERIFY(radioStations[0].short_description == "cebuano,filipino");
			QVERIFY(radioStations[0].streams[0].index == 0);
			QVERIFY(radioStations[0].streams[0].url == "http://us1.amfmph.com:8708/live");
			QVERIFY(radioStations[0].streams[0].bitrate == "64");
			QVERIFY(radioStations[0].streams[0].type == "AAC+, 64 kBit/s");

			QVERIFY(radioStations[1].name == "KLAS Sports Radio 89.5 Kingston");
			QVERIFY(radioStations[1].home_url == "http://www.klassportsradio.com/");
			QVERIFY(radioStations[1].location == "Kingston, Jamaica");
			QVERIFY(radioStations[1].image.isEmpty());
			QVERIFY(radioStations[1].short_description == "english");
			QVERIFY(radioStations[1].streams[0].index == 0);
			QVERIFY(radioStations[1].streams[0].url == "http://ice.audionow.com/6283KLASSportsRadioopus.ogg");
			QVERIFY(radioStations[1].streams[0].bitrate == "0");
			QVERIFY(radioStations[1].streams[0].type == "OGG");

			QVERIFY(radioStations.count() == 2);
		}
};

QTEST_GUILESS_MAIN(RadioBrowserParserTest)

#include "RadioBrowserParserTest.moc"
