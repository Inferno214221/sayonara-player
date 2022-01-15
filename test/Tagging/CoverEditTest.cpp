/*
 * Copyright (C) 2011-2021 Michael Lugmair
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

#include "SayonaraTest.h"
#include "Database/TestTracks.h"
#include "Components/Covers/CoverLocation.h"
#include "Components/Tagging/CoverEditor.h"
#include "Components/Tagging/Editor.h"
#include "Components/Covers/CoverChangeNotifier.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Utils.h"
#include "Utils/FileUtils.h"

#include <QPixmap>
#include <QSignalSpy>

// access working directory with Test::Base::tempPath("somefile.txt");

class CoverEditTest :
	public Test::Base
{
	Q_OBJECT

	public:
		CoverEditTest() :
			Test::Base("CoverEditTest") {}

	private:
		QPixmap mLogoCover = QPixmap(":/test/logo.png");

		void writeFiles(MetaDataList& tracks)
		{
			QByteArray content;
			Util::File::readFileIntoByteArray(":/test/mp3test.mp3", content);

			for(int i = 0; i < tracks.count(); i++)
			{
				const auto filename = QString("somefile%1.mp3").arg(i);
				const auto tempPath = Test::Base::tempPath(filename);
				tracks[i].setFilepath(tempPath);

				Util::File::writeFile(content, tempPath);
			}
		}

	private slots:
		void testInitialState();
		void testReplaceCurrentCover();
		void testReplaceCoverForAll();
		void testCommit();
		void testCommitWithoutUpdate();
};

void CoverEditTest::testInitialState()
{
	auto tagEditor = Tagging::Editor();
	auto coverEditor = Tagging::CoverEditor(&tagEditor, nullptr);
	auto tracks = Test::createTracks();
	tagEditor.setMetadata(tracks);

	QVERIFY(coverEditor.isCoverForAllAvailable() == false);
	QVERIFY(coverEditor.currentOriginalCover().isNull());
	QVERIFY(coverEditor.currentReplacementCover().isNull());
	QVERIFY(!coverEditor.currentCoverLocation().isValid());
	QVERIFY(coverEditor.count() == tracks.count());
}

void CoverEditTest::testReplaceCurrentCover()
{
	auto tagEditor = Tagging::Editor();
	auto coverEditor = Tagging::CoverEditor(&tagEditor, nullptr);
	auto tracks = Test::createTracks();
	tagEditor.setMetadata(tracks);

	for(auto i = 0; i < coverEditor.count(); i += 2)
	{
		coverEditor.setCurrentIndex(i);
		coverEditor.replaceCurrentCover(mLogoCover);
	}

	for(auto i = 0; i < coverEditor.count(); i++)
	{
		coverEditor.setCurrentIndex(i);

		const auto replacement = coverEditor.currentReplacementCover();
		const auto isEven = (i % 2 == 0);
		QVERIFY(replacement.isNull() == !isEven);
	}

	for(auto i = 0; i < coverEditor.count(); i++)
	{
		coverEditor.setCurrentIndex(i);
		coverEditor.replaceCurrentCover(QPixmap());

		const auto replacement = coverEditor.currentReplacementCover();
		QVERIFY(replacement.isNull());
	}
}

void CoverEditTest::testReplaceCoverForAll()
{
	auto tagEditor = Tagging::Editor();
	auto coverEditor = Tagging::CoverEditor(&tagEditor, nullptr);
	auto tracks = Test::createTracks();
	tagEditor.setMetadata(tracks);

	const auto starCover = QPixmap(":/test/star.png");
	const auto coverKey = Util::calcHash(Util::convertPixmapToByteArray(mLogoCover));
	const auto starKey = Util::calcHash(Util::convertPixmapToByteArray(starCover));

	for(auto i = 0; i < coverEditor.count(); i++)
	{
		coverEditor.setCurrentIndex(i);
		coverEditor.replaceCurrentCover(mLogoCover);
	}

	coverEditor.replaceCoverForAll(starCover);
	QVERIFY(coverEditor.isCoverForAllAvailable());

	for(auto i = 0; i < coverEditor.count(); i++)
	{
		const auto replacement = coverEditor.currentReplacementCover();
		const auto key = Util::calcHash(Util::convertPixmapToByteArray(replacement));

		QVERIFY(key == starKey);
	}

	coverEditor.replaceCoverForAll(QPixmap());
	QVERIFY(!coverEditor.isCoverForAllAvailable());

	for(auto i = 0; i < coverEditor.count(); i++)
	{
		const auto replacement = coverEditor.currentReplacementCover();
		const auto key = Util::calcHash(Util::convertPixmapToByteArray(replacement));

		QVERIFY(key == coverKey);
	}
}

void CoverEditTest::testCommit()
{
	auto tagEditor = Tagging::Editor();
	auto coverEditor = Tagging::CoverEditor(&tagEditor, nullptr);
	auto tracks = Test::createTracks();
	writeFiles(tracks);
	tagEditor.setMetadata(tracks);

	for(auto i = 0; i < coverEditor.count(); i++)
	{
		coverEditor.setCurrentIndex(i);
		coverEditor.replaceCurrentCover(mLogoCover);
		coverEditor.updateTrack(i);
	}

	auto spy = QSignalSpy(Cover::ChangeNotfier::instance(), &Cover::ChangeNotfier::sigCoversChanged);
	{
		tagEditor.commit();
	}
	QCOMPARE(spy.count(), 1);
}

void CoverEditTest::testCommitWithoutUpdate()
{
	auto tagEditor = Tagging::Editor();
	auto coverEditor = Tagging::CoverEditor(&tagEditor, nullptr);
	auto tracks = Test::createTracks();
	writeFiles(tracks);
	tagEditor.setMetadata(tracks);

	for(auto i = 0; i < coverEditor.count(); i++)
	{
		coverEditor.setCurrentIndex(i);
		coverEditor.replaceCurrentCover(mLogoCover);
	}

	auto spy = QSignalSpy(Cover::ChangeNotfier::instance(), &Cover::ChangeNotfier::sigCoversChanged);
	{
		tagEditor.commit();
	}
	QCOMPARE(spy.count(), 0);
}

QTEST_MAIN(CoverEditTest)

#include "CoverEditTest.moc"
