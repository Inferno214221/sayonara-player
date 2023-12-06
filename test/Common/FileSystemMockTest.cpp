/*
 * Copyright (C) 2011-2023 Michael Lugmair
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

#include <QMap>

// access working directory with Test::Base::tempPath("somefile.txt");

class FileSystemMockTest :
	public Test::Base
{
	Q_OBJECT

	public:
		FileSystemMockTest() :
			Test::Base("FileSystemMockTest") {}

	private slots:

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void testFilesExist()
		{
			auto fileSystem = Test::FileSystemMock {
				{
					{"/path/to", {"file1", "file2"}},
					{"/other/path", {}}
				}
			};

			QVERIFY(fileSystem.exists("/"));
			QVERIFY(fileSystem.exists("/path"));
			QVERIFY(fileSystem.exists("/path/to"));
			QVERIFY(fileSystem.exists("/path/to/file1"));
			QVERIFY(fileSystem.exists("/path/to/file2"));
			QVERIFY(!fileSystem.exists("/other/path/file3"));
			QVERIFY(!fileSystem.exists("/totally/other/path/file4"));
		}

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void testIsDirOrFile()
		{
			auto fileSystem = Test::FileSystemMock {
				{
					{"/path/to", {"file1"}},
					{"/other/path", {}}
				}
			};

			QVERIFY(fileSystem.isDir("/"));
			QVERIFY(fileSystem.isDir("/path"));
			QVERIFY(fileSystem.isDir("/path/to"));
			QVERIFY(!fileSystem.isDir("/path/to/file1"));
			QVERIFY(fileSystem.isFile("/path/to/file1"));
			QVERIFY(!fileSystem.isDir("/other/path/file2"));
			QVERIFY(!fileSystem.isFile("/other/path/file2"));
		}

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void testCreateDirectories()
		{
			auto fileSystem = Test::FileSystemMock {{}};

			fileSystem.createDirectories("/path/to");

			QVERIFY(fileSystem.exists("/"));
			QVERIFY(fileSystem.exists("/path"));
			QVERIFY(fileSystem.exists("/path/to"));
			QVERIFY(fileSystem.isDir("/"));
			QVERIFY(fileSystem.isDir("/path"));
			QVERIFY(fileSystem.isDir("/path/to"));
		}

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void testWriteFile()
		{
			struct TestCase
			{
				QString availableDirectory;
				QString filename;
				bool expectedSuccess {false};
			};

			const auto testCases = std::array {
				TestCase {"/", "/path/to/file", false},
				TestCase {"/path/to", "/path/to/file", true}
			};

			for(const auto& testCase: testCases)
			{
				auto fileSystem = Test::FileSystemMock {
					{
						{testCase.availableDirectory, {}}
					}
				};

				const auto success = fileSystem.writeFile("some data", testCase.filename);
				QVERIFY(success == testCase.expectedSuccess);
			}
		}

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void testWriteAndReadFile()
		{
			auto fileSystem = Test::FileSystemMock {
				{
					{"/path/to", {"file1"}}
				}
			};

			auto files = QStringList {"file1", "file2"};
			for(const auto& file: files)
			{
				const auto fullPath = QString("/path/to/%1").arg(file);
				const auto emptyContent = fileSystem.readFileIntoString(fullPath);
				QVERIFY(emptyContent.isEmpty());

				const auto content = QString("Content for %1").arg(file).toLocal8Bit();
				const auto success = fileSystem.writeFile(content, fullPath);
				QVERIFY(success);

				const auto data = fileSystem.readFileIntoString(fullPath);
				QVERIFY(data.toLocal8Bit() == content);
			}
		}

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void testCopyFile()
		{

			struct TestCase
			{
				QString sourceFile;
				QString targetFile;
				bool expectedSuccess {false};
			};

			const auto testCases = std::array {
				TestCase {"/path/to/file1", "/path/to/file2", true},
				TestCase {"/path/to/file1", "/another/dir", true},
				TestCase {"/path/to/file1", "/another/dir/file3", true},
				TestCase {"/path/to/file1", "/some/dir", true}, // directory structure is created in real FS
				TestCase {"/path/to/file2", "/path/to/file3", false}
			};

			for(const auto& testCase: testCases)
			{
				auto fileSystem = Test::FileSystemMock {
					{
						{"/path/to", {"file1"}},
						{"/another/dir", {}}
					}
				};

				const auto success = fileSystem.copyFile(testCase.sourceFile, testCase.targetFile);
				QVERIFY(success == testCase.expectedSuccess);
				QVERIFY(fileSystem.exists(testCase.targetFile) == success);
			}
		}

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void testDeleteFiles()
		{
			auto fileSystem = Test::FileSystemMock {
				{
					{"/path/to", {"file1", "file2", "file3"}},
					{"/another/path/to", {"file3"}}
				}
			};

			const auto filesToDelete = QStringList {
				"/path/to/file1",
				"/path/to/file2",
				"/another/path/to/file3",
			};

			QVERIFY(fileSystem.exists("/path/to/file3"));
			for(const auto& file: filesToDelete)
			{
				QVERIFY(fileSystem.exists(file));
				fileSystem.writeFile("data", file);
				QVERIFY(!fileSystem.readFileIntoString(file).isEmpty());
			}

			fileSystem.deleteFiles(filesToDelete);

			for(const auto& file: filesToDelete)
			{
				QVERIFY(!fileSystem.exists(file));
				QVERIFY(fileSystem.readFileIntoString(file).isEmpty());
			}

			QVERIFY(fileSystem.exists("/path/to/file3"));
		}

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void testDeleteDirs()
		{
			auto fileSystem = Test::FileSystemMock {
				{
					{"/path/to", {"file1", "file2"}},
					{"/path/to/another", {"file3"}},
					{"/path/to/another/dir", {"file4"}}
				}
			};

			QVERIFY(fileSystem.exists("/"));
			QVERIFY(fileSystem.exists("/path"));
			QVERIFY(fileSystem.exists("/path/to"));
			QVERIFY(fileSystem.exists("/path/to/file1"));
			QVERIFY(fileSystem.exists("/path/to/file2"));
			QVERIFY(fileSystem.exists("/path/to/another/file3"));
			QVERIFY(fileSystem.exists("/path/to/another/dir/file4"));

			fileSystem.deleteFiles({"/path/to/another"});

			QVERIFY(fileSystem.exists("/"));
			QVERIFY(fileSystem.exists("/path"));
			QVERIFY(fileSystem.exists("/path/to"));
			QVERIFY(fileSystem.exists("/path/to/file1"));
			QVERIFY(fileSystem.exists("/path/to/file2"));
			QVERIFY(!fileSystem.exists("/path/to/another/file3"));
			QVERIFY(!fileSystem.exists("/path/to/another/dir/file4"));

			fileSystem.deleteFiles({"/path"});

			QVERIFY(fileSystem.exists("/"));
			QVERIFY(!fileSystem.exists("/path"));
			QVERIFY(!fileSystem.exists("/path/to"));
			QVERIFY(!fileSystem.exists("/path/to/file1"));
			QVERIFY(!fileSystem.exists("/path/to/file2"));
			QVERIFY(!fileSystem.exists("/path/to/another/file3"));
			QVERIFY(!fileSystem.exists("/path/to/another/dir/file4"));
		}

		[[maybe_unused]] void testEntryListByNameList() // NOLINT(readability-convert-member-functions-to-static)
		{
			struct TestCase
			{
				QStringList nameFilters;
				QStringList expectedFiles;
			};

			const auto testCases = std::array {
				TestCase {{"*.mp3"},
				          {"file1.mp3"}},
				TestCase {{"mp3"},
				          {"file1.mp3"}},
				TestCase {{"mp3",       "ogg"},
				          {"file1.mp3", "file2.ogg"}},
				TestCase {{"txt"},
				          {}},
			};

			auto fileSystem = Test::FileSystemMock {
				{
					{"/path/to", {"file1.mp3", "file2.ogg", "file3.wav"}}
				}
			};

			for(const auto& testCase: testCases)
			{
				const auto files = fileSystem.entryList({"/path/to"}, testCase.nameFilters, QDir::Filter::Files);
				QVERIFY(files == testCase.expectedFiles);

				const auto dirs = fileSystem.entryList({"/path/to"}, testCase.nameFilters, QDir::Filter::Dirs);
				QVERIFY(dirs.isEmpty());
			}
		}

		[[maybe_unused]] void testEntryListByType() // NOLINT(readability-convert-member-functions-to-static)
		{
			struct TestCase
			{
				QString dir;
				QStringList exptectedSubdirs;
			};

			const auto testCases = std::array {
				TestCase {"/", {"path"}},
				TestCase {"/path", {"to"}},
				TestCase {"/path/to", {"sub1", "sub2"}}
			};

			auto fileSystem = Test::FileSystemMock {
				{
					{"/path/to/sub1", {}},
					{"/path/to/sub2", {}}
				}
			};

			for(const auto& testCase: testCases)
			{
				QVERIFY(fileSystem.entryList({testCase.dir}, {}, QDir::Filter::Dirs) == testCase.exptectedSubdirs);
			}
		}

		[[maybe_unused]] void testChangeDir() // NOLINT(readability-convert-member-functions-to-static)
		{
			struct TestCase
			{
				QString dir;
				QString targetDir;
				bool exptectedSuccess {false};
				QString expectedDir;
			};

			const auto testCases = std::array {
				TestCase {"/", "sub1", true, "/sub1"},
				TestCase {"/sub1", "sub2", false, {}},
				TestCase {"/sub1", "..", true, "/"}
			};

			auto fileSystem = Test::FileSystemMock {
				{
					{"/sub1", {}}
				}
			};

			for(const auto& testCase: testCases)
			{
				const auto dir = QDir {testCase.dir};
				const auto targetDir = fileSystem.cd(dir, testCase.targetDir);
				QVERIFY(targetDir.has_value() == testCase.exptectedSuccess);
				if(targetDir.has_value())
				{
					QVERIFY(targetDir->absolutePath() == testCase.expectedDir);
				}
			}
		}
};

QTEST_GUILESS_MAIN(FileSystemMockTest)

#include "FileSystemMockTest.moc"
