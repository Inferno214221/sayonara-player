#include <QTest>
#include "AbstractTaggingTest.h"
#include "Utils/Tagging/Tagging.h"
#include "Utils/FileUtils.h"
#include "Utils/MetaData/MetaData.h"


class DiscnumberTest : public AbstractTaggingTest
{
	Q_OBJECT

private:
	void run_test(const QString& filename) override;

private slots:
	void id3_test();
	void xiph_test();
};


void DiscnumberTest::run_test(const QString& filename)
{
	QString album_artist = QString::fromUtf8("Motörhead фыва");
	MetaData md(filename);
	MetaData md2(filename);

	Tagging::Util::getMetaDataOfFile(md);
	QVERIFY(md.discnumber == 5);

	md.discnumber = 1;
	md.n_discs = 2;
	Tagging::Util::setMetaDataOfFile(md);
	QVERIFY(md.discnumber == 1);
	QVERIFY(md.n_discs == 2);

	Tagging::Util::getMetaDataOfFile(md2);
	qDebug() << "Expect 1, get " << md2.discnumber;
	QVERIFY(md2.discnumber == 1);

	qDebug() << "Expect 2, get " << md2.n_discs;
	QVERIFY(md2.n_discs == 2);


	md.discnumber = 8;
	md.n_discs = 9;
	Tagging::Util::setMetaDataOfFile(md);
	QVERIFY(md.discnumber == 8);
	QVERIFY(md.n_discs == 9);

	Tagging::Util::getMetaDataOfFile(md2);
	qDebug() << "Expect 8, get " << md2.discnumber;
	QVERIFY(md2.discnumber == 8);

	qDebug() << "Expect 9, get " << md2.n_discs;
	QVERIFY(md2.n_discs == 9);

	md.discnumber = 10;
	md.n_discs = 12;
	Tagging::Util::setMetaDataOfFile(md);

	Tagging::Util::getMetaDataOfFile(md2);

	qDebug() << "Expect 10, get " << md2.discnumber;
	QVERIFY(md2.discnumber == 10);

	qDebug() << "Expect 12, get " << md2.n_discs;
	QVERIFY(md2.n_discs == 12);
}

void DiscnumberTest::id3_test()
{
	AbstractTaggingTest::id3_test();
}

void DiscnumberTest::xiph_test()
{
	AbstractTaggingTest::xiph_test();
}

QTEST_MAIN(DiscnumberTest)

#include "DiscnumberTest.moc"
