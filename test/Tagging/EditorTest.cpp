#include "test/Common/SayonaraTest.h"
#include "test/Common/TaggingMocks.h"
#include "test/Common/DatabaseUtils.h"

#include "Components/Tagging/Editor.h"
#include "Components/Tagging/ChangeNotifier.h"
#include "Database/Connector.h"
#include "Database/LibraryDatabase.h"
#include "Utils/Algorithm.h"
#include "Utils/FileUtils.h"
#include "Utils/MetaData/MetaData.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Tagging/TagReader.h"
#include "Utils/Tagging/TagWriter.h"
#include "Utils/Tagging/Tagging.h"
#include "Utils/Utils.h"

#include <QFile>
#include <QSignalSpy>

using namespace Tagging;

namespace
{
	class LocalTagReader :
		public Test::TagReaderMock
	{
		public:
			explicit LocalTagReader(const bool isCoverSupported) :
				m_isCoverSupported {isCoverSupported} {}

			[[nodiscard]] bool isCoverSupported(const QString& /*filepath*/) const override
			{
				return m_isCoverSupported;
			}

		private:
			bool m_isCoverSupported;
	};

	class LocalTagWriter :
		public Test::TagWriterMock
	{
		public:
			bool writeMetaData(const QString& filepath, const MetaData& track) override
			{
				return TagWriterMock::writeMetaData(filepath, track);
			}

			bool updateMetaData(const MetaData& track) override
			{
				m_tracks[track.filepath()] = track;
				return TagWriterMock::updateMetaData(track);
			}

			bool writeCover(const QString& filepath, const QPixmap& cover) override
			{
				m_covers[filepath] = cover;
				return TagWriterMock::writeCover(filepath, cover);
			}

			QMap<QString, QPixmap> covers() { return m_covers; }

			QMap<QString, MetaData> tracks() { return m_tracks; };

		private:
			QMap<QString, QPixmap> m_covers;
			QMap<QString, MetaData> m_tracks;
	};

	constexpr const auto TracksPerAlbum = 10;
	struct TrackTemplate
	{
		QString artist;
		QString album;
		QString title;
	};

	MetaDataList createTracks(const int count, const TrackTemplate& trackTemplate)
	{
		auto result = MetaDataList {};
		for(int i = 0; i < count; i++)
		{
			auto title = trackTemplate.title;
			title.replace('%', QString::number(i));

			auto track = MetaData {};
			track.setTitle(title);
			track.setArtist(trackTemplate.artist);
			track.setAlbum(trackTemplate.album);
			track.setFilepath(QString("/path/%1/%2/%3.mp3")
				                  .arg(track.artist())
				                  .arg(track.album())
				                  .arg(track.title()));

			result.push_back(std::move(track));
		}

		return result;
	}

	MetaDataList
	createAlbums(const int count, const TrackTemplate& trackTemplate, const std::function<MetaData(MetaData)>& modifier)
	{
		auto tracks = MetaDataList {};
		for(int i = 0; i < count; i++)
		{
			auto albumName = trackTemplate.album;
			albumName.replace('%', QString::number(i));

			const auto albumTemplate = TrackTemplate {
				trackTemplate.artist,
				albumName,
				trackTemplate.title
			};

			tracks << createTracks(TracksPerAlbum, albumTemplate); // NOLINT(*-magic-numbers)
		}

		for(auto& track: tracks)
		{
			track = modifier(track);
		}

		return tracks;
	}

	MetaDataList createAlbums(const int count, const TrackTemplate& trackTemplate)
	{
		return createAlbums(count, trackTemplate, [](const auto& track) { return track; });
	}

	std::pair<bool, MetaDataList> createLibrary(DB::LibraryDatabase* libraryDatabase, const MetaDataList& tracks)
	{
		const auto success = libraryDatabase->storeMetadata(tracks);
		if(!success)
		{
			return {false, {}};
		}

		auto fetchedTracks = MetaDataList {};
		libraryDatabase->getAllTracks(fetchedTracks);

		return {fetchedTracks.count() == tracks.count(), fetchedTracks};
	}
}

