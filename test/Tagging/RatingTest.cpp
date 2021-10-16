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
