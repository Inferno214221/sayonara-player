/*
 * Copyright (C) 2011-2021 Michael Lugmair
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

#include "test/Common/SayonaraTest.h"
#include "test/Common/TestTracks.h"
#include "test/Common/FileSystemMock.h"

#include "Utils/DirectoryReader.h"
#include "Utils/FileUtils.h"
#include "Utils/FileSystem.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Tagging/Tagging.h"
#include "Utils/Tagging/TagReader.h"
#include "Utils/Algorithm.h"

#include <QMap>
#include <QStringList>

// access working directory with Test::Base::tempPath("somefile.txt");

namespace
{
	class TagReaderMock :
		public Tagging::TagReader
	{
		public:
			explicit TagReaderMock(Util::FileSystemPtr fileSystem) :
				m_fileSystem {std::move(fileSystem)} {}

			~TagReaderMock() override = default;

			std::optional<MetaData> readMetadata(const QString& filepath) override
			{
				auto track = MetaData {};
				if(m_fileSystem->exists(filepath) && !filepath.endsWith("invalid.mp3"))
				{
					track.setFilepath(filepath);
					return {track};
				}

				return std::nullopt;
			}

		private:
			Util::FileSystemPtr m_fileSystem;

	};

	std::shared_ptr<Test::FileSystemMock> createTrackFiles()
	{
		return std::make_shared<Test::FileSystemMock>(
			QMap<QString, QStringList> {
				{"/path",                {"a.mp3", "a.ogg", "invalid.mp3"}},
				{"/path/to",             {"b.mp3", "b.ogg"}},
				{"/path/to/somewhere",   {"c.mp3", "c.ogg"}},
				{"/path/to/another",     {"d.mp3", "d.ogg"}},
				{"/path/to/another/dir", {"e.mp3", "e.ogg"}}});
	}
}

class DirectoryReaderTest :
	public Test::Base
{
	Q_OBJECT

	public:
		DirectoryReaderTest() :
			Test::Base("DirectoryReaderTest") {}

	private slots:
		void testScanFilesInDirectory();
		void testScanRecursively();
};

// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
[[maybe_unused]] void DirectoryReaderTest::testScanFilesInDirectory()
{
	const auto fileSystem = createTrackFiles();
	const auto tagReader = std::make_shared<TagReaderMock>(fileSystem);
	const auto directoryReader = Util::DirectoryReader::create(fileSystem, tagReader);

	struct TestCase
	{
		QString baseDir;
		QStringList nameFilters;
		QStringList expectedFiles;
	};

	const auto testCases = std::array {
		TestCase {"/path", {}, {"/path/a.mp3", "/path/a.ogg", "/path/invalid.mp3"}},
		TestCase {"/path", {"*.mp3"}, {"/path/a.mp3", "/path/invalid.mp3"}},
		TestCase {"/path", {"ogg"}, {"/path/a.ogg"}},
		TestCase {"/path", {"m4a"}, {}},
	};

	for(auto testCase: testCases)
	{
		auto files = directoryReader->scanFilesInDirectory({testCase.baseDir}, testCase.nameFilters);
		files.sort();
		testCase.expectedFiles.sort();

		QVERIFY(files == testCase.expectedFiles);
	}
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
[[maybe_unused]] void DirectoryReaderTest::testScanRecursively()
{
	const auto fileSystem = createTrackFiles();
	const auto tagReader = std::make_shared<TagReaderMock>(fileSystem);
	auto directoryReader = Util::DirectoryReader::create(fileSystem, tagReader);
	const auto allFiles = Test::flattenFileSystemStructure(fileSystem->allFiles());

	struct TestCase
	{
		QString baseDir;
		QStringList nameFilters;
		int expectedFileCount;
	};

	const auto testCases = std::array {
		TestCase {"/", {}, 11},
		TestCase {"/", {"*.mp3", "*.ogg"}, 11},
		TestCase {"/", {"*.mp3"}, 6},
		TestCase {"/", {"*.ogg"}, 5},
		TestCase {"/path", {"*.mp3", "ogg"}, 11},
		TestCase {"/path/to", {"*.mp3", "ogg"}, 8},
		TestCase {"/path/to/another", {"*.mp3", "ogg"}, 4},
		TestCase {"/", {"*.wav", "m4a"}, 0},
	};

	for(const auto& testCase: testCases)
	{
		const auto files = directoryReader->scanFilesRecursively(testCase.baseDir, testCase.nameFilters);
		QVERIFY(files.count() == testCase.expectedFileCount);
		for(const auto& name: files)
		{
			QVERIFY(allFiles.contains(name));

			const auto [dir, filename] = Util::File::splitFilename(name);
			QVERIFY(dir.startsWith(testCase.baseDir));
		}
	}
}

QTEST_GUILESS_MAIN(DirectoryReaderTest)

#include "DirectoryReaderTest.moc"
