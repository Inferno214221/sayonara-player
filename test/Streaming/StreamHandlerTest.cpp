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
#include "Common/PlaylistMocks.h"
#include "Common/PlayManagerMock.h"
#include "Common/FileSystemMock.h"
#include "Components/Streaming/Streams/StreamHandler.h"
#include "Components/Playlist/Playlist.h"
#include "Utils/Parser/StreamParser.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Streams/Station.h"
#include "Utils/Algorithm.h"

#include <QSignalSpy>

// access working directory with Test::Base::tempPath("somefile.txt");

namespace
{
	class PlaylistCreatorMock :
		public Playlist::Creator
	{
		public:
			~PlaylistCreatorMock() override = default;

			PlaylistPtr playlist(int  /*playlistIndex*/) override { return m_playlist; }

			PlaylistPtr playlistById(int /*playlistId*/) override { return m_playlist; }

			[[nodiscard]] QString requestNewPlaylistName(const QString& /*prefix*/) const override { return {}; }

			int createPlaylist(
				const MetaDataList& tracks, const QString& name, bool /*temporary*/,
				bool /*isLocked*/) override
			{
				auto fileSystem = std::make_shared<Test::AllFilesAvailableFileSystem>();
				m_playlist = std::make_shared<Playlist::Playlist>(0, name, m_playManager.get(), fileSystem);
				m_playlist->createPlaylist(tracks);
				return 0;
			}

			int createPlaylist(
				const QStringList& /*pathList*/, const QString& /*name*/,
				bool /*temporary*/,
				Playlist::LocalPathPlaylistCreator* /*creator*/) override { return 0; }

			int createPlaylist(const CustomPlaylist& /*customPlaylist*/) override { return 0; }

			int createEmptyPlaylist(bool /*override*/) override { return 0; }

			int createCommandLinePlaylist(
				const QStringList& /*pathList*/,
				Playlist::LocalPathPlaylistCreator* /*creator*/) override
			{
				return 0;
			}

		private:

			PlaylistPtr m_playlist;
			std::shared_ptr<PlayManagerMock> m_playManager {std::make_shared<PlayManagerMock>()};
	};

	class TestStationParser :
		public StreamParser
	{
		public:
			TestStationParser() :
				StreamParser(nullptr) {}

			void parse(
				const QString& /*name*/, const QStringList& /*urls*/, const QString& /*userAgent*/,
				int /*timeout*/) override {}

			~TestStationParser() override = default;

			void stopParsing() override {}

			[[nodiscard]] MetaDataList tracks() const override { return m_tracks; }

			[[nodiscard]] bool isStopped() const override { return false; }

			void setCoverUrl(const QString& /*coverUrl*/) override {}

			void findMetaData(const int count)
			{
				m_tracks.clear();
				for(int i = 0; i < count; i++)
				{
					m_tracks << MetaData(QString("/path/to/file%1.mp3").arg(i));
				}
				emit sigFinished(count > 0);
			}

		private:
			MetaDataList m_tracks;
	};

	bool isStreamEqual(const Stream& stream1, const Stream& stream2)
	{
		return (stream1.name() == stream2.name()) &&
		       (stream1.url() == stream2.url()) &&
		       (stream1.isUpdatable() == stream2.isUpdatable());
	}
}

class TestStationParserFactory :
	public StationParserFactory
{
	public:
		~TestStationParserFactory() override = default;

		[[nodiscard]] StreamParser* createParser() const override { return m_parser.get(); }

	private:
		std::shared_ptr<StreamParser> m_parser {std::make_shared<TestStationParser>()};
};

