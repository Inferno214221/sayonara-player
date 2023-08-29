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
#include "Common/TaggingMocks.h"

#include "Database/Connector.h"
#include "Database/LibraryDatabase.h"
#include "Components/Library/Importer/LibraryImporter.h"
#include "Utils/MetaData/MetaDataList.h"

#include <QDir>
#include <QMap>
#include <QSignalSpy>
#include <QStringList>
// access working directory with Test::Base::tempPath("somefile.txt");

Q_DECLARE_METATYPE(Library::Importer::ImportStatus);

namespace
{
	constexpr const auto ThreadTimeout = 100;
	constexpr const LibraryId TestLibraryId = 1;
	const auto LibraryDir = QString {"/path/to/library"};
	const auto ImportTargetDir = QString {"importedSound"};
	const auto ImportSourceDir = QString {"/tmp/dirToImport"};
	const auto FullTargetDir = LibraryDir + '/' + ImportTargetDir;

	std::unique_ptr<Library::Importer> createImporter(const Util::FileSystemPtr& fileSystem)
	{
		auto* db = DB::Connector::instance();
		db->registerLibraryDatabase(TestLibraryId);

		auto* libraryDatabase = db->libraryDatabase(TestLibraryId, db->databaseId());
		return std::make_unique<Library::Importer>(libraryDatabase,
		                                           fileSystem,
		                                           std::make_shared<Test::TagReaderMock>(),
		                                           nullptr);
	}

	std::pair<Util::FileSystemPtr, QStringList>
	createFileSystem(const QString& sourceDir, const QStringList& filesInImportDir, const QString& targetDir)
	{
		auto sourceFilepaths = QStringList {};
		for(const auto& file: filesInImportDir)
		{
			sourceFilepaths << QDir(sourceDir).absoluteFilePath(file);
		}

		auto fileSystem = std::make_shared<Test::FileSystemMock>(QMap<QString, QStringList> {
			{sourceDir, filesInImportDir},
			{targetDir, {}}
		});

		return {fileSystem, sourceFilepaths};
	}

	void clearDatabase()
	{
		auto* db = DB::Connector::instance();
		auto* libraryDatabase = DB::Connector::instance()->libraryDatabase(TestLibraryId, db->databaseId());
		libraryDatabase->deleteAllTracks(false);

		auto tracks = MetaDataList {};
		libraryDatabase->getAllTracks(tracks);
		QVERIFY(tracks.isEmpty());
	}

	int getTrackCountInLibrary()
	{
		auto* db = DB::Connector::instance();
		auto* libraryDatabase = DB::Connector::instance()->libraryDatabase(TestLibraryId, db->databaseId());
		auto tracks = MetaDataList {};
		libraryDatabase->getAllTracks(tracks);

		return tracks.count();
	}

	bool checkImportStatusInSpy(const QSignalSpy& spy, const int index, Library::Importer::ImportStatus status)
	{
		return spy[index].at(0).value<Library::Importer::ImportStatus>() == status;
	}
}

