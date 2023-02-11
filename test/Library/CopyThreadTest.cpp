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
#include "Common/TagReaderMock.h"
#include "Common/FileSystemMock.h"

#include "Components/Library/Importer/CopyProcessor.h"
#include "Components/Library/Importer/ImportCache.h"

// access working directory with Test::Base::tempPath("somefile.txt");

namespace
{
	constexpr const auto* libraryPath = "/path/to/library";

	std::shared_ptr<Library::ImportCache> createCache()
	{
		auto importCache = std::make_shared<Library::ImportCache>(libraryPath, std::make_shared<Test::TagReaderMock>());
		importCache->addFile("/import/dir/file1.mp3", "/import/dir");
		importCache->addFile("/import/dir/subdir/file2.mp3", "/import/dir");
		importCache->addFile("/import/dir/subsubdir/file3.mp3", "/import/dir");

		return importCache;
	}
}

class CopyThreadTest :
	public Test::Base
{
	Q_OBJECT

	public:
		CopyThreadTest() :
			Test::Base("CopyThreadTest") {}

	private slots:

		[[maybe_unused]] void testCopy() // NOLINT(readability-convert-member-functions-to-static)
		{
			const auto cache = createCache();
			const auto fileSystem = std::make_shared<Test::FileSystemMock>(QMap<QString, QStringList> {
				{"/import/dir",           {"file1.mp3"}},
				{"/import/dir/subdir",    {"file2.mp3"}},
				{"/import/dir/subsubdir", {"file3.mp3"}},
				{"/path/to/library",      {}}
			});

			auto* copyThread = Library::CopyProcessor::create("someDir", cache, fileSystem);
			copyThread->copy();

			QVERIFY(fileSystem->exists("/path/to/library/someDir/file1.mp3"));
			QVERIFY(fileSystem->exists("/path/to/library/someDir/subdir/file2.mp3"));
			QVERIFY(fileSystem->exists("/path/to/library/someDir/subsubdir/file3.mp3"));
		}
};

QTEST_GUILESS_MAIN(CopyThreadTest)

#include "CopyThreadTest.moc"
