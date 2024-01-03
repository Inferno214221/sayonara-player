/*
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
#include "Common/TestTracks.h"
#include "Common/TaggingMocks.h"

#include "Components/Library/Threads/ReloadThread.h"
#include "Components/Library/Threads/ReloadThreadFileScanner.h"
#include "Database/Connector.h"
#include "Database/LibraryDatabase.h"
#include "Database/Library.h"
#include "Utils/MetaData/MetaData.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Algorithm.h"
#include "Utils/FileUtils.h"
#include "Utils/Tagging/TagReader.h"

#include <QSignalSpy>

#include <optional>

namespace
{
	class FileScannerMock :
		public Library::ReloadThreadFileScanner
	{
		public:
			FileScannerMock(const QStringList& files, const QStringList& additionalExistingFiles) :
				m_files(files),
				m_existingFiles {QStringList() << files << additionalExistingFiles} {}

			[[nodiscard]] QStringList getFilesRecursive(const QDir& /*baseDir*/) override { return m_files; }

			[[nodiscard]] bool exists(const QString& filename) override { return m_existingFiles.contains(filename); }

			[[nodiscard]] bool checkFile(const QString& filename) override { return exists(filename); }

		private:
			QStringList m_files;
			QStringList m_existingFiles;
	};

	std::shared_ptr<Library::ReloadThread>
	createReloadThread(const LibraryId libraryId, const QString& libraryPath, const Library::ReloadQuality quality,
	                   Library::ReloadThreadFileScanner* fileScanner, const Tagging::TagReaderPtr& tagReader)
	{
		auto reloadThread = std::make_shared<Library::ReloadThread>(fileScanner, tagReader, nullptr);
		reloadThread->setLibrary(libraryId, libraryPath);
		reloadThread->setQuality(quality);

		return reloadThread;
	}

	MetaDataList createTracks(const QString& libraryPath, const int count = 1000000)
	{
		auto tracks = Test::createTracks();
		for(auto& track: tracks)
		{
			auto filepath = track.filepath();
			filepath.prepend(libraryPath);
			track.setFilepath(filepath);
		}

		while(count < tracks.count())
		{
			tracks.pop_back();
		}

		return tracks;
	}

	QStringList extractFilepaths(const MetaDataList& tracks)
	{
		auto result = QStringList {};
		Util::Algorithm::transform(tracks, result, [](const auto& track) {
			return track.filepath();
		});

		return result;
	}

	MetaDataList getTracksInDatabase(DB::LibraryDatabase* libraryDatabase)
	{
		auto tracks = MetaDataList {};
		libraryDatabase->getAllTracks(tracks);
		return tracks;
	}

	DB::LibraryDatabase* createLibraryDatabase(const LibraryId libraryId)
	{
		auto* db = DB::Connector::instance();
		const auto libraryDatabases = db->libraryDatabases();
		for(auto* libDb: libraryDatabases)
		{
			db->deleteLibraryDatabase(libDb->libraryId());
		}

		db->registerLibraryDatabase(libraryId);

		auto* libraryDb = db->libraryDatabase(libraryId, db->databaseId());
		libraryDb->deleteAllTracks(false);

		return libraryDb;
	}
}

class ReloadThreadTest :
	public Test::Base
{
	Q_OBJECT

	public:
		ReloadThreadTest() :
			Test::Base("ReloadThreadTest") {}

	private slots:
		[[maybe_unused]] void testSignals();
		[[maybe_unused]] void testRemoveNonExistentTracks();
		[[maybe_unused]] void testTaggingOperations();
};

[[maybe_unused]] void ReloadThreadTest::testSignals()
{
	constexpr const auto TrackCount = 100;
	constexpr const auto libraryId = 1;

	createLibraryDatabase(libraryId);

	auto tracks = MetaDataList {};
	for(int i = 0; i < TrackCount; i++)
	{
		tracks
			<< Test::createTrack(i, QString("title%1").arg(i), QString("artist%1").arg(i), QString("album%1").arg(i));
	}

	const auto libraryPath = Test::Base::tempPath();
	auto* fileScanner = new FileScannerMock(extractFilepaths(tracks), {libraryPath});
	auto tagReader = std::make_shared<Test::TagReaderMock>();
	auto reloadThread =
		createReloadThread(libraryId, libraryPath, Library::ReloadQuality::Fast, fileScanner, tagReader);

	auto spyFinished = QSignalSpy(reloadThread.get(), &Library::ReloadThread::finished);
	auto spyReloadingLibrary = QSignalSpy(reloadThread.get(), &Library::ReloadThread::sigReloadingLibrary);

	reloadThread->start();

	spyFinished.wait();

	QVERIFY(spyReloadingLibrary.takeFirst().at(0).toString().startsWith("Analyzing library"));
	QVERIFY(spyReloadingLibrary.takeFirst().at(0).toString().startsWith("Deleting double tracks"));
	QVERIFY(spyReloadingLibrary.takeFirst().at(0).toString().startsWith("Deleting orphaned tracks"));
	QVERIFY(spyReloadingLibrary.count() == TrackCount);
}

