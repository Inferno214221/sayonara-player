/* StandardPathTest.cpp */
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

#include "Utils/StandardPaths.h"
#include "Utils/Macros.h"
#include "Utils/FileUtils.h"
#include "Utils/Utils.h"

#include <QDir>
#include <QFile>
#include <QStandardPaths>

// access working directory with Test::Base::tempPath("somefile.txt");

class StandardPathTest :
	public Test::Base
{
	Q_OBJECT

	public:
		StandardPathTest() :
			Test::Base("StandardPathTest") {}

		QDir qtestPath() { return tempPath(".qttest"); }

	private slots:

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void testHomePath()
		{
			const auto standardHomes = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
			QVERIFY(standardHomes.count() == 1);
			QVERIFY(standardHomes[0] == tempPath());

			const auto dirHome = QDir::homePath();
			QVERIFY(dirHome == tempPath());
		}

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void testSharePath()
		{
			QVERIFY(Util::xdgSharePath() == qtestPath().absoluteFilePath("share/sayonara"));
			QVERIFY(Util::xdgSharePath("file.txt") == qtestPath().absoluteFilePath("share/sayonara/file.txt"));
			QVERIFY(QFile::exists(Util::xdgSharePath()));
		}

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void testXdgCachePath()
		{
			QVERIFY(Util::xdgCachePath() == qtestPath().absoluteFilePath("cache/sayonara"));
			QVERIFY(Util::xdgCachePath("file.txt") == qtestPath().absoluteFilePath("cache/sayonara/file.txt"));
			QVERIFY(QFile::exists(Util::xdgCachePath()));
		}

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void testXdgConfigPath()
		{
			QVERIFY(Util::xdgConfigPath() == qtestPath().absoluteFilePath("config/sayonara"));
			QVERIFY(Util::xdgConfigPath("file.txt") == qtestPath().absoluteFilePath("config/sayonara/file.txt"));
			QVERIFY(QFile::exists(Util::xdgConfigPath()));
		}

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void testTempPath()
		{
			QVERIFY(Util::tempPath() == "/tmp/sayonara");
			QVERIFY(Util::tempPath("file.txt") == "/tmp/sayonara/file.txt");
			QVERIFY(QFile::exists(Util::tempPath()));
		}

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void testSimilartArtistsPath()
		{
			QVERIFY(Util::similarArtistsPath() == qtestPath().absoluteFilePath("cache/sayonara/similar_artists"));
			QVERIFY(QFile::exists(Util::similarArtistsPath()));
		}

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void testTranslationPaths()
		{
			QVERIFY(Util::translationsPath() == qtestPath().absoluteFilePath("share/sayonara/translations"));
			QVERIFY(Util::translationsSharePath() ==
			        Util::File::cleanFilename(QString("%1/%2").arg(SAYONARA_INSTALL_SHARE_PATH).arg("translations")));
			QVERIFY(QFile::exists(Util::translationsPath()));
		}

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void testCoverDirectories()
		{
			QVERIFY(Util::coverDirectory() == qtestPath().absoluteFilePath("cache/sayonara/covers"));
			QVERIFY(Util::coverDirectory("file.txt") == qtestPath().absoluteFilePath("cache/sayonara/covers/file.txt"));
			QVERIFY(QFile::exists(Util::coverDirectory()));

			QVERIFY(Util::coverTempDirectory() == "/tmp/sayonara/covers");
			QVERIFY(Util::coverTempDirectory("file.txt") == "/tmp/sayonara/covers/file.txt");
			QVERIFY(QFile::exists(Util::coverTempDirectory()));
		}

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void testLyricsPath()
		{
			QVERIFY(Util::lyricsPath() == qtestPath().absoluteFilePath("cache/sayonara/lyrics"));
			QVERIFY(Util::lyricsPath("file.txt") == qtestPath().absoluteFilePath("cache/sayonara/lyrics/file.txt"));
			QVERIFY(QFile::exists(Util::lyricsPath()));
		}
};

QTEST_GUILESS_MAIN(StandardPathTest)

#include "StandardPathTest.moc"
