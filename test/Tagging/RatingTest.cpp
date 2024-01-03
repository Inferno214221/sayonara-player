/* RatingTest.cpp
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
#include "AbstractTaggingTest.h"
#include "Utils/Tagging/Tagging.h"
#include "Utils/FileUtils.h"
#include "Utils/MetaData/MetaData.h"

class RatingTest :
	public AbstractTaggingTest
{
	Q_OBJECT

	public:
		RatingTest() :
			AbstractTaggingTest("RatingTest") {}

		~RatingTest() override = default;

	private:
		void runTest(const QString& filename) override;

	private slots:
		void id3Test();
		void xiphTest();
};

void RatingTest::runTest(const QString& filename)
{
	auto metadata = MetaData(filename);
	auto metadataReloaded = MetaData(filename);

	Tagging::Utils::getMetaDataOfFile(metadata);

	metadata.setRating(Rating::Three);
	Tagging::Utils::setMetaDataOfFile(metadata);
	QVERIFY(metadata.rating() == Rating::Three);

	Tagging::Utils::getMetaDataOfFile(metadataReloaded);
	QVERIFY(metadataReloaded.rating() == Rating::Three);

	metadata.setRating(Rating::Zero);
	Tagging::Utils::setMetaDataOfFile(metadata);
	QVERIFY(metadata.rating() == Rating::Zero);

	Tagging::Utils::getMetaDataOfFile(metadataReloaded);
	QVERIFY(metadataReloaded.rating() == Rating::Zero);
}

void RatingTest::id3Test()
{
	AbstractTaggingTest::id3Test();
}

void RatingTest::xiphTest()
{
	AbstractTaggingTest::xiphTest();
}

QTEST_GUILESS_MAIN(RatingTest)

#include "RatingTest.moc"
