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
#include "Common/PlayManagerMock.h"
#include "Common/PlaylistMocks.h"
#include "Common/FileSystemMock.h"
#include "Common/LibraryDatabaseProvider.h"

#include "Components/DynamicPlayback/SimilarArtistFetcher.h"
#include "Components/DynamicPlayback/DynamicPlaybackHandler.h"
#include "Components/DynamicPlayback/ArtistMatch.h"
#include "Utils/MetaData/MetaData.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Settings/Settings.h"

#include <thread>
#include <QSignalSpy>


// access working directory with Test::Base::tempPath("somefile.txt");

namespace
{
	constexpr const auto* CurrentArtist = "currentArtist";
	constexpr const auto* SimilarArtist = "similarArtist";

	class TestSimilarArtistFetcher :
		public DynamicPlayback::SimilarArtistFetcher
	{
		public:
			TestSimilarArtistFetcher(const QString& artist, const DynamicPlayback::ArtistMatch& artistMatch) :
				DynamicPlayback::SimilarArtistFetcher {artist, nullptr},
				m_artistMatch {artistMatch} {}

			~TestSimilarArtistFetcher() override = default;

			[[nodiscard]] const DynamicPlayback::ArtistMatch& similarArtists() const override { return m_artistMatch; }

		protected:
			void fetchSimilarArtists(const QString& /*artist*/) override
			{
				emit sigFinished();
			}

		private:
			DynamicPlayback::ArtistMatch m_artistMatch;
	};

	class SimilarArtistFetcherFactory :
		public DynamicPlayback::SimilarArtistFetcherFactory
	{
		public:
			explicit SimilarArtistFetcherFactory(const DynamicPlayback::ArtistMatch& artistMatch) :
				m_artistMatch {artistMatch} {}

			~SimilarArtistFetcherFactory() override = default;

			DynamicPlayback::SimilarArtistFetcher* create(const QString& artist) override
			{
				return new TestSimilarArtistFetcher(artist, m_artistMatch);
			}

		private:
			DynamicPlayback::ArtistMatch m_artistMatch;
	};

	MetaData createTrack(const QString& artist, const QString& path)
	{
		auto track = MetaData {path};
		track.setArtist(artist);

		return track;
	}

	class LocalPlayManagerMock :
		public PlayManagerMock
	{
		public:
			void emitTrackChanged(const MetaData& track) { emit sigCurrentTrackChanged(track); }
	};

	class LocalPlaylistHandlerMock :
		public PlaylistHandlerMock
	{
		public:
			LocalPlaylistHandlerMock()
			{
				setActiveIndex(createPlaylist({}, "None", true, false));
			}
	};

	void setDynamicPlaybackEnabled(const bool enabled)
	{
		auto playlistMode = GetSetting(Set::PL_Mode);
		playlistMode.setDynamic(enabled, enabled);
		SetSetting(Set::PL_Mode, playlistMode);
	}

	struct TestEnv
	{
		std::shared_ptr<Test::FileSystemMock> fileSystem {std::make_shared<Test::AllFilesAvailableFileSystem>()};
		LocalPlayManagerMock playManager;
		LocalPlaylistHandlerMock playlistHandler;
		std::shared_ptr<SimilarArtistFetcherFactory> similarArtistFetcherFactory;
		std::shared_ptr<DynamicPlayback::Handler> handler;
		PlaylistPtr activePlaylist {playlistHandler.activePlaylist()};
		MetaData currentTrack;

		TestEnv(const QString& currentArtist, const DynamicPlayback::ArtistMatch& artistMatch) :
			similarArtistFetcherFactory {std::make_shared<SimilarArtistFetcherFactory>(artistMatch)},
			handler {std::make_shared<DynamicPlayback::Handler>(
				&playManager, &playlistHandler, similarArtistFetcherFactory, fileSystem)
			},
			currentTrack {createTrack(currentArtist, "/path/to/track.mp3")}
		{
			Playlist::appendTracks(*activePlaylist, MetaDataList {currentTrack}, Playlist::Reason::Undefined);
		}
	};
}

