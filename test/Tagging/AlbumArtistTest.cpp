/* AlbumArtistTest.cpp
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

#include <QTest>
#include "Utils/Tagging/Tagging.h"
#include "Utils/FileUtils.h"
#include "Utils/MetaData/MetaData.h"
#include "AbstractTaggingTest.h"

class AlbumArtistTest :
	public AbstractTaggingTest
{
	Q_OBJECT

	public:
		AlbumArtistTest() :
			AbstractTaggingTest("AlbumArtistTest") {}

	private:
		void runTest(const QString& filename) override;

	private slots:
		void id3Test();
		void xiphTest();
};

void AlbumArtistTest::runTest(const QString& filename)
{
	const auto albumArtist = QString::fromUtf8("Motörhead фыва");
	auto metadata = MetaData(filename);
	auto metadataReloaded = metadata;
	Tagging::Utils::getMetaDataOfFile(metadata);

	metadata.setAlbumArtist(albumArtist);
	Tagging::Utils::setMetaDataOfFile(metadata);

	Tagging::Utils::getMetaDataOfFile(metadataReloaded);
	QVERIFY(metadataReloaded.albumArtist() == albumArtist);
}

void AlbumArtistTest::id3Test()
{
	AbstractTaggingTest::id3Test();
}

void AlbumArtistTest::xiphTest()
{
	AbstractTaggingTest::xiphTest();
}

QTEST_GUILESS_MAIN(AlbumArtistTest)

#include "AlbumArtistTest.moc"
