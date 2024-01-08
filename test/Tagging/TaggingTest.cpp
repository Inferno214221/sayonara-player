/* TaggingTest.cpp
 *
 * Copyright (C) 2011-2024 Michael Lugmair
 *
 * This file is part of sayonara player
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <utility>

#include "AbstractTaggingTest.h"
// access working directory with Test::Base::tempPath("somefile.txt");
#include "Utils/MetaData/MetaData.h"
#include "Utils/Tagging/Tagging.h"
#include "Utils/Set.h"
#include "Utils/MetaData/Genre.h"
#include "Utils/FileUtils.h"
#include "Utils/Logger/Logger.h"

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

	MetaData createBasicTrack(const QString& filename)
	{
		auto track = MetaData(filename);
		track.setArtist("artist");
		track.setAlbum("album");
		track.setGenres(QStringList() << "genre1" << "genre2");
		track.setTitle("title");
		track.setComment("comment");
		track.setYear(1995);
		track.setTrackNumber(17);

		return track;
	}

	// only reason of existence for this function is to provide a common interface
	// with updateOnlyChanged
	void updateAll(const MetaData& /*oldtrack*/, const MetaData& newTrack)
	{
		Tagging::Utils::setMetaDataOfFile(newTrack);
	}

	void updateOnlyChanged(const MetaData& oldTrack, const MetaData& newTrack)
	{
		Tagging::Utils::setOnlyChangedMetaDataOfFile(oldTrack, newTrack);
	}

	bool findInFile(const QString& filename, const QByteArray& data)
	{
		QByteArray fileData;
		Util::File::readFileIntoByteArray(filename, fileData);
		return fileData.contains(data);
	}
}

class TaggingTest :
	public Test::Base
{
	Q_OBJECT

	public:
		TaggingTest() :
			Test::Base("TaggingTest") {}

	private slots:

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void testWriteAndReadBasicTags()
		{
			struct TestCase
			{
				TestEnv testEnv;
			};

			const auto testCases = std::array {
				TestCase {{":/test/mp3test.mp3", tempPath("sayonara-test.mp3")}},
				TestCase {{":/test/oggtest.ogg", tempPath("sayonara-test.ogg")}},
				TestCase {{":/test/wavtest.wav", tempPath("wavRiff.wav")}},
				TestCase {{":/test/emptyTestFile.mp3", tempPath("emptyTestFile.mp3")}},
				TestCase {{":/test/mp4test.mp4", tempPath("sayonara-test.mp4")}}
			};

			for(const auto& testCase: testCases)
			{
				const auto track = createBasicTrack(testCase.testEnv.filename());

				const auto success = Tagging::Utils::setMetaDataOfFile(track);
				QVERIFY(success);

				auto trackNew = MetaData(testCase.testEnv.filename());
				Tagging::Utils::getMetaDataOfFile(trackNew);
				QVERIFY(trackNew.artist() == "artist");
				QVERIFY(trackNew.album() == "album");
				QVERIFY(trackNew.genres().count() == 2);
				QVERIFY(trackNew.genres().contains(Genre("genre1")));
				QVERIFY(trackNew.genres().contains(Genre("genre2")));
				QVERIFY(trackNew.title() == "title");
				QVERIFY(trackNew.comment() == "comment");
				QVERIFY(trackNew.year() == 1995);
				QVERIFY(trackNew.trackNumber() == 17);
			}
		}

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void testWriteOnlyChangedTracks()
		{
			struct TestCase
			{
				TestEnv testEnv;
			};

			const auto testCases = std::array {
				TestCase {{":/test/mp3test.mp3", tempPath("sayonara-test.mp3")}},
				TestCase {{":/test/oggtest.ogg", tempPath("sayonara-test.ogg")}},
				TestCase {{":/test/wavtest.wav", tempPath("wavRiff.wav")}},
				TestCase {{":/test/emptyTestFile.mp3", tempPath("emptyTestFile.mp3")}},
				TestCase {{":/test/mp4test.mp4", tempPath("sayonara-test.mp4")}}
			};

			for(const auto setTagsFirst: {true, false})
			{
				for(const auto& testCase: testCases)
				{
					const auto track = setTagsFirst
					                   ? createBasicTrack(testCase.testEnv.filename())
					                   : MetaData {testCase.testEnv.filename()};
					const auto written = Tagging::Utils::setMetaDataOfFile(track);
					QVERIFY(written == true);

					auto updatedTrack = track;
					updatedTrack.setArtist("artist2");
					updatedTrack.setAlbum("album2");
					updatedTrack.setGenres(QStringList() << "genre3" << "genre4");
					updatedTrack.setTitle("title2");
					updatedTrack.setComment("comment2");
					updatedTrack.setYear(1996);
					updatedTrack.setTrackNumber(18);

					const auto writtenAgain = Tagging::Utils::setOnlyChangedMetaDataOfFile(track, updatedTrack);
					QVERIFY(writtenAgain);

					auto trackReloaded = MetaData(testCase.testEnv.filename());
					Tagging::Utils::getMetaDataOfFile(trackReloaded);
					QVERIFY(trackReloaded.artist() == "artist2");
					QVERIFY(trackReloaded.album() == "album2");
					QVERIFY(trackReloaded.genres().count() == 2);
					QVERIFY(trackReloaded.genres().contains(Genre("genre3")));
					QVERIFY(trackReloaded.genres().contains(Genre("genre4")));
					QVERIFY(trackReloaded.title() == "title2");
					QVERIFY(trackReloaded.comment() == "comment2");
					QVERIFY(trackReloaded.year() == 1996);
					QVERIFY(trackReloaded.trackNumber() == 18);
				}
			}
		}

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void testSelectiveFlacUpdate()
		{
			// https://gitlab.com/luciocarreras/sayonara-player/-/issues/321
			struct TestCase
			{
				QByteArray oldDateTag;
				std::function<void(MetaData, MetaData)> callUpdate;
				QByteArray expectedNewDateTag;
			};

			// exfalso tags are lower case
			const auto testCases = std::array {
				TestCase {"date=2024-01-01", updateAll, "DATE=2024"},
				TestCase {"date=2024-01-01", updateOnlyChanged, "DATE=2024-01-01"}
			};

			for(const auto& testCase: testCases)
			{
				const auto env = TestEnv {":/test/flactest.flac", tempPath("sayonara-test.flac")};

				auto track = MetaData {env.filename()};
				Tagging::Utils::getMetaDataOfFile(track);

				QVERIFY(findInFile(env.filename(), testCase.oldDateTag));

				auto newTrack = track;
				newTrack.setTitle("Some new title");

				testCase.callUpdate(track, newTrack);

				QVERIFY(findInFile(env.filename(), testCase.expectedNewDateTag));
				if(testCase.oldDateTag != testCase.expectedNewDateTag)
				{
					QVERIFY(!findInFile(env.filename(), testCase.oldDateTag));
				}
			}
		}
};

QTEST_GUILESS_MAIN(TaggingTest)

#include "TaggingTest.moc"
