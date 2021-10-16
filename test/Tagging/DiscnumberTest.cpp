#include <QTest>
#include "AbstractTaggingTest.h"
#include "Utils/Tagging/Tagging.h"
#include "Utils/FileUtils.h"
#include "Utils/MetaData/MetaData.h"

class DiscnumberTest :
	public AbstractTaggingTest
{
	Q_OBJECT

	public:
		DiscnumberTest() :
			AbstractTaggingTest("DiscnumberTest") {}

	private:
		void runTest(const QString& filename) override;

	private slots:
		void id3Test();
		void xiphTest();
};

void DiscnumberTest::runTest(const QString& filename)
{
	const auto albumArtist = QString::fromUtf8("Motörhead фыва");
	auto metadata = MetaData(filename);
	auto metadataReloaded = metadata;

	Tagging::Utils::getMetaDataOfFile(metadata);
	QVERIFY(metadata.discnumber() == 5);

	metadata.setDiscnumber(1);
	metadata.setDiscCount(2);
	Tagging::Utils::setMetaDataOfFile(metadata);
	QVERIFY(metadata.discnumber() == 1);
	QVERIFY(metadata.discCount() == 2);

	Tagging::Utils::getMetaDataOfFile(metadataReloaded);
	qDebug() << "Expect 1, get " << metadataReloaded.discnumber();
	QVERIFY(metadataReloaded.discnumber() == 1);

	qDebug() << "Expect 2, get " << metadataReloaded.discCount();
	QVERIFY(metadataReloaded.discCount() == 2);

	metadata.setDiscnumber(8);
	metadata.setDiscCount(9);
	Tagging::Utils::setMetaDataOfFile(metadata);
	QVERIFY(metadata.discnumber() == 8);
	QVERIFY(metadata.discCount() == 9);

	Tagging::Utils::getMetaDataOfFile(metadataReloaded);
	qDebug() << "Expect 8, get " << metadataReloaded.discnumber();
	QVERIFY(metadataReloaded.discnumber() == 8);

	qDebug() << "Expect 9, get " << metadataReloaded.discCount();
	QVERIFY(metadataReloaded.discCount() == 9);

	metadata.setDiscnumber(10);
	metadata.setDiscCount(12);
	Tagging::Utils::setMetaDataOfFile(metadata);

	Tagging::Utils::getMetaDataOfFile(metadataReloaded);

	qDebug() << "Expect 10, get " << metadataReloaded.discnumber();
	QVERIFY(metadataReloaded.discnumber() == 10);

	qDebug() << "Expect 12, get " << metadataReloaded.discCount();
	QVERIFY(metadataReloaded.discCount() == 12);
}

void DiscnumberTest::id3Test()
{
	AbstractTaggingTest::id3Test();
}

void DiscnumberTest::xiphTest()
{
	AbstractTaggingTest::xiphTest();
}

QTEST_GUILESS_MAIN(DiscnumberTest)

#include "DiscnumberTest.moc"
