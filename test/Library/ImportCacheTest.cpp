/* ImportCacheTest.cpp
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

#include "Components/Library/Importer/ImportCache.h"
#include "Utils/Tagging/TagReader.h"

class ImportCacheTest :
	public Test::Base
{
	Q_OBJECT

	public:
		ImportCacheTest() :
			Test::Base("ImportCacheTest") {}

	private slots:
		void test();
};

void ImportCacheTest::test()
{
	const QString libraryPath("/path/to/my/library");
	const QStringList paths
		{
			"/some/path/to/be/imported/cover.jpg",
			"/some/path/to/be/imported/subfolder/playlist.pls"
		};

	const auto tagReader = Tagging::TagReader::create();
	Library::ImportCache cache1(libraryPath, tagReader);
	Library::ImportCache cache2(libraryPath, tagReader);
	Library::ImportCache cache3(libraryPath, tagReader);

	for(const QString& path: paths)
	{
		cache1.addFile(path, "/some/path/to/be/imported");
	}

	for(const QString& path: paths)
	{
		cache2.addFile(path, "/some/path/to");
	}

	const QString& cache3Path("/files/from/somewhere/else");
	cache3.addFile(cache3Path, "/files/from/somewhere/else");

	QString p11 = cache1.targetFilename(paths[0], "");
	QVERIFY(p11 == "/path/to/my/library/cover.jpg");

	QString p12 = cache1.targetFilename(paths[1], "");
	QVERIFY(p12 == "/path/to/my/library/subfolder/playlist.pls");

	QString p21 = cache2.targetFilename(paths[0], "deeper");
	QVERIFY(p21 == "/path/to/my/library/deeper/be/imported/cover.jpg");

	QString p22 = cache2.targetFilename(paths[1], "deeper");
	QVERIFY(p22 == "/path/to/my/library/deeper/be/imported/subfolder/playlist.pls");

	QString p3 = cache3.targetFilename(cache3Path, "");
	QVERIFY(p3 == "/path/to/my/library");
}

QTEST_GUILESS_MAIN(ImportCacheTest)

#include "ImportCacheTest.moc"
