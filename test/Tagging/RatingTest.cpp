#include <QTest>
#include "AbstractTaggingTest.h"
#include "Utils/Tagging/Tagging.h"
#include "Utils/FileUtils.h"
#include "Utils/MetaData/MetaData.h"


class RatingTest : public AbstractTaggingTest
{
	Q_OBJECT

private:
	void run_test(const QString& filename) override;

private slots:
	void id3_test();
	void xiph_test();
};


void RatingTest::run_test(const QString& filename)
{
	MetaData md(filename);
	MetaData md2(filename);

	Tagging::Util::getMetaDataOfFile(md);

	md.rating = 3;
	Tagging::Util::setMetaDataOfFile(md);
	QVERIFY(md.rating == 3);

	Tagging::Util::getMetaDataOfFile(md2);
	qDebug() << "Expect 3, get " << md2.rating;
	QVERIFY(md2.rating == 3);

	md.rating = 0;
	Tagging::Util::setMetaDataOfFile(md);
	QVERIFY(md.rating == 0);

	Tagging::Util::getMetaDataOfFile(md2);
	qDebug() << "Expect 0, get " << md2.rating;
	QVERIFY(md2.rating == 0);
}

void RatingTest::id3_test()
{
	AbstractTaggingTest::id3_test();
}

void RatingTest::xiph_test()
{
	AbstractTaggingTest::xiph_test();
}

QTEST_MAIN(RatingTest)

#include "RatingTest.moc"
