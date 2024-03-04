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
#include "Common/DatabaseUtils.h"
#include "Common/LibraryDatabaseProvider.h"

#include "Components/Library/LocalLibrary.h"
#include "Components/LibraryManagement/LibraryManager.h"
#include "Components/Playlist/LibraryPlaylistInteractor.h"
#include "Components/Tagging/ChangeNotifier.h"
#include "Database/Albums.h"
#include "Database/Connector.h"
#include "Database/LibraryDatabase.h"
#include "Utils/Algorithm.h"
#include "Utils/MetaData/Album.h"
#include "Utils/MetaData/Artist.h"
#include "Utils/MetaData/MetaDataList.h"

#include <QString>
#include <QSignalSpy>

// access working directory with Test::Base::tempPath("somefile.txt");

namespace
{
	constexpr const auto testLibraryId = 0;

	class LibraryPlaylistInteractorMock :
		public LibraryPlaylistInteractor
	{
		public:
			~LibraryPlaylistInteractorMock() override = default;

			void createPlaylist(const QStringList& /*paths*/, bool /*createNewPlaylist*/) override {}

			void createPlaylist(const MetaDataList& /*tracks*/, bool /*createNewPlaylist*/) override {}

			void append(const MetaDataList& /*tracks*/) override {}

			void insertAfterCurrentTrack(const MetaDataList& /*tracks*/) override {}
	};

	std::shared_ptr<Library::Manager> createLibraryManager(const QString& path)
	{
		auto deleter = [&](Library::Manager* libraryManager) {
			delete libraryManager->libraryInstance(testLibraryId);
			delete libraryManager;
		};

		auto* libraryPlaylistInteractor = new LibraryPlaylistInteractorMock();
		auto* libraryManager = Library::Manager::create(libraryPlaylistInteractor);
		libraryManager->addLibrary("Test Library", path);

		return std::shared_ptr<Library::Manager>(libraryManager, deleter);
	}

	MetaData changeArtist(MetaData track, const Artist& artist)
	{
		track.setArtistId(artist.id());
		track.setArtist(artist.name());

		DB::Connector::instance()->libraryDatabase(testLibraryId, 0)->updateTrack(track);

		return track;
	}

	MetaData changeAlbum(MetaData track, const Album& album)
	{
		track.setAlbumId(album.id());
		track.setAlbum(album.name());

		DB::Connector::instance()->libraryDatabase(testLibraryId, 0)->updateTrack(track);

		return track;
	}
}

class LocalLibraryTest :
	public Test::Base
{
	Q_OBJECT

	public:
		LocalLibraryTest() :
			Test::Base("LocalLibraryTest") {}

	private slots:
		[[maybe_unused]] void testTracksAreOnlyVisibleAfterInit();
		[[maybe_unused]] void testAlbumMergeSignalsAreEmmitted();
		[[maybe_unused]] void testArtistMergeSignalsAreEmmitted();

};

[[maybe_unused]] void LocalLibraryTest::testTracksAreOnlyVisibleAfterInit()
{
	auto libraryManager = createLibraryManager(Test::Base::tempPath("Library"));
	auto libraryProvider = Test::LibraryDatabaseProvider(testLibraryId, tempPath(), {
		{"album", "artist", "title"},
		{"album", "artist", "title2"},
		{"album", "artist", "title3"}
	}, DB::ArtistIdInfo::ArtistIdField::ArtistId);

	auto* localLibrary = libraryManager->libraryInstance(testLibraryId);
	auto spyAlbums = QSignalSpy(localLibrary, &LocalLibrary::sigAllAlbumsLoaded);
	auto spyArtists = QSignalSpy(localLibrary, &LocalLibrary::sigAllArtistsLoaded);
	auto spyTracks = QSignalSpy(localLibrary, &LocalLibrary::sigAllTracksLoaded);

	QVERIFY(localLibrary->albums().count() == 0);
	QVERIFY(localLibrary->artists().count() == 0);
	QVERIFY(localLibrary->tracks().isEmpty());

	QVERIFY(spyAlbums.count() == 0);
	QVERIFY(spyArtists.count() == 0);
	QVERIFY(spyTracks.count() == 0);

	localLibrary->init();

	QVERIFY(spyAlbums.count() == 1);
	QVERIFY(spyArtists.count() == 1);
	QVERIFY(spyTracks.count() == 1);

	QVERIFY(localLibrary->albums().count() == 1);
	QVERIFY(localLibrary->artists().count() == 1);
	QVERIFY(localLibrary->tracks().count() == 3);
}

[[maybe_unused]] void LocalLibraryTest::testAlbumMergeSignalsAreEmmitted()
{
	auto libraryManager = createLibraryManager(Test::Base::tempPath("Library"));

	auto libraryProvider = Test::LibraryDatabaseProvider(testLibraryId, tempPath(), {
		{"album1", "artist", "title"},
		{"album2", "artist", "title2"},
		{"album3", "artist", "title3"}
	}, DB::ArtistIdInfo::ArtistIdField::ArtistId);

	auto* localLibrary = libraryManager->libraryInstance(testLibraryId);
	localLibrary->init();

	const auto& tracks = localLibrary->tracks();

	auto trackPairs = QList<QPair<MetaData, MetaData>> {};
	for(const auto& track: tracks)
	{
		trackPairs << QPair {track, changeAlbum(track, localLibrary->albums()[0])
		};
	}

	auto spy = QSignalSpy(localLibrary, &LocalLibrary::sigAllAlbumsLoaded);

	QVERIFY(localLibrary->albums().count() == 3);
	QVERIFY(spy.count() == 0);

	Tagging::ChangeNotifier::instance()->changeMetadata(trackPairs);

	QVERIFY(spy.count() == 1);
	QVERIFY(localLibrary->albums().count() == 1);
}

[[maybe_unused]] void LocalLibraryTest::testArtistMergeSignalsAreEmmitted()
{
	auto libraryManager = createLibraryManager(Test::Base::tempPath("Library"));

	auto libraryProvider = Test::LibraryDatabaseProvider(testLibraryId, tempPath(), {
		{"album1", "artist1", "title"},
		{"album2", "artist2", "title2"},
		{"album3", "artist3", "title3"}
	}, DB::ArtistIdInfo::ArtistIdField::ArtistId);

	auto* localLibrary = libraryManager->libraryInstance(testLibraryId);

	localLibrary->init();

	const auto& tracks = localLibrary->tracks();

	auto trackPairs = QList<QPair<MetaData, MetaData>> {};
	for(const auto& track: tracks)
	{
		trackPairs << QPair {track, changeArtist(track, localLibrary->artists()[0])
		};
	}

	auto spy = QSignalSpy(localLibrary, &LocalLibrary::sigAllArtistsLoaded);

	QVERIFY(localLibrary->artists().count() == 3);
	QVERIFY(spy.count() == 0);

	Tagging::ChangeNotifier::instance()->changeMetadata(trackPairs);

	QVERIFY(spy.count() == 1);
	const auto newTracks = localLibrary->tracks();
	const auto newArtists = localLibrary->artists();
	QVERIFY(localLibrary->artists().count() == 1);
}

QTEST_GUILESS_MAIN(LocalLibraryTest)

#include "LocalLibraryTest.moc"