class LibraryImporterTest :
	public Test::Base
{
	Q_OBJECT

	public:
		LibraryImporterTest() :
			Test::Base("LibraryImporterTest")
		{
			qRegisterMetaType<Library::Importer::ImportStatus>("ImportStatus");
		}

	private slots:

		[[maybe_unused]] void testSuccessfulCaching() // NOLINT(readability-convert-member-functions-to-static)
		{
			const auto filesToImport = QStringList {"file1.mp3", "file2.mp3", "file3.mp3"};
			auto [fileSystem, sourceFilepaths] = createFileSystem(ImportSourceDir, filesToImport, ImportTargetDir);

			auto libraryImporter = createImporter(fileSystem);
			auto spy = QSignalSpy(libraryImporter.get(), &Library::Importer::sigStatusChanged);

			libraryImporter->import(LibraryDir, sourceFilepaths, FullTargetDir);

			spy.wait(ThreadTimeout);

			QVERIFY(spy.count() == 2);
			QVERIFY(checkImportStatusInSpy(spy, 0, Library::Importer::ImportStatus::Caching));
			QVERIFY(checkImportStatusInSpy(spy, 1, Library::Importer::ImportStatus::CachingFinished));
			QVERIFY(libraryImporter->cachedTracks().count() == 3);
		}

		[[maybe_unused]] void testEmptyCaching() // NOLINT(readability-convert-member-functions-to-static)
		{
			auto [fileSystem, sourceFilepaths] = createFileSystem(ImportSourceDir, {}, FullTargetDir);
			auto libraryImporter = createImporter(fileSystem);

			qRegisterMetaType<Library::Importer::ImportStatus>("bla");
			auto spy = QSignalSpy(libraryImporter.get(), &Library::Importer::sigStatusChanged);

			libraryImporter->import(LibraryDir, sourceFilepaths, FullTargetDir);

			spy.wait(ThreadTimeout);

			QVERIFY(spy.count() == 1);
			QVERIFY(checkImportStatusInSpy(spy, 0, Library::Importer::ImportStatus::NoValidTracks));
			QVERIFY(libraryImporter->cachedTracks().isEmpty());
		}

		[[maybe_unused]] void testNoSoundfilesCaching() // NOLINT(readability-convert-member-functions-to-static)
		{
			const auto filesToImport = QStringList {"file1.jpg", "file2.png"};
			auto [fileSystem, sourceFilepaths] = createFileSystem(ImportSourceDir, filesToImport, FullTargetDir);

			auto libraryImporter = createImporter(fileSystem);
			auto spy = QSignalSpy(libraryImporter.get(), &Library::Importer::sigStatusChanged);

			libraryImporter->import(LibraryDir, sourceFilepaths, FullTargetDir);

			spy.wait(ThreadTimeout);

			QVERIFY(spy.count() == 2);
			QVERIFY(checkImportStatusInSpy(spy, 0, Library::Importer::ImportStatus::Caching));
			QVERIFY(checkImportStatusInSpy(spy, 1, Library::Importer::ImportStatus::NoTracks));
			QVERIFY(libraryImporter->cachedTracks().isEmpty());
		}

		[[maybe_unused]] void testSuccessfulCopy() // NOLINT(readability-convert-member-functions-to-static)
		{
			clearDatabase();

			const auto filesToImport = QStringList {"file1.mp3", "file2.mp3", "file3.mp3"};
			auto [fileSystem, sourceFilepaths] = createFileSystem(ImportSourceDir, filesToImport, FullTargetDir);

			auto libraryImporter = createImporter(fileSystem);
			auto spy = QSignalSpy(libraryImporter.get(), &Library::Importer::sigStatusChanged);

			libraryImporter->import(LibraryDir, sourceFilepaths, FullTargetDir);
			spy.wait(ThreadTimeout);

			libraryImporter->copy(ImportTargetDir);
			spy.wait(ThreadTimeout);

			QVERIFY(spy.count() == 4);
			QVERIFY(checkImportStatusInSpy(spy, 2, Library::Importer::ImportStatus::Importing));
			QVERIFY(checkImportStatusInSpy(spy, 3, Library::Importer::ImportStatus::Imported));
			QVERIFY(fileSystem->exists("/path/to/library/importedSound/file1.mp3"));
			QVERIFY(getTrackCountInLibrary() == 3);
		}

		[[maybe_unused]] void testImportLibraryTracks() // NOLINT(readability-convert-member-functions-to-static)
		{
			clearDatabase();

			const auto filesToImport = QStringList {"file1.mp3", "file2.mp3", "file3.mp3"};
			auto [fileSystem, sourceFilepaths] = createFileSystem(LibraryDir, filesToImport, FullTargetDir);

			auto libraryImporter = createImporter(fileSystem);
			auto spy = QSignalSpy(libraryImporter.get(), &Library::Importer::sigStatusChanged);

			libraryImporter->import(LibraryDir, sourceFilepaths, FullTargetDir);
			spy.wait(ThreadTimeout);

			QVERIFY(spy.count() == 1);
			QVERIFY(checkImportStatusInSpy(spy, 0, Library::Importer::ImportStatus::NoValidTracks));
			QVERIFY(libraryImporter->cachedTracks().isEmpty());
		}
};

QTEST_GUILESS_MAIN(LibraryImporterTest)

#include "LibraryImporterTest.moc"
