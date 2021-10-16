#include "AbstractTaggingTest.h"
// access working directory with Test::Base::tempPath("somefile.txt");

#include "Utils/Tagging/TaggingCover.h"

#include <QByteArray>
#include <QPixmap>
#include <QString>

class CoverTest :
	public AbstractTaggingTest
{
	Q_OBJECT

	public:
		CoverTest() :
			AbstractTaggingTest("CoverTest") {}

	private:
		void runTest(const QString& filename) override;

		QPixmap logo() const
		{
			return QPixmap(":/test/logo.png");
		}

		QPixmap logoBig() const
		{
			return QPixmap(":/test/logo.png").scaled(1000, 1000);
		}

	private slots:
		void id3Test();
		void xiphTest();
};

void CoverTest::runTest(const QString& filename)
{
	QVERIFY(Tagging::hasCover(filename) == false);
	QVERIFY(Tagging::isCoverSupported(filename) == true);

	const auto originalFilesize = QFileInfo(filename).size();

	{
		const auto isNull = Tagging::extractCover(filename).isNull();
		QVERIFY2(isNull, "test no cover extraction");
	}

	{
		QByteArray coverData;
		QString mimeType;
		const auto success = Tagging::extractCover(filename, coverData, mimeType);
		QVERIFY2(!success, "test no cocver extraction data");
		QVERIFY2(coverData.isEmpty(), "test empty cover data");
		QVERIFY2(mimeType.isEmpty(), "test empty cover mime type");
	}

	{
		auto success = Tagging::writeCover(filename, logoBig());
		QVERIFY2(success, "test write from pixmap");
		QVERIFY2(Tagging::hasCover(filename), "has cover 1");
		QVERIFY2(!Tagging::extractCover(filename).isNull(), "check cover 1");
	}

	const auto bigFilesize = QFileInfo(filename).size();
	QVERIFY2((bigFilesize - originalFilesize) > 0, "Check if filesize is bigger");

	{
		const auto success = Tagging::writeCover(filename, ":/test/logo.png");
		QVERIFY2(success, "test write from file");
		QVERIFY2(Tagging::hasCover(filename), "has cover 2");
		QVERIFY2(!Tagging::extractCover(filename).isNull(), "test successful extraction 2");
	}

	const auto smallerFilesize = QFileInfo(filename).size();
	QVERIFY2((smallerFilesize > originalFilesize) && (smallerFilesize < bigFilesize),
	         "Check if filesize is in between");

	{
		QByteArray coverData;
		QString mimeType;
		const auto success = Tagging::extractCover(filename, coverData, mimeType);
		QVERIFY2(success, "test data extraction after writing");
		QVERIFY2(!coverData.isEmpty(), "test cover data not empty");
		QVERIFY2(!mimeType.isEmpty(), "test mime type not empty");
	}
}

void CoverTest::id3Test()
{
	AbstractTaggingTest::id3Test();
}

void CoverTest::xiphTest()
{
	AbstractTaggingTest::xiphTest();
}

QTEST_MAIN(CoverTest)

#include "CoverTest.moc"
