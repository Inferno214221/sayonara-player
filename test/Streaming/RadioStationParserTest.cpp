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

#include "Components/Streaming/StationSearcher/FMStreamParser.h"
#include "Utils/FileUtils.h"

#include <QList>

// access working directory with Test::Base::tempPath("somefile.txt");

namespace
{
	constexpr const auto HtmlFile = ":/test/fmstream.org.html";

	QByteArray readHtmlFile()
	{
		auto data = QByteArray {};
		Util::File::readFileIntoByteArray(HtmlFile, data);

		return data;
	}
}

class RadioStationParserTest :
	public Test::Base
{
	Q_OBJECT

	public:
		RadioStationParserTest() :
			Test::Base("RadioStationParserTest") {}

	private slots:
		void test();
};

void RadioStationParserTest::test()
{
	const auto data = readHtmlFile();
	const auto parser = FMStreamParser();
	const auto stations = parser.parse(data);

	QVERIFY(stations.count() == 4);

	{
		const auto station = stations[0];
		QVERIFY(station.name == "Metal FM Com");
		QVERIFY(station.location == "Wuppertal - Deutschland, Germany");
		QVERIFY(station.style == "Metal");
		QVERIFY(station.home_url == "https://metal-fm.com");
		QVERIFY(station.description.startsWith("Metal-FM. com, gehört zu den Top 3 der deutschen Metal Radios und "));
		QVERIFY(station.description.endsWith("den Chat 4 Metalheads (://chat4metalheads. de) abgegeben"));
		QVERIFY(station.short_description == "Heavy Metal, Power Metal, Thrash Metal, Death Metal um.");
		QVERIFY(station.streams.count() == 1);
		QVERIFY(station.streams[0].bitrate == "128");
		QVERIFY(station.streams[0].type == "mp3");
		QVERIFY(station.streams[0].url == "http://stream.laut.fm/metal-fm-com");
	}

	{
		const auto station = stations[1];
		QVERIFY(station.name == "Radio - Thrash Metal");
		QVERIFY(station.location == "Vinnytsya, Ukraine");
		QVERIFY(station.style.isEmpty());
		QVERIFY(station.home_url.isEmpty());
		QVERIFY(station.description.isEmpty());
		QVERIFY(station.short_description.isEmpty());
		QVERIFY(station.streams.count() == 1);
		QVERIFY(station.streams[0].bitrate == "80");
		QVERIFY(station.streams[0].type == "vor");
		QVERIFY(station.streams[0].url == "http://music.myradio.ua/Trash_Metal128.ogg");
	}

	{
		const auto station = stations[2];
		QVERIFY(station.name == "Radio Caprice - Thrash Metal");
		QVERIFY(station.location == "Moscow, Russia");
		QVERIFY(station.style == "Rock Heavy-Metal");
		QVERIFY(station.home_url == "http://radcap.ru");
		QVERIFY(station.description == "Rock Heavy-Metal");
		QVERIFY(station.short_description == "Rock Heavy-Metal");
		QVERIFY(station.streams.count() == 3);
		QVERIFY(station.streams[0].bitrate == "128");
		QVERIFY(station.streams[0].type == "2");
		QVERIFY(station.streams[0].url == "http://79.120.77.11:8000/thrashmetal");
		QVERIFY(station.streams[1].bitrate == "128");
		QVERIFY(station.streams[1].type == "2");
		QVERIFY(station.streams[1].url == "http://79.120.77.11:9101/");
		QVERIFY(station.streams[2].bitrate == "48");
		QVERIFY(station.streams[2].type == "2");
		QVERIFY(station.streams[2].url == "http://79.120.77.11:8002/thrashmetal");
	}

	{
		const auto station = stations[3];
		QVERIFY(station.name == "UR 1 Persha Prog");
		QVERIFY(station.location == "Kiev, UKR, Ukraine");
		QVERIFY(station.style == "News-Talk");
		QVERIFY(station.frequency == "DAB 11D");
		QVERIFY(station.home_url == "https://radioukr.com.ua");
		QVERIFY(station.description == QString::fromUtf8("Перший канал українського радіо Ukraine News World"));
		QVERIFY(station.short_description == "Pershyy kanal Ukrayins'koho radio");
		QVERIFY(station.streams.count() == 4);
		QVERIFY(station.streams[0].bitrate == "256");
		QVERIFY(station.streams[0].type == "mp3");
		QVERIFY(station.streams[0].url == "http://radio.nrcu.gov.ua:8000/ur1-mp3");
		QVERIFY(station.streams[1].bitrate == "128");
		QVERIFY(station.streams[1].type == "mp3");
		QVERIFY(station.streams[1].url == "http://radio.nrcu.gov.ua:8000/ur1-mp3-m");
		QVERIFY(station.streams[2].bitrate.isEmpty());
		QVERIFY(station.streams[2].type.isEmpty());
		QVERIFY(station.streams[2].url == "http://nrcu.gov.ua/en/schedule/play-live.html?channelID=1");
		QVERIFY(station.streams[3].bitrate.isEmpty());
		QVERIFY(station.streams[3].type.isEmpty());
		QVERIFY(station.streams[3].url == "http://nrcu.gov.ua/uk/schedule/play-live.html?channelID=1");
	}
}

QTEST_GUILESS_MAIN(RadioStationParserTest)

#include "RadioStationParserTest.moc"
