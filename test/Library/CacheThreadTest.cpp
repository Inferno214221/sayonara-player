/*
 * Copyright (C) 2011-2022 Michael Lugmair
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
#include "Common/FileSystemMock.h"
#include "Common/TagReaderMock.h"
#include "Common/DirectoryReaderMock.h"
#include "Components/Library/Importer/CachingThread.h"
#include "Components/Library/Importer/ImportCache.h"

#include <QMap>
#include <QStringList>
#include <QSignalSpy>

// access working directory with Test::Base::tempPath("somefile.txt");

namespace
{
	constexpr const auto Archive = 1;
	constexpr const auto SoundFile = 1;
	constexpr const auto CoverFile = 1;

	Library::ImportCacher* createCacher(const QStringList& filesToImport, const QString& libraryPath,
	                                    const Util::ArchiveExtractorPtr& archiveExtractor,
	                                    const std::shared_ptr<Test::FileSystemMock>& fileSystem)
	{
		return Library::ImportCacher::create(filesToImport,
		                                     libraryPath,
		                                     std::make_shared<Test::TagReaderMock>(),
		                                     archiveExtractor,
		                                     std::make_shared<Test::DirectoryReaderMock>(fileSystem),
		                                     fileSystem);
	}
}

class CacheThreadTest :
	public Test::Base
{
	Q_OBJECT

	public:
		CacheThreadTest() :
			Test::Base("CacheThreadTest") {}

	private slots:
		[[maybe_unused]] void testDirRecursion();
		[[maybe_unused]] void testSupportedArchives();
		[[maybe_unused]] void testArchivesWithinDir();
};

// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
[[maybe_unused]] void CacheThreadTest::testDirRecursion()
{
	constexpr const auto* libraryPath = "/my/awesome/library";
	constexpr const auto* importPath = "/path/to/import";

	const auto fileStructure = QMap<QString, QStringList> {
		{libraryPath,               {}},
		{"/path/to/import",         {"file1.mp3",   "file2.mp3",   "file3.mp3",   "file4.mp3"}},
		{"/path/to/import/sub",     {"file11.mp3",  "file12.mp3",  "file13.mp3",  "file14.mp3"}},
		{"/path/to/import/sub/sub", {"file111.mp3", "file112.mp3", "file113.mp3", "file114.mp3"}}
	};

	auto fileSystem = std::make_shared<Test::FileSystemMock>(fileStructure);
	auto archiveExtractor =
		std::make_shared<Test::ArchiveExtractorMock>(QMap<QString, QStringList> {}, QStringList {}, fileSystem);

	auto* cacher = createCacher({importPath}, libraryPath, archiveExtractor, fileSystem);
	cacher->cacheFiles();
	const auto result = cacher->cacheResult();

	QVERIFY(result.cache->soundFileCount() == 12);
	QVERIFY(result.temporaryFiles.isEmpty());
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
[[maybe_unused]] void CacheThreadTest::testSupportedArchives()
{
	constexpr const auto* importPath = "/path/to/archives";
	constexpr const auto* libraryPath = "/my/awesome/library";

	const auto fileStructure = QMap<QString, QStringList> {
		{libraryPath, {}},
		{importPath,  {"file1.zip", "file2.rar", "file3.tgz"}},
	};

	const auto fileSystem = std::make_shared<Test::FileSystemMock>(fileStructure);

	const auto archiveContents = QMap<QString, QStringList> {
		{"file1.zip", QStringList {"zip1.mp3", "zip2.mp3", "zip3.mp3", "zip4.mp3", "Cover1.jpg"}},
		{"file2.rar", QStringList {"rar1.mp3", "rar2.mp3", "rar3.mp3", "Cover2.jpg"}},
		{"file3.tgz", QStringList {"tgz1.mp3", "tgz2.mp3"}}
	};

	struct TestCase
	{
		QStringList supportedArchives;
		int expectedCount {0};
		int expectedSoundFiles {0};
	};

	const auto testCases = {
		TestCase {{}, 3 * Archive, 0},
		TestCase {{"zip"}, 4 * SoundFile + 1 * CoverFile + 2 * Archive, 4},
		TestCase {{"zip", "rar"}, 7 * SoundFile + 2 * CoverFile + 1 * Archive, 7},
		TestCase {{"zip", "rar", "tgz"}, 9 * SoundFile + 2 * CoverFile, 9}
	};

	for(const auto& testCase: testCases)
	{
		const auto archiveExtractor =
			std::make_shared<Test::ArchiveExtractorMock>(archiveContents, testCase.supportedArchives, fileSystem);

		const auto filesToImport = QStringList {
			QString("%1/file1.zip").arg(importPath),
			QString("%1/file2.rar").arg(importPath),
			QString("%1/file3.tgz").arg(importPath),
		};

		auto* cacher = createCacher(filesToImport, libraryPath, archiveExtractor, fileSystem);
		cacher->cacheFiles();

		const auto result = cacher->cacheResult();
		QVERIFY(result.cache->count() == testCase.expectedCount);
		QVERIFY(result.cache->soundFileCount() == testCase.expectedSoundFiles);
	}
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
[[maybe_unused]] void CacheThreadTest::testArchivesWithinDir()
{
	constexpr const auto* importPath = "/path/to/archives";
	constexpr const auto* libraryPath = "/my/awesome/library";

	const auto fileStructure = QMap<QString, QStringList> {
		{libraryPath,                          {}},
		{QString("%1").arg(importPath),        {"file1.zip", "1.mp3"}},
		{QString("%1/subdir").arg(importPath), {"file2.zip", "2.mp3", "3.mp3"}},
	};

	const auto fileSystem = std::make_shared<Test::FileSystemMock>(fileStructure);

	const auto archiveContents = QMap<QString, QStringList> {
		{"file1.zip", QStringList {"zip1.mp3", "zip2.mp3", "zip3.mp3", "zip4.mp3", "Cover1.jpg"}},
		{"file2.zip", QStringList {"zip5.mp3", "zip6.mp3", "zip7.mp3", "Cover2.png"}}
	};

	struct TestCase
	{
		QStringList filesToImport;
		int expectedFiles {0};
		int expectedSoundFiles {0};
	};

	const auto testCases = {
		TestCase {{importPath}, 2 * Archive + 3 * SoundFile, 3},
		TestCase {{QString("%1/subdir").arg(importPath)}, 1 * Archive + 2 * SoundFile, 2},
		TestCase {{QString("%1/file1.zip").arg(importPath)}, 4 * SoundFile + 1 * CoverFile, 4},
		TestCase {{QString("%1/subdir/file2.zip").arg(importPath)}, 3 * SoundFile + 1 * CoverFile, 3},
		TestCase {{QString("%1/file1.zip").arg(importPath), QString("%1/subdir/file2.zip").arg(importPath)},
		          7 * SoundFile + 2 * CoverFile, 7
		}
	};

	for(const auto& testCase: testCases)
	{
		const auto archiveExtractor =
			std::make_shared<Test::ArchiveExtractorMock>(archiveContents, QStringList {"zip"}, fileSystem);

		auto* cacher = createCacher(testCase.filesToImport, libraryPath, archiveExtractor, fileSystem);
		cacher->cacheFiles();

		const auto result = cacher->cacheResult();
		QVERIFY(result.cache->count() == testCase.expectedFiles);
		QVERIFY(result.cache->soundFileCount() == testCase.expectedSoundFiles);
	}
}

QTEST_GUILESS_MAIN(CacheThreadTest)

#include "CacheThreadTest.moc"