class StreamHandlerTest :
	public Test::Base
{
	Q_OBJECT

	public:
		StreamHandlerTest() :
			Test::Base("StreamHandlerTest") {}

	private:
		static void clearDatabase()
		{
			auto streamHandler = StreamHandler(new PlaylistCreatorMock(),
			                                   std::make_shared<TestStationParserFactory>());
			{
				auto stations = QList<StationPtr> {};
				streamHandler.getAllStreams(stations);

				for(const auto& station: stations)
				{
					streamHandler.removeStream(station->name());
				}
			}

			{
				auto stations = QList<StationPtr> {};
				streamHandler.getAllStreams(stations);
				QVERIFY(stations.isEmpty());
			}
		}

	private slots: // NOLINT(readability-redundant-access-specifiers)
		[[maybe_unused]] void testStreamIsInsertedCorrectly() // NOLINT(readability-convert-member-functions-to-static)
		{
			const auto stations = std::array {
				std::make_shared<Stream>("name1", "url1", true),
				std::make_shared<Stream>("name2", "url2", false)
			};

			auto streamHandler = StreamHandler(new PlaylistCreatorMock(),
			                                   std::make_shared<TestStationParserFactory>());

			for(const auto& station: stations)
			{
				streamHandler.addNewStream(station);

				const auto insertedStation = std::dynamic_pointer_cast<Stream>(streamHandler.station(station->name()));
				QVERIFY(isStreamEqual(*insertedStation, *station));
			}

			clearDatabase();
		}

		[[maybe_unused]] void testAddNewStream() // NOLINT(readability-convert-member-functions-to-static)
		{
			const auto stations = std::array {
				std::make_shared<Stream>("name1", "url1", true),
				std::make_shared<Stream>("name2", "url2", false)
			};

			auto streamHandler = StreamHandler(new PlaylistCreatorMock(),
			                                   std::make_shared<TestStationParserFactory>());

			for(const auto& station: stations)
			{
				streamHandler.addNewStream(station);
			}

			auto allStreams = QList<StationPtr> {};
			const auto success = streamHandler.getAllStreams(allStreams);
			QVERIFY(success);
			QVERIFY(allStreams.count() == stations.size());
			Util::Algorithm::sort(allStreams, [](const auto& stream1, const auto& stream2) {
				return (stream1->name() < stream2->name());
			});

			for(auto i = 0U; i < stations.size(); i++)
			{
				const auto castedStation = std::dynamic_pointer_cast<Stream>(allStreams[i]);
				QVERIFY(castedStation->name() ==
				        stations[i]->name()); // NOLINT(cppcoreguidelines-pro-bounds-constant-array-index)
				QVERIFY(castedStation->url() ==
				        stations[i]->url()); // NOLINT(cppcoreguidelines-pro-bounds-constant-array-index)
				QVERIFY(castedStation->isUpdatable() ==
				        stations[i]->isUpdatable()); // NOLINT(cppcoreguidelines-pro-bounds-constant-array-index)
			}

			clearDatabase();
		}

		[[maybe_unused]] void testDelete() // NOLINT(readability-convert-member-functions-to-static)
		{
			struct TestCase
			{
				QStringList names;
				int expectedEntriesAfterDeletion {0};
			};

			const auto testCases = std::array {
				TestCase {{"name1"}, 1},
				TestCase {{"name2"}, 1},
				TestCase {{"name1", "name2"}, 0},
				TestCase {{"name2", "name1"}, 0},
				TestCase {{"name3"}, 2}
			};

			for(const auto& testCase: testCases)
			{
				const auto stations = std::array {
					std::make_shared<Stream>("name1", "url1", true),
					std::make_shared<Stream>("name2", "url2", false)
				};

				auto streamHandler = StreamHandler(new PlaylistCreatorMock(),
				                                   std::make_shared<TestStationParserFactory>());

				for(const auto& station: stations)
				{
					streamHandler.addNewStream(station);
					QVERIFY(streamHandler.station(station->name()));
				}

				for(const auto& name: testCase.names)
				{
					streamHandler.removeStream(name);
				}

				auto allStreams = QList<StationPtr> {};
				streamHandler.getAllStreams(allStreams);
				QVERIFY(allStreams.count() == testCase.expectedEntriesAfterDeletion);

				clearDatabase();
			}
		}

		[[maybe_unused]] void testUpdate() // NOLINT(readability-convert-member-functions-to-static)
		{
			struct TestCase
			{
				QString name;
				std::shared_ptr<Stream> station;
				bool expectedSuccess;
			};

			const auto testCases = std::array {
				TestCase {"name1", std::make_shared<Stream>("newName1", "newUrl1", false), true},
				TestCase {"name2", std::make_shared<Stream>("newName2", "newUrl2", true), true},
				TestCase {"name3", std::make_shared<Stream>("newName3", "newUrl3", true), false}
			};

			for(const auto& testCase: testCases)
			{
				const auto stations = std::array {
					std::make_shared<Stream>("name1", "url1", true),
					std::make_shared<Stream>("name2", "url2", false)
				};

				auto streamHandler = StreamHandler(new PlaylistCreatorMock(),
				                                   std::make_shared<TestStationParserFactory>());

				for(const auto& station: stations)
				{
					streamHandler.addNewStream(station);
				}

				const auto success = streamHandler.updateStream(testCase.name, testCase.station);
				QVERIFY(success == testCase.expectedSuccess);

				if(testCase.expectedSuccess)
				{
					const auto newName = testCase.station->name();
					const auto newStation = std::dynamic_pointer_cast<Stream>(streamHandler.station(newName));

					QVERIFY(isStreamEqual(*testCase.station, *newStation));
				}

				clearDatabase();
			}
		}

		[[maybe_unused]] void testParseStation() // NOLINT(readability-convert-member-functions-to-static)
		{
			constexpr const auto playlistIndex = 0;
			struct TestCase
			{
				int parsedTracks;
				QString name;
			};

			const auto testCases = std::array {
				TestCase {5, "name1"},
				TestCase {0, "name2"}
			};

			for(const auto& testCase: testCases)
			{
				auto parserFactory = std::make_shared<TestStationParserFactory>();
				auto* parser = dynamic_cast<TestStationParser*>(parserFactory->createParser());
				auto* playlistCreator = new PlaylistCreatorMock();
				auto streamHandler = StreamHandler(playlistCreator, parserFactory);

				const auto station = std::make_shared<Stream>(testCase.name, "url", true);
				streamHandler.parseStation(station);
				parser->findMetaData(testCase.parsedTracks);

				const auto playlist = playlistCreator->playlist(playlistIndex);
				if(testCase.parsedTracks > 0)
				{
					QVERIFY(playlist->count() == testCase.parsedTracks);
					QVERIFY(playlist->name() == testCase.name);
				}
				else
				{
					QVERIFY(playlist == nullptr);
				}
			}

			clearDatabase();
		}

		[[maybe_unused]] void
		testInsertAndRemoveTemporaryStation() // NOLINT(readability-convert-member-functions-to-static)
		{
			const auto stations = std::array {
				std::make_shared<Stream>("name1", "url1", true),
				std::make_shared<Stream>("name2", "url2", false)
			};

			for(const auto& station: stations)
			{
				auto streamHandler = StreamHandler(new PlaylistCreatorMock(),
				                                   std::make_shared<TestStationParserFactory>());

				streamHandler.addTemporaryStation(station);
				QVERIFY(streamHandler.isTemporary(station->name()));

				const auto fetchedStation = std::dynamic_pointer_cast<Stream>(streamHandler.station(station->name()));
				QVERIFY(isStreamEqual(*fetchedStation, *station));

				streamHandler.removeStream(station->name());
				QVERIFY(!streamHandler.isTemporary(station->name()));
				QVERIFY(streamHandler.station(station->name()) == nullptr);
			}
		}

		[[maybe_unused]] void
		testInsertAndSaveTemporaryStation() // NOLINT(readability-convert-member-functions-to-static)
		{
			const auto stations = std::array {
				std::make_shared<Stream>("name1", "url1", true),
				std::make_shared<Stream>("name2", "url2", false)
			};

			for(const auto& station: stations)
			{
				auto streamHandler = StreamHandler(new PlaylistCreatorMock(),
				                                   std::make_shared<TestStationParserFactory>());

				streamHandler.addTemporaryStation(station);
				QVERIFY(streamHandler.isTemporary(station->name()));

				streamHandler.addNewStream(station);
				QVERIFY(!streamHandler.isTemporary(station->name()));

				const auto fetchedStation = std::dynamic_pointer_cast<Stream>(streamHandler.station(station->name()));
				QVERIFY(isStreamEqual(*fetchedStation, *station));

				clearDatabase();
			}
		}
};

QTEST_GUILESS_MAIN(StreamHandlerTest)

#include "StreamHandlerTest.moc"
