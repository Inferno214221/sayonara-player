#include "SayonaraTest.h"

#include "Utils/Tagging/Tagging.h"
#include "Utils/Tagging/TaggingLyrics.h"
#include "Utils/FileUtils.h"
#include "Utils/MetaData/MetaData.h"
#include "AbstractTaggingTest.h"

class LyricsTest : public AbstractTaggingTest
{
	Q_OBJECT

public:
	LyricsTest() :
		AbstractTaggingTest("LyricsTest")
	{}

	~LyricsTest() override = default;

private:
	void run_test(const QString& filename) override;

private slots:
	void id3_test();
	void xiph_test();
};

void LyricsTest::run_test(const QString& filename)
{

	const QString lyrics = QString::fromUtf8("Those are söme lyrics фыва");
	MetaData md(filename);
	Tagging::Utils::getMetaDataOfFile(md);

	bool wroteLyrics = Tagging::writeLyrics(md, lyrics);
	QVERIFY(wroteLyrics == true);

	QString readLyrics;
	bool success = Tagging::extractLyrics(md, readLyrics);

	QVERIFY(success == true);
	QVERIFY(lyrics.compare(readLyrics) == 0);
}

void LyricsTest::id3_test()
{
	AbstractTaggingTest::id3_test();
}

void LyricsTest::xiph_test()
{
	AbstractTaggingTest::xiph_test();
}

QTEST_GUILESS_MAIN(LyricsTest)

#include "LyricsTest.moc"