class EditorTest :
	public Test::Base
{
	Q_OBJECT

	public:
		EditorTest() :
			Test::Base("EditorTest") {}

		~EditorTest() override = default;

	private slots:

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void testGenerator()
		{
			const auto tracks = createAlbums(2, {"artist", "album%", "title%"});
			for(int a = 0, index = 0; a < 2; a++)
			{
				for(int t = 0; t < TracksPerAlbum; t++, index++)
				{
					QVERIFY(tracks[index].artist() == "artist");
					QVERIFY(tracks[index].album() == QString("album%1").arg(a));
					QVERIFY(tracks[index].title() == QString("title%1").arg(t));
				}
			}
		};

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void testInitialState()
		{
			struct TestCase
			{
				int albumCount {1};
				int expectedCount {0};
				bool canLoadEntireAlbum {false};
			};

			const auto testCases = std::array {
				TestCase {1, TracksPerAlbum, false},
				TestCase {0, 0, false},
				TestCase {4, 4 * TracksPerAlbum, false}
			};

			for(const auto& testCase: testCases)
			{
				const auto tracks = createAlbums(testCase.albumCount, {"artist", "album%", "title%"});
				auto editor = Editor(TagReader::create(), TagWriter::create(), tracks);
				QVERIFY(editor.count() == testCase.expectedCount);
				QVERIFY(editor.hasChanges() == false);
				QVERIFY(editor.canLoadEntireAlbum() == false);
				QVERIFY(editor.failedFiles().isEmpty());

				for(int i = 0; i < editor.count(); i++)
				{
					QVERIFY(editor.hasCoverReplacement(i) == false);
					QVERIFY(editor.metadata(i).artist() == tracks[i].artist());
					QVERIFY(editor.metadata(i).album() == tracks[i].album());
					QVERIFY(editor.metadata(i).title() == tracks[i].title());
					QVERIFY(editor.metadata(i).filepath() == tracks[i].filepath());
				}
			}
		};

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void testCanLoadEntireAlbum()
		{
			struct TestCase
			{
				int albumCount {0};
				int albumId {-1};
				bool expectedValue {false};
			};

			const auto testCases = std::array {
				TestCase {0, -1, false},
				TestCase {1, -1, false},
				TestCase {1, 5, true},
				TestCase {2, -1, false},
				TestCase {2, 5, true}
			};

			for(const auto& testCase: testCases)
			{
				auto modifier = [albumId = testCase.albumId](auto track) {
					track.setAlbumId(albumId);
					return track;
				};

				const auto tracks = createAlbums(testCase.albumCount,
				                                 {"artist", "album%", "title%"},
				                                 std::move(modifier));
				auto editor = Editor(TagReader::create(), TagWriter::create(), tracks);

				QVERIFY(editor.canLoadEntireAlbum() == testCase.expectedValue);
			}
		}

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void testIsCoverSupported()
		{
			struct TestCase
			{
				bool isCoverSupported {false};
			};

			const auto testCases = std::array {
				TestCase {true},
				TestCase {false}
			};

			for(const auto& testCase: testCases)
			{
				const auto tracks = createTracks(4, {"artist", "album", "title%"});
				auto editor = Editor(std::make_shared<LocalTagReader>(testCase.isCoverSupported),
				                     TagWriter::create(),
				                     tracks);

				for(int i = 0; i < editor.count(); i++)
				{
					QVERIFY(editor.isCoverSupported(i) == testCase.isCoverSupported);
				}
			}
		}

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void testRating()
		{
			auto tracks = createTracks(6, {"artist", "album", "title%"});
			auto editor = Editor(std::make_shared<LocalTagReader>(false), TagWriter::create(), tracks);

			for(int i = 0; i < editor.count(); i++)
			{
				QVERIFY(editor.metadata(i).rating() == static_cast<Rating>(0));
				tracks[i].setRating(static_cast<Rating>(i));
				editor.updateTrack(i, tracks[i]);
				QVERIFY(editor.metadata(i).rating() == static_cast<Rating>(i));
			}
		}

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void testUndo()
		{
			struct TestCase
			{
				QString changedArtist;
				bool expectedChanges {false};
			};

			const auto testCases = std::array {
				TestCase {"artist", false},
				TestCase {"newArtist", true}
			};

			for(const auto& testCase: testCases)
			{
				const auto tracks = createAlbums(1, {"artist", "album%", "title%"});
				const auto tracksNew = createAlbums(1, {testCase.changedArtist, "album%", "title%"});
				auto editor = Editor(std::make_shared<LocalTagReader>(false), TagWriter::create(), tracks);

				for(int i = 0; i < editor.count(); i++)
				{
					QVERIFY(!editor.hasChanges());

					editor.updateTrack(i, tracksNew[i]);
					QVERIFY(editor.hasChanges() == testCase.expectedChanges);
					QVERIFY(editor.metadata(i).isEqualDeep(tracksNew[i]));

					editor.undo(i);
					QVERIFY(!editor.hasChanges());
				}
			}
		}

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void testCanUpdateCover()
		{
			struct TestCase
			{
				int numAlbums {1};
				QString pixmapPath;
				bool isCoverSupported {false};
				bool expectedSuccess {false};
				int expectedUpdatedCovers {0};
			};

			const auto testCases = std::array {
				TestCase {1, ":/test/logo.png", true, true, TracksPerAlbum},
				TestCase {1, "asdfasdf", true, false, 0},
				TestCase {1, ":/test/logo.png", false, false, 0},
			};

			for(const auto& testCase: testCases)
			{
				const auto tagReader = std::make_shared<LocalTagReader>(testCase.isCoverSupported);
				const auto tagWriter = std::make_shared<LocalTagWriter>();
				const auto tracks = createAlbums(1, {"artist", "album%", "title%"});

				auto editor = Editor(tagReader, tagWriter, tracks);

				const auto pixmap = QPixmap(testCase.pixmapPath);
				for(int i = 0; i < editor.count(); i++)
				{
					editor.updateCover(i, pixmap);

					QVERIFY(editor.hasCoverReplacement(i) == testCase.expectedSuccess);
					QVERIFY(editor.hasChanges() == false);
				}

				QVERIFY(tagWriter->covers().count() == 0);
				editor.commit();
				QVERIFY(tagWriter->covers().count() == testCase.expectedUpdatedCovers);
			}
		}

		MetaDataList updateTrackNumber(MetaDataList tracks, Editor& editor)
		{
			for(int i = 0; i < tracks.count(); i++)
			{
				tracks[i].setTrackNumber(i + 1);
				editor.updateTrack(i, tracks[i]);
			}

			return tracks;
		}

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void testChangeNotifierIsTriggeredOnCommit()
		{
			auto [libraryCreated, tracks] =
				createLibrary(createLibraryDatabase(), createAlbums(3, {"artist", "album%", "title%"}));
			QVERIFY(libraryCreated);

			auto editor = Editor(std::make_shared<LocalTagReader>(true),
			                     std::make_shared<LocalTagWriter>(),
			                     tracks);

			tracks = updateTrackNumber(std::move(tracks), editor);

			auto* changeNotifier = Tagging::ChangeNotifier::instance();
			auto spy = QSignalSpy(changeNotifier, &Tagging::ChangeNotifier::sigMetadataChanged);

			editor.commit();
			QVERIFY(spy.count() == 1);

			const auto changedMetaData = changeNotifier->changedMetadata();
			QVERIFY(changedMetaData.count() == tracks.count());

			int index = 1;
			for(const auto& [oldTrack, newTrack]: changedMetaData)
			{
				QVERIFY(oldTrack.trackNumber() == 0);
				QVERIFY(newTrack.trackNumber() == index++);
			}
		}

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void testDatabaseIsAffectedOnCommit()
		{
			auto* library = createLibraryDatabase();
			auto [libraryCreated, tracks] =
				createLibrary(library, createAlbums(3, {"artist", "album%", "title%"}));
			QVERIFY(libraryCreated);

			auto editor = Editor(std::make_shared<LocalTagReader>(true),
			                     std::make_shared<LocalTagWriter>(),
			                     tracks);

			tracks = updateTrackNumber(std::move(tracks), editor);

			editor.commit();

			auto fetchedTracks = MetaDataList {};
			library->getAllTracks(fetchedTracks);
			Util::Algorithm::sort(fetchedTracks, [](const auto& track1, const auto track2) {
				return track1.trackNumber() < track2.trackNumber();
			});

			for(int i = 0; i < fetchedTracks.count(); i++)
			{
				QVERIFY(fetchedTracks[i].trackNumber() == i + 1);
			}
		}

	private:
		DB::LibraryDatabase* createLibraryDatabase()
		{
			auto* libraryDatabase = DB::Connector::instance()->registerLibraryDatabase(0);
			Test::DB::clearDatabase(libraryDatabase);

			auto tracks = MetaDataList {};
			libraryDatabase->getAllTracks(tracks);

			if(!tracks.isEmpty())
			{
				throw "TEST ERROR: Could not clear library database";
			}

			return libraryDatabase;
		}
};

QTEST_MAIN(EditorTest)

#include "EditorTest.moc"
