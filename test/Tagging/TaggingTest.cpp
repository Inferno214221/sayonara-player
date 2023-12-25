#include <utility>

#include "AbstractTaggingTest.h"
// access working directory with Test::Base::tempPath("somefile.txt");
#include "Utils/MetaData/MetaData.h"
#include "Utils/Tagging/Tagging.h"
#include "Utils/Set.h"
#include "Utils/MetaData/Genre.h"
#include "Utils/FileUtils.h"

namespace
{
	class TestEnv
	{
		public:
			TestEnv(const QString& resource, QString targetFile) :
				m_targetFile {std::move(targetFile)}
			{
				Util::File::deleteFiles({m_targetFile});

				auto content = QByteArray {};
				Util::File::readFileIntoByteArray(resource, content);
				const auto written = Util::File::writeFile(content, m_targetFile);
				QVERIFY(written);
			}

			~TestEnv()
			{
				Util::File::deleteFiles({m_targetFile});
			}

			[[nodiscard]] QString filename() const { return m_targetFile; }

		private:
			QString m_targetFile;
	};
}

class TaggingTest :
	public Test::Base
{
	Q_OBJECT

	public:
		TaggingTest() :
			Test::Base("TaggingTest") {}

	private slots:

		void testWriteAndReadBasicTags()
		{
			const auto testCases = std::array {
				TestEnv {":/test/mp3test.mp3", tempPath("sayonara-test.mp3")},
				TestEnv {":/test/oggtest.ogg", tempPath("sayonara-test.ogg")},
				TestEnv {":/test/emptyTestFile.mp3", tempPath("emptyTestFile.mp3")}
			};

			for(const auto& testCase: testCases)
			{
				auto track = MetaData(testCase.filename());
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

				auto trackNew = MetaData(testCase.filename());
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
		};
};

QTEST_GUILESS_MAIN(TaggingTest)

#include "TaggingTest.moc"
