#include <QTest>
#include "AbstractTaggingTest.h"
#include "Utils/Tagging/Tagging.h"
#include "Utils/FileUtils.h"
#include "Utils/MetaData/MetaData.h"

class RatingTest : public AbstractTaggingTest
{
	Q_OBJECT

public:
	RatingTest() :
		AbstractTaggingTest("RatingTest")
	{}

	~RatingTest() override = default;

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

	Tagging::Utils::getMetaDataOfFile(md);

	md.setRating(Rating::Three);
	Tagging::Utils::setMetaDataOfFile(md);
	QVERIFY(md.rating() == Rating::Three);

	Tagging::Utils::getMetaDataOfFile(md2);
	qDebug() << "Expect 3, get " << static_cast<int>(md2.rating());
	QVERIFY(md2.rating() == Rating::Three);

	md.setRating(Rating::Zero);
	Tagging::Utils::setMetaDataOfFile(md);
	QVERIFY(md.rating() == Rating::Zero);

	Tagging::Utils::getMetaDataOfFile(md2);
	qDebug() << "Expect 0, get " << static_cast<int>(md2.rating());
	QVERIFY(md2.rating() == Rating::Zero);
}

void RatingTest::id3_test()
{
	AbstractTaggingTest::id3_test();
}

void RatingTest::xiph_test()
{
	AbstractTaggingTest::xiph_test();
}

QTEST_GUILESS_MAIN(RatingTest)

#include "RatingTest.moc"