[[maybe_unused]] void ReloadThreadTest::testRemoveNonExistentTracks()
{
	constexpr const auto libraryId = 2;
	const auto libraryPath = Test::Base::tempPath();

	struct TestCase
	{
		int trackCountInDb;
		int scannedTracks;
		int expectedTracks;
	};

	constexpr const auto testCases = std::array {
		TestCase {20, 0, 0},
		TestCase {10, 5, 5},
		TestCase {5, 10, 10}
	};

	for(const auto& testCase: testCases)
	{
		auto* libraryDb = createLibraryDatabase(libraryId);

		const auto tracks = createTracks(libraryPath, testCase.trackCountInDb);
		const auto success = libraryDb->storeMetadata(tracks);
		QVERIFY(success);
		QVERIFY(getTracksInDatabase(libraryDb).count() == tracks.count());

		const auto scannedTracks = createTracks(libraryPath, testCase.scannedTracks);
		auto* fileScanner = new FileScannerMock(extractFilepaths(scannedTracks), {libraryPath});
		auto tagReader = std::make_shared<Test::TagReaderMock>();
		auto reloadThread =
			createReloadThread(libraryId, libraryPath, Library::ReloadQuality::Fast, fileScanner, tagReader);

		auto spy = QSignalSpy(reloadThread.get(), &Library::ReloadThread::finished);

		reloadThread->start();

		spy.wait();
		QVERIFY(spy.count() == 1);

		const auto tracksInDatabase = getTracksInDatabase(libraryDb);
		const auto filepathsInDatabase = extractFilepaths(tracksInDatabase);

		const auto expectedTracks = createTracks(libraryPath, testCase.expectedTracks);
		QVERIFY(filepathsInDatabase == extractFilepaths(expectedTracks));
	}
}

[[maybe_unused]]void ReloadThreadTest::testTaggingOperations()
{
	const auto libraryPath = Test::Base::tempPath();
	constexpr const auto libraryId = 3;

	struct TestCase
	{
		int trackCountInDb;
		int scannedTracks;
		Library::ReloadQuality quality;
		int expectedTagReads;
	};

	constexpr const auto testCases = std::array {
		TestCase {20, 0, Library::ReloadQuality::Fast, 0},
		TestCase {20, 0, Library::ReloadQuality::Accurate, 0},
		TestCase {0, 20, Library::ReloadQuality::Fast, 20},
		TestCase {0, 20, Library::ReloadQuality::Accurate, 20},
		TestCase {10, 5, Library::ReloadQuality::Fast, 0},
		TestCase {10, 5, Library::ReloadQuality::Accurate, 5},
		TestCase {5, 10, Library::ReloadQuality::Fast, 5},
		TestCase {5, 10, Library::ReloadQuality::Accurate, 10}
	};

	for(const auto& testCase: testCases)
	{
		auto* libraryDb = createLibraryDatabase(libraryId);

		const auto tracks = createTracks(libraryPath, testCase.trackCountInDb);
		const auto success = libraryDb->storeMetadata(tracks);
		QVERIFY(success);
		QVERIFY(getTracksInDatabase(libraryDb).count() == tracks.count());

		const auto scannedTracks = createTracks(libraryPath, testCase.scannedTracks);
		auto* fileScanner = new FileScannerMock(extractFilepaths(scannedTracks), {libraryPath});
		auto tagReader = std::make_shared<Test::TagReaderMock>();
		auto reloadThread = createReloadThread(libraryId, libraryPath, testCase.quality, fileScanner, tagReader);

		auto spy = QSignalSpy(reloadThread.get(), &Library::ReloadThread::finished);

		reloadThread->start();

		spy.wait();
		QVERIFY(spy.count() == 1);
		QVERIFY(tagReader->count() == testCase.expectedTagReads);
	}
}

QTEST_GUILESS_MAIN(ReloadThreadTest)

#include "ReloadThreadTest.moc"
