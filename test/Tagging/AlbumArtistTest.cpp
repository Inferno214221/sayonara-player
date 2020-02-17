#include <QTest>
#include "Utils/Tagging/Tagging.h"
#include "Utils/FileUtils.h"
#include "Utils/MetaData/MetaData.h"
#include "AbstractTaggingTest.h"

class AlbumArtistTest : public AbstractTaggingTest
{
	Q_OBJECT

public:
	AlbumArtistTest() :
		AbstractTaggingTest("AlbumArtistTest")
	{}

private:
	void run_test(const QString& filename) override;

private slots:
	void id3_test();
	void xiph_test();
};


void AlbumArtistTest::run_test(const QString& filename)
{
	QString album_artist = QString::fromUtf8("Motörhead фыва");
	MetaData md(filename);
    Tagging::Utils::getMetaDataOfFile(md);

	md.setAlbumArtist(album_artist);
    Tagging::Utils::setMetaDataOfFile(md);

	MetaData md2(filename);
    Tagging::Utils::getMetaDataOfFile(md2);

	QString md_album_artist = md.albumArtist();
	QString md2_album_artist = md2.albumArtist();

	QVERIFY(md_album_artist.compare(album_artist) == 0);
	QVERIFY(md_album_artist.compare(md2_album_artist) == 0);
}

void AlbumArtistTest::id3_test()
{
	AbstractTaggingTest::id3_test();
}

void AlbumArtistTest::xiph_test()
{
	AbstractTaggingTest::xiph_test();
}

QTEST_GUILESS_MAIN(AlbumArtistTest)

#include "AlbumArtistTest.moc"
