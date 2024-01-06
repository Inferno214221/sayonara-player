/* RatingTest.cpp
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

#include "Common/SayonaraTest.h"

#include "Utils/FileUtils.h"
#include "Utils/MetaData/MetaData.h"
#include "Utils/Tagging/MP4/PopularimeterFrame.h"
#include "Utils/Tagging/Tagging.h"

#include <taglib/fileref.h>

namespace
{
	QString prepareFile(const QString& sourceFile, const QString& targetDir)
	{
		auto filename = QString {};
		Util::File::copyFile(sourceFile, targetDir, filename);
		QFile(filename).setPermissions(filename, static_cast<QFileDevice::Permission>(0x7777));

		return filename;
	}

	template<typename FrameType>
	void writeFile(const QString& filename, const Rating rating)
	{
		auto fileRef = TagLib::FileRef(TagLib::FileName(filename.toUtf8()));
		QVERIFY(Tagging::isValidFile(fileRef));

		const auto parsedTag = Tagging::getParsedTagFromFileRef(fileRef);
		auto* tag = parsedTag.mp4Tag();

		auto frame = FrameType(tag);
		auto popularimeter = Models::Popularimeter({}, rating, 1);
		frame.write(popularimeter);
		QVERIFY(frame.isFrameAvailable());

		const auto saved = fileRef.save();
		QVERIFY(saved);
	}

	template<typename FrameType>
	Rating readFile(const QString& filename)
	{
		auto fileRef = TagLib::FileRef(TagLib::FileName(filename.toUtf8()));
		const auto parsedTag = Tagging::getParsedTagFromFileRef(fileRef);
		auto* tag = parsedTag.mp4Tag();

		auto frame = FrameType(tag);
		auto popularimeter = Models::Popularimeter {};
		frame.read(popularimeter);

		return popularimeter.rating;
	}
}

class RatingTest :
	public Test::Base
{
	Q_OBJECT

	public:
		RatingTest() :
			Test::Base("RatingTest") {}

		~RatingTest() override = default;

	private slots:

		void testWriteAndReadRatingWithFreshFile()
		{
			struct TestCase
			{
				QString sourceFile;
				bool deleteFile {false};
			};

			const auto testCases = std::array {
				TestCase {":/test/mp3test.mp3"},
				TestCase {":/test/oggtest.ogg"},
				TestCase {":/test/mp4test.mp4"}
			};

			for(const auto& testCase: testCases)
			{
				for(int i = 0; i <= 5; i++)
				{
					const auto filename = prepareFile(testCase.sourceFile, tempPath());

					auto metadata = MetaData(filename);
					auto metadataReloaded = MetaData(filename);

					Tagging::Utils::getMetaDataOfFile(metadata);
					QVERIFY(metadata.rating() == Rating::Zero);

					const auto rating = static_cast<Rating>(i);
					metadata.setRating(rating);
					QVERIFY(metadata.rating() == rating);

					Tagging::Utils::setMetaDataOfFile(metadata);
					Tagging::Utils::getMetaDataOfFile(metadataReloaded);

					QVERIFY(metadataReloaded.rating() == rating);

					Util::File::deleteFiles({filename});
				}
			}
		}

		void testWriteAndReadRatingWithUpdate()
		{
			struct TestCase
			{
				QString sourceFile;
				bool deleteFile {false};
			};

			const auto testCases = std::array {
				TestCase {":/test/mp3test.mp3"},
				TestCase {":/test/oggtest.ogg"},
				TestCase {":/test/mp4test.mp4"}
			};

			for(const auto& testCase: testCases)
			{
				const auto filename = prepareFile(testCase.sourceFile, tempPath());
				auto metadata = MetaData(filename);

				for(int i = 0; i <= 5; i++)
				{
					auto metadataReloaded = MetaData(filename);

					const auto rating = static_cast<Rating>(i);
					metadata.setRating(rating);

					Tagging::Utils::setMetaDataOfFile(metadata);
					Tagging::Utils::getMetaDataOfFile(metadataReloaded);

					QVERIFY(metadataReloaded.rating() == rating);
				}

				Util::File::deleteFiles({filename});
			}
		}

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void testMediaMonkeyFrame()
		{
			runFrameTest<MP4::MediaMonkeyRateFrame>();
		}

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void testItunesFrame()
		{
			runFrameTest<MP4::ITunesRatingFrame>();
		}

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void testRead()
		{
			struct TestCase
			{
				QString sourceFile;
				Rating expectedRating;
			};

			const auto testCases = std::array {
				TestCase {":/test/out-mediamonkey2half.mp4", Rating::Three},
				TestCase {":/test/out-mediamonkey4.mp4", Rating::Four},
				TestCase {":/test/out-winamp2.mp4", Rating::Two},
				TestCase {":/test/out-winamp4.mp4", Rating::Four},
			};

			for(const auto& testCase: testCases)
			{
				const auto filename = prepareFile(testCase.sourceFile, tempPath());
				const auto rating = readFile<MP4::MediaMonkeyRateFrame>(filename);
				QVERIFY(rating == testCase.expectedRating);
			}
		}

	private:
		template<typename FrameType>
		void runFrameTest()
		{
			for(int i = 0; i <= 5; i++)
			{
				const auto filename = prepareFile(":/test/mp4test.mp4", tempPath());
				const auto rating = static_cast<Rating>(i);

				writeFile<FrameType>(filename, rating);

				const auto receivedRating = readFile<FrameType>(filename);
				QVERIFY(receivedRating == rating);

				Util::File::deleteFiles({filename});
			}
		}
};

QTEST_GUILESS_MAIN(RatingTest)

#include "RatingTest.moc"
