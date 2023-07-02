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
#include "Utils/Parser/PodcastParser.h"
#include "Utils/MetaData/MetaDataList.h"

namespace
{
	/* URLs are noted as "https:/ /bla.mp3" because Qt's MOC compiler has problems with a correct URI scheme and skips the file
	 * Probably because of confusing double slash with comments
	 */

	const QString PodcastData = R"rss(<?xml version="1.0" encoding="UTF-8" ?>
<?xml-stylesheet href="https:/ /feeds.buzzsprout.com/styles.xsl" type="text/xsl"?>
<rss version="2.0" xmlns:itunes="http:/ /www.itunes.com/dtds/podcast-1.0.dtd" xmlns:podcast="https:/ /podcastindex.org/namespace/1.0" xmlns:content="http:/ /purl.org/rss/1.0/modules/content/" xmlns:atom="http:/ /www.w3.org/2005/Atom">
<channel>
  <title>Global Title</title>
  <description>Global description</description>
  <image>
     <url>https:/ /global-standard-image.jpg</url>
  </image>
  <itunes:image href="https:/ /global-itunes-image.jpg" />
  <item>
    <itunes:author>iTunes Artist</itunes:author>
    <title>Standard Title 1</title>
    <description>Local description 1</description>
    <enclosure url="https:/ /path-to-item-1.mp3" length="53921918" type="audio/mpeg" />
    <pubDate>Sat, 01 Jul 2023 00:00:00 +0200</pubDate>
    <itunes:duration>4483</itunes:duration>
  </item>
  <item>
    <itunes:title>iTunes Title 2</itunes:title>
    <itunes:image href="https:/ /local-itunes-image-1.jpg" />
    <itunes:author>iTunes Artist</itunes:author>
    <title>Standard Title 2</title>
    <description>Local description 2</description>
    <enclosure url="https:/ /path-to-item-2.mp3" length="51706509" type="audio/mpeg" />
    <pubDate>Thu, 01 Jun 2020 00:00:00 +0200</pubDate>
    <itunes:duration>4301</itunes:duration>
  </item>
</channel>
</rss>)rss";
}

class PodcastParserTest :
	public Test::Base
{
	Q_OBJECT

	public:
		PodcastParserTest() :
			Test::Base("PodcastParserTest") {}

	private slots:

		void testParsing() // NOLINT(readability-convert-member-functions-to-static)
		{
			const auto tracks = PodcastParser::parsePodcastXmlFile(PodcastData);

			QVERIFY(tracks.count() == 2);
			{
				const auto& track = tracks[0];
				QVERIFY(track.title() == "Standard Title 1");
				QVERIFY(track.artist() == "iTunes Artist");
				QVERIFY(track.filepath() == "https:/ /path-to-item-1.mp3");
				// QVERIFY(track.year() == 2023);
				QVERIFY(track.durationMs() == 4'483'000);
				QVERIFY(track.coverDownloadUrls().contains("https:/ /global-itunes-image.jpg"));
				QVERIFY(track.coverDownloadUrls().contains("https:/ /global-standard-image.jpg"));
				QVERIFY(track.radioMode() == RadioMode::Podcast);
			}

			{
				const auto& track = tracks[1];
				QVERIFY(track.title() == "iTunes Title 2");
				QVERIFY(track.artist() == "iTunes Artist");
				QVERIFY(track.filepath() == "https:/ /path-to-item-2.mp3");
				// QVERIFY(track.year() == 2020);
				QVERIFY(track.durationMs() == 4'301'000);
				// not ready for one cover per track
				QVERIFY(!track.coverDownloadUrls().contains("https:/ /local-itunes-image-1.jpg"));
				QVERIFY(track.coverDownloadUrls().contains("https:/ /global-itunes-image.jpg"));
				QVERIFY(track.coverDownloadUrls().contains("https:/ /global-standard-image.jpg"));
				QVERIFY(track.radioMode() == RadioMode::Podcast);
			}
		}
};

QTEST_GUILESS_MAIN(PodcastParserTest)

#include "PodcastParserTest.moc"

