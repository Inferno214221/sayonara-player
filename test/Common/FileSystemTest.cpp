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

#include "Utils/FileUtils.h"

#include <QMap>

// access working directory with Test::Base::tempPath("somefile.txt");

namespace
{
	QMap<QString, QStringList> createStructure()
	{
		return QMap<QString, QStringList> {
			{"/some/dir",        {"file1.txt",    "file2.txt"}},
			{"/some/dir/below/", {"subfile1.txt", "subfile2.txt"}},
			{"/some/other/dir",  {"file3.txt",    "file4.txt"}}
		};
	}
}

class FileSystemTest :
	public Test::Base
{
	Q_OBJECT

	public:
		FileSystemTest() :
			Test::Base("FileSystemTest") {}

	private slots:

		[[maybe_unused]] void testExists() // NOLINT(readability-convert-member-functions-to-static)
		{
			auto fileSystem = Test::FileSystemMock(createStructure());

			QVERIFY(fileSystem.exists("/"));
			QVERIFY(fileSystem.exists("/some"));
			QVERIFY(fileSystem.exists("/some/dir"));
			QVERIFY(fileSystem.exists("/some/dir/below"));
			QVERIFY(fileSystem.exists("/some/dir/file1.txt"));
			QVERIFY(fileSystem.exists("/some/dir/below/subfile1.txt"));
			QVERIFY(fileSystem.exists("/some/other/dir/file3.txt"));
		}

		[[maybe_unused]] void testFiletypes() // NOLINT(readability-convert-member-functions-to-static)
		{
			auto fileSystem = Test::FileSystemMock(createStructure());

			QVERIFY(fileSystem.isDir("/some"));
			QVERIFY(fileSystem.isDir("/some/dir"));
			QVERIFY(fileSystem.isDir("/some/dir/"));
			QVERIFY(fileSystem.isDir("/some/dir/below"));
			QVERIFY(fileSystem.isDir("/some/other//dir"));
			QVERIFY(fileSystem.isFile("/some/dir/file2.txt"));
			QVERIFY(fileSystem.isFile("//some/./dir/below/subfile1.txt"));
			QVERIFY(fileSystem.isFile("/some/other/dir/file4.txt"));
		}

		[[maybe_unused]] void testDeleteDir() // NOLINT(readability-convert-member-functions-to-static)
		{
			auto fileSystem = Test::FileSystemMock(createStructure());

			QVERIFY(fileSystem.exists("/some/dir"));
			QVERIFY(fileSystem.exists("/some/dir/file1.txt"));
			QVERIFY(fileSystem.exists("/some/dir/file2.txt"));
			QVERIFY(fileSystem.exists("/some/dir/below/subfile1.txt"));
			QVERIFY(fileSystem.exists("/some/dir/below/subfile2.txt"));

			fileSystem.deleteFiles({"/some/dir"});

			QVERIFY(!fileSystem.exists("/some/dir"));
			QVERIFY(!fileSystem.exists("/some/dir/file1.txt"));
			QVERIFY(!fileSystem.exists("/some/dir/file2.txt"));
			QVERIFY(!fileSystem.exists("/some/dir/below/subfile1.txt"));
			QVERIFY(!fileSystem.exists("/some/dir/below/subfile2.txt"));

			QVERIFY(fileSystem.exists("/some"));
			QVERIFY(fileSystem.exists("/some/other/dir"));
		}

		[[maybe_unused]] void testWriteFile() // NOLINT(readability-convert-member-functions-to-static)
		{
			auto fileSystem = Test::FileSystemMock(createStructure());

			QVERIFY(!fileSystem.exists("/some/dir/file3.txt"));
			fileSystem.writeFile({"hallo"}, "/some/dir/file3.txt");
			QVERIFY(fileSystem.exists("/some/dir/file3.txt"));

			fileSystem.writeFile({"hallo"}, "/completely/new/path/file12.txt");
			QVERIFY(fileSystem.isDir("/completely"));
			QVERIFY(fileSystem.isDir("/completely/new"));
			QVERIFY(fileSystem.isDir("/completely/new/path"));
			QVERIFY(fileSystem.isFile("/completely/new/path/file12.txt"));
		}

		[[maybe_unused]] void testCopyFile() // NOLINT(readability-convert-member-functions-to-static)
		{
			auto fileSystem = Test::FileSystemMock(createStructure());

			struct TestCase
			{
				QString source;
				QString target;
				bool expectedSuccess;
			};

			const auto testCases = {
				TestCase {"/some/dir/file1.txt", "/some/dir/file3.txt", true},
				TestCase {"/some/dir/file1.txt", "/somewhere/different/file1.txt", true},
				TestCase {"/some/dir/file2.txt", "/some/dir/file1.txt", true},
				TestCase {"/some/dir/non-existtent.txt", "/some/dir/file1.txt", false}
			};

			for(const auto& testCase: testCases)
			{
				const auto success = fileSystem.copyFile(testCase.source, testCase.target);
				QVERIFY(success == testCase.expectedSuccess);

				if(testCase.expectedSuccess)
				{
					const auto [d, f] = Util::File::splitFilename(testCase.target);
					QVERIFY(fileSystem.isDir(d));
					QVERIFY(fileSystem.isFile(testCase.target));
				}
			}
		}
};

QTEST_GUILESS_MAIN(FileSystemTest)

#include "FileSystemTest.moc"
