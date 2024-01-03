/* LyricsTest.cpp
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

#include "Utils/Tagging/Tagging.h"
#include "Utils/Tagging/TaggingLyrics.h"
#include "Utils/FileUtils.h"
#include "Utils/MetaData/MetaData.h"
#include "AbstractTaggingTest.h"

class LyricsTest :
	public AbstractTaggingTest
{
	Q_OBJECT

	public:
		LyricsTest() :
			AbstractTaggingTest("LyricsTest") {}

		~LyricsTest() override = default;

	private:
		void runTest(const QString& filename) override;

	private slots:
		void id3Test();
		void xiphTest();
};

void LyricsTest::runTest(const QString& filename)
{
	const auto lyrics = QString::fromUtf8("Those are söme lyrics фыва");
	auto metadata = MetaData(filename);
	Tagging::Utils::getMetaDataOfFile(metadata);

	const auto wroteLyrics = Tagging::writeLyrics(metadata, lyrics);
	QVERIFY(wroteLyrics == true);

	QString readLyrics;
	const auto success = Tagging::extractLyrics(metadata, readLyrics);

	QVERIFY(success == true);
	QVERIFY(lyrics.compare(readLyrics) == 0);
}

void LyricsTest::id3Test()
{
	AbstractTaggingTest::id3Test();
}

void LyricsTest::xiphTest()
{
	AbstractTaggingTest::xiphTest();
}

QTEST_GUILESS_MAIN(LyricsTest)

#include "LyricsTest.moc"


