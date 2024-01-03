/* MimeDataTest.cpp
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

#include "Gui/Utils/MimeData/CustomMimeData.h"
#include "Utils/FileUtils.h"
#include "Utils/MetaData/MetaData.h"
#include "Utils/MetaData/MetaDataList.h"

#include <QUrl>
#include <QList>

// access working directory with Test::Base::tempPath("somefile.txt");

class MimeDataTest :
	public Test::Base
{
	Q_OBJECT

	public:
		MimeDataTest() :
			Test::Base("MimeDataTest")
		{
			Util::File::createDir(Test::Base::tempPath("directory1"));
			Util::File::createDir(Test::Base::tempPath("directory2"));
		}

	private slots:
		void testMetadata();
		void testEmptyMetadata();
		void testDirsNonUrl();
		void testDirsUrl();
		void testDragSource();
};

void MimeDataTest::testMetadata()
{
	auto mimeData = Gui::CustomMimeData(this);

	MetaDataList tracks;
	for(int i = 0; i < 10; i++)
	{
		auto md = MetaData(Test::Base::tempPath(QString("filename%1.mp3").arg(i)));
		tracks << md;
	}

	mimeData.setMetadata(tracks);

	QVERIFY(mimeData.hasMetadata());
	QVERIFY(mimeData.metadata().size() == 10);

	QVERIFY(mimeData.hasUrls());
	QVERIFY(mimeData.urls().size() == 10);

	const auto urls = mimeData.urls();
	for(const auto& url : urls)
	{
		QVERIFY(Util::File::isUrl(url.toString()));
	}
}

void MimeDataTest::testEmptyMetadata()
{
	auto mimeData = Gui::CustomMimeData(this);

	const auto url = QUrl("file:///path/to/somewhere.mp3");
	mimeData.setUrls({url});

	MetaDataList tracks;
	mimeData.setMetadata(tracks);

	QVERIFY(!mimeData.hasMetadata());
	QVERIFY(mimeData.metadata().size() == 0);

	QVERIFY(mimeData.hasUrls());
	QVERIFY(mimeData.urls().size() == 1);
}

void MimeDataTest::testDirsNonUrl()
{
	auto mimeData = Gui::CustomMimeData(this);

	const auto urls = QList<QUrl> {
		QUrl(Test::Base::tempPath("directory1")),
		QUrl(Test::Base::tempPath("directory2"))
	};

	mimeData.setUrls(urls);

	QVERIFY(mimeData.hasUrls());
	QVERIFY(mimeData.urls().size() == 2);
	QVERIFY(mimeData.hasMetadata() == false);

	const auto fetchedUrls = mimeData.urls();
	for(const auto& url : fetchedUrls)
	{
		QVERIFY(!Util::File::isUrl(url.toString()));
	}
}

void MimeDataTest::testDirsUrl()
{
	auto mimeData = Gui::CustomMimeData(this);

	auto urls = QList<QUrl> {
		QUrl(Test::Base::tempPath("directory1")),
		QUrl(Test::Base::tempPath("directory2"))
	};

	for(auto& url : urls)
	{
		url.setScheme("file");
	}

	mimeData.setUrls(urls);

	QVERIFY(mimeData.hasUrls());
	QVERIFY(mimeData.urls().size() == 2);
	QVERIFY(mimeData.hasMetadata() == false);

	const auto fetchedUrls = mimeData.urls();
	for(const auto& url : fetchedUrls)
	{
		QVERIFY(Util::File::isUrl(url.toString()));
	}
}

void MimeDataTest::testDragSource()
{
	{
		auto mimeData = Gui::CustomMimeData(this);
		QVERIFY(mimeData.hasDragSource(this));
	}

	{
		auto mimeData = Gui::CustomMimeData();
		QVERIFY(!mimeData.hasDragSource(this));
	}
}

QTEST_GUILESS_MAIN(MimeDataTest)

#include "MimeDataTest.moc"
