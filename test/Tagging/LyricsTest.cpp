#include "SayonaraTest.h"

#include "Utils/Tagging/Tagging.h"
#include "Utils/Tagging/TaggingLyrics.h"
#include "Utils/FileUtils.h"
#include "Utils/MetaData/MetaData.h"
#include "AbstractTaggingTest.h"

class LyricsTest :
	public AbstractTaggingTest
{
	Q_OBJECT

	public:
		LyricsTest() :
			AbstractTaggingTest("LyricsTest") {}

		~LyricsTest() override = default;

	private:
		void runTest(const QString& filename) override;

	private slots:
		void id3Test();
		void xiphTest();
};

void LyricsTest::runTest(const QString& filename)
{
	const auto lyrics = QString::fromUtf8("Those are söme lyrics фыва");
	auto metadata = MetaData(filename);
	Tagging::Utils::getMetaDataOfFile(metadata);

	const auto wroteLyrics = Tagging::writeLyrics(metadata, lyrics);
	QVERIFY(wroteLyrics == true);

	QString readLyrics;
	const auto success = Tagging::extractLyrics(metadata, readLyrics);

	QVERIFY(success == true);
	QVERIFY(lyrics.compare(readLyrics) == 0);
}

void LyricsTest::id3Test()
{
	AbstractTaggingTest::id3Test();
}

void LyricsTest::xiphTest()
{
	AbstractTaggingTest::xiphTest();
}

QTEST_GUILESS_MAIN(LyricsTest)

#include "LyricsTest.moc"