class DynamicPlaybackHandlerTest :
	public Test::Base
{
	Q_OBJECT

	public:
		DynamicPlaybackHandlerTest() :
			Test::Base("DynamicPlaybackHandlerTest")
		{
			qRegisterMetaType<MetaData>("MetaData");
		}

	private slots:

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void testTrackIsAddedWhenSimilarArtistIsAvailable()
		{
			auto libraryDatabase = Test::LibraryDatabaseProvider(0, tempPath(), {
				{"album", SimilarArtist, "track"},
			}, ::DB::ArtistIdInfo::ArtistIdField::ArtistId);

			struct TestCase
			{
				bool enabledDynamicPlayback;
				int expectedPlaylistCount;
			};

			const auto testCases = std::array {
				TestCase {true, 2},
				TestCase {false, 1},
			};

			for(const auto& testCase: testCases)
			{
				setDynamicPlaybackEnabled(testCase.enabledDynamicPlayback);

				auto artistMatch = DynamicPlayback::ArtistMatch(CurrentArtist);
				artistMatch.add({SimilarArtist, "mbid-1", 0.9}); // NOLINT(*-magic-numbers)

				auto testEnv = TestEnv(CurrentArtist, artistMatch);

				auto spy = QSignalSpy(&testEnv.playManager, &PlayManager::sigCurrentTrackChanged);
				QCOMPARE(testEnv.activePlaylist->count(), 1);

				testEnv.playManager.emitTrackChanged(testEnv.currentTrack);
				spy.wait(1000); // NOLINT(*-magic-numbers)
				QCOMPARE(testEnv.activePlaylist->count(), testCase.expectedPlaylistCount);
			}
		}

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void testUseTrackFromAnotherCluster()
		{
			auto libraryDatabase = Test::LibraryDatabaseProvider(0, tempPath(), {
				{"album", SimilarArtist, "track"},
			}, ::DB::ArtistIdInfo::ArtistIdField::ArtistId);

			struct TestCase
			{
				double similarity;
				int expectedPlaylistCount;
			};

			const auto testCases = std::array {
				TestCase {0.9, 2},
				TestCase {0.00001, 2},
			};

			for(const auto& testCase: testCases)
			{
				setDynamicPlaybackEnabled(true);

				auto artistMatch = DynamicPlayback::ArtistMatch(CurrentArtist);
				artistMatch.add({SimilarArtist, "mbid-1", testCase.similarity}); // NOLINT(*-magic-numbers)

				auto testEnv = TestEnv(CurrentArtist, artistMatch);
				auto spy = QSignalSpy(&testEnv.playManager, &PlayManager::sigCurrentTrackChanged);

				testEnv.playManager.emitTrackChanged(testEnv.currentTrack);
				spy.wait(1000); // NOLINT(*-magic-numbers)
				QCOMPARE(testEnv.activePlaylist->count(), testCase.expectedPlaylistCount);
			}
		}

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void doNotAddTheSameTrackTwice()
		{
			auto libraryDatabase = Test::LibraryDatabaseProvider(0, tempPath(), {
				{"album", SimilarArtist, "track"},
			}, ::DB::ArtistIdInfo::ArtistIdField::ArtistId);

			setDynamicPlaybackEnabled(true);

			auto artistMatch = DynamicPlayback::ArtistMatch(CurrentArtist);
			artistMatch.add({SimilarArtist, "mbid-1", 0.9}); // NOLINT(*-magic-numbers)

			auto testEnv = TestEnv(CurrentArtist, artistMatch);
			auto spy = QSignalSpy(&testEnv.playManager, &PlayManager::sigCurrentTrackChanged);

			testEnv.playManager.emitTrackChanged(testEnv.currentTrack);
			spy.wait(1000); // NOLINT(*-magic-numbers)
			QCOMPARE(testEnv.activePlaylist->count(), 2);

			testEnv.playManager.emitTrackChanged(testEnv.currentTrack);
			spy.wait(1000); // NOLINT(*-magic-numbers)
			QCOMPARE(testEnv.activePlaylist->count(), 2);
		}
};

QTEST_GUILESS_MAIN(DynamicPlaybackHandlerTest)

#include "DynamicPlaybackHandlerTest.moc"
