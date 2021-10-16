#include <QTest>
#include "Utils/Tagging/Tagging.h"
#include "Utils/FileUtils.h"
#include "Utils/MetaData/MetaData.h"
#include "AbstractTaggingTest.h"

class AlbumArtistTest :
	public AbstractTaggingTest
{
	Q_OBJECT

	public:
		AlbumArtistTest() :
			AbstractTaggingTest("AlbumArtistTest") {}

	private:
		void runTest(const QString& filename) override;

	private slots:
		void id3Test();
		void xiphTest();
};

void AlbumArtistTest::runTest(const QString& filename)
{
	const auto albumArtist = QString::fromUtf8("Motörhead фыва");
	auto metadata = MetaData(filename);
	auto metadataReloaded = metadata;
	Tagging::Utils::getMetaDataOfFile(metadata);

	metadata.setAlbumArtist(albumArtist);
	Tagging::Utils::setMetaDataOfFile(metadata);

	Tagging::Utils::getMetaDataOfFile(metadataReloaded);
	QVERIFY(metadataReloaded.albumArtist() == albumArtist);
}

void AlbumArtistTest::id3Test()
{
	AbstractTaggingTest::id3Test();
}

void AlbumArtistTest::xiphTest()
{
	AbstractTaggingTest::xiphTest();
}

QTEST_GUILESS_MAIN(AlbumArtistTest)

#include "AlbumArtistTest.moc"
