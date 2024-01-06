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

#include "Common/SayonaraTest.h"

#include "Utils/Tagging/Tagging.h"
#include "Utils/Tagging/TaggingLyrics.h"
#include "Utils/FileUtils.h"
#include "Utils/MetaData/MetaData.h"

namespace
{
	QString prepareFile(const QString& sourceFile, const QString& targetDir)
	{
		auto filename = QString {};
		Util::File::copyFile(sourceFile, targetDir, filename);
		QFile(filename).setPermissions(filename, static_cast<QFileDevice::Permission>(0x7777));

		return filename;
	}
}

class LyricsTest :
	public Test::Base
{
	Q_OBJECT

	public:
		LyricsTest() :
			Test::Base("LyricsTest") {}

		~LyricsTest() override = default;

	private slots:

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void testWriteAndReadLyrics()
		{
			const auto testCases = QStringList {
				":/test/mp3test.mp3",
				":/test/oggtest.ogg",
				":/test/mp4test.mp4"
			};

			for(const auto& testCase: testCases)
			{
				const auto filename = prepareFile(testCase, tempPath());

				const auto lyrics = QString::fromUtf8("Those are söme lyrics фыва");
				auto metadata = MetaData(filename);
				Tagging::Utils::getMetaDataOfFile(metadata);

				const auto wroteLyrics = Tagging::writeLyrics(filename, lyrics);
				QVERIFY(wroteLyrics == true);

				QString readLyrics;
				const auto success = Tagging::extractLyrics(filename, readLyrics);

				QVERIFY(success == true);
				QVERIFY(lyrics.compare(readLyrics) == 0);

				Util::File::deleteFiles({filename});
			}
		}
};

QTEST_GUILESS_MAIN(LyricsTest)

#include "LyricsTest.moc"
