#include "SayonaraTest.h"
// access working directory with Test::Base::tempPath("somefile.txt");

#include "Gui/Utils/MimeData/CustomMimeData.h"

#include "Utils/FileUtils.h"
#include "Utils/MetaData/MetaData.h"
#include "Utils/MetaData/MetaDataList.h"

#include <QUrl>
#include <QList>

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
