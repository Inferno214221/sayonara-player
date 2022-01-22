/* StandardPathTest.cpp */
/*
 * Copyright (C) 2011-2020 Michael Lugmair
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

#include <QDir>
#include <QFile>

// access working directory with Test::Base::tempPath("somefile.txt");

class StandardPathTest : 
    public Test::Base
{
    Q_OBJECT

    public:
        StandardPathTest() :
            Test::Base("StandardPathTest")
        {
	        Util::File::removeFilesInDirectory(QDir::home().absoluteFilePath(".qttest"));
        }

        ~StandardPathTest()
        {
			Util::File::removeFilesInDirectory(QDir::home().absoluteFilePath(".qttest"));
        }

    private slots:
        void test();
};

void StandardPathTest::test()
{
	const auto homePath = QDir(QDir::home().absoluteFilePath(".qttest"));

	QVERIFY(!QFile::exists(homePath.absoluteFilePath("share/sayonara")));
	QVERIFY(Util::xdgSharePath() == homePath.absoluteFilePath("share/sayonara"));
	QVERIFY(Util::xdgSharePath("file.txt") == homePath.absoluteFilePath("share/sayonara/file.txt"));
	QVERIFY(QFile::exists(Util::xdgSharePath()));

	QVERIFY(Util::xdgConfigPath() == homePath.absoluteFilePath("config/sayonara"));
	QVERIFY(Util::xdgConfigPath("file.txt") == homePath.absoluteFilePath("config/sayonara/file.txt"));
	QVERIFY(QFile::exists(Util::xdgConfigPath()));

	QVERIFY(Util::xdgCachePath() == homePath.absoluteFilePath("cache/sayonara"));
	QVERIFY(Util::xdgCachePath("file.txt") == homePath.absoluteFilePath("cache/sayonara/file.txt"));
	QVERIFY(QFile::exists(Util::xdgCachePath()));

	QVERIFY(Util::tempPath() == "/tmp/sayonara");
	QVERIFY(Util::tempPath("file.txt") == "/tmp/sayonara/file.txt");
	QVERIFY(QFile::exists(Util::tempPath()));

	QVERIFY(Util::similarArtistsPath() == homePath.absoluteFilePath("cache/sayonara/similar_artists"));
	QVERIFY(QFile::exists(Util::similarArtistsPath()));

	QVERIFY(Util::translationsPath() == homePath.absoluteFilePath("share/sayonara/translations"));
	QVERIFY(Util::translationsSharePath() == Util::File::cleanFilename(QString("%1/%2").arg(SAYONARA_INSTALL_SHARE_PATH).arg("translations")));
	QVERIFY(QFile::exists(Util::translationsPath()));

	QVERIFY(Util::coverDirectory() == homePath.absoluteFilePath("cache/sayonara/covers"));
	QVERIFY(Util::coverDirectory("file.txt") == homePath.absoluteFilePath("cache/sayonara/covers/file.txt"));
	QVERIFY(QFile::exists(Util::coverDirectory()));

	QVERIFY(Util::coverTempDirectory() == "/tmp/sayonara/covers");
	QVERIFY(Util::coverTempDirectory("file.txt") == "/tmp/sayonara/covers/file.txt");
	QVERIFY(QFile::exists(Util::coverTempDirectory()));

	QVERIFY(Util::lyricsPath() == homePath.absoluteFilePath("cache/sayonara/lyrics"));
	QVERIFY(Util::lyricsPath("file.txt") == homePath.absoluteFilePath("cache/sayonara/lyrics/file.txt"));
	QVERIFY(QFile::exists(Util::lyricsPath()));
}

QTEST_GUILESS_MAIN(StandardPathTest)
#include "StandardPathTest.moc"
