#include "AbstractTaggingTest.h"
// access working directory with Test::Base::tempPath("somefile.txt");
#include "Utils/MetaData/MetaData.h"
#include "Utils/Tagging/Tagging.h"
#include "Utils/Set.h"
#include "Utils/MetaData/Genre.h"
#include "Utils/FileUtils.h"

class TaggingTest : 
    public AbstractTaggingTest
{
    Q_OBJECT

    public:
        TaggingTest() :
            AbstractTaggingTest("TaggingTest")
        {}

    private slots:
        void id3Test() { AbstractTaggingTest::id3_test(); };
		void xiphTest() { AbstractTaggingTest::xiph_test(); };
		void emptyFileTest();

	protected:
		void run_test(const QString& filename) override;
};

void TaggingTest::run_test(const QString& f)
{
	const auto filename = Util::File::getFilenameOfPath(f);
	const auto filepath = tempPath(filename);
	auto track = MetaData(filepath);
	track.setArtist("artist");
	track.setAlbum("album");
	track.setDiscCount(4);
	track.setDiscnumber(3);
	track.setAlbumArtist("albumartist");
	track.setGenres(QStringList() << "genre1" << "genre2");
	track.setTitle("title");
	track.setComment("comment");
	track.setRating(Rating(Rating::Four));
	track.setYear(1995);
	track.setTrackNumber(17);

	Tagging::Utils::setMetaDataOfFile(track);

	auto trackNew = MetaData(filepath);
	Tagging::Utils::getMetaDataOfFile(trackNew);
	QVERIFY(trackNew.artist() == "artist");
	QVERIFY(trackNew.album() == "album");
	QVERIFY(trackNew.discCount() == 4);
	QVERIFY(trackNew.discnumber() == 3);
	QVERIFY(trackNew.albumArtist() == "albumartist");
	QVERIFY(trackNew.genres().count() == 2);
	QVERIFY(trackNew.genres().contains(Genre("genre1")));
	QVERIFY(trackNew.genres().contains(Genre("genre2")));
	QVERIFY(trackNew.title() == "title");
	QVERIFY(trackNew.comment() == "comment");
	QVERIFY(trackNew.rating() == Rating::Four);
	QVERIFY(trackNew.year() == 1995);
	QVERIFY(trackNew.trackNumber() == 17);
}

void TaggingTest::emptyFileTest()
{
	const auto filepath = tempPath("emptyFile.mp3");
	const auto audioData = QByteArray("Some audio data");
	Util::File::writeFile(audioData, filepath);

	auto track = MetaData(filepath);
	track.setArtist("artist");
	track.setAlbum("album");
	track.setDiscCount(4);
	track.setDiscnumber(3);
	track.setAlbumArtist("albumartist");
	track.setGenres(QStringList() << "genre1" << "genre2");
	track.setTitle("title");
	track.setComment("comment");
	track.setRating(Rating(Rating::Four));
	track.setYear(1995);
	track.setTrackNumber(17);

	const auto success = Tagging::Utils::setMetaDataOfFile(track);
	QVERIFY(success);

	auto trackNew = MetaData(filepath);
	Tagging::Utils::getMetaDataOfFile(trackNew);
	QVERIFY(trackNew.artist() == "artist");
	QVERIFY(trackNew.album() == "album");
	QVERIFY(trackNew.discCount() == 4);
	QVERIFY(trackNew.discnumber() == 3);
	QVERIFY(trackNew.albumArtist() == "albumartist");
	QVERIFY(trackNew.genres().count() == 2);
	QVERIFY(trackNew.genres().contains(Genre("genre1")));
	QVERIFY(trackNew.genres().contains(Genre("genre2")));
	QVERIFY(trackNew.title() == "title");
	QVERIFY(trackNew.comment() == "comment");
	QVERIFY(trackNew.rating() == Rating::Four);
	QVERIFY(trackNew.year() == 1995);
	QVERIFY(trackNew.trackNumber() == 17);
}

QTEST_GUILESS_MAIN(TaggingTest)
#include "TaggingTest.moc"
