#include "test/Common/SayonaraTest.h"

#include "Components/Covers/CoverLocation.h"
#include "Components/Covers/CoverFetchManager.h"
#include "Components/Covers/Fetcher/CoverFetcherUrl.h"
#include "Utils/FileUtils.h"
#include "Utils/StandardPaths.h"
#include "Utils/MetaData/Album.h"
#include "Utils/MetaData/Artist.h"
#include "Utils/MetaData/MetaData.h"
#include "Utils/Tagging/Tagging.h"
#include "Utils/Tagging/TaggingCover.h"

#include <QFile>

using Cover::Location;

namespace
{
	QString coverLocationToString(const Location& location)
	{
		return QString("Cover Location: Valid? %1, Preferred path: %2, Search Term: %3, Identifier: %4")
			.arg(location.isValid())
			.arg(location.preferredPath())
			.arg(location.searchTerm())
			.arg(location.identifier());
	}

	Album createAlbum(const QString& albumPath)
	{
		Album album;
		album.setId(1);
		album.setName("Album");
		album.setAlbumArtist("AlbumArtist");
		album.setArtists({"Artist1", "Artist2"});
		album.setPathHint({albumPath});
		album.setCoverDownloadUrls({"www.from.the.internet.de", "www.from.google.de"});

		return album;
	}

	Artist createArtist()
	{
		Artist artist;
		artist.setId(1);
		artist.setName("Artist");
		artist.setCoverDownloadUrls({"www.from.the.internet.de", "www.from.google.de"});
		return artist;
	}

	MetaData createTrack(const QString& filename)
	{
		MetaData track;
		track.setId(1);
		track.setTitle("Track");
		track.setAlbumArtist("AlbumArtist");
		track.setArtist("Artist");
		track.setArtistId(1);
		track.setAlbum("Album");
		track.setAlbumId(1);
		track.setFilepath(filename);
		track.setCoverDownloadUrls({"www.from.the.internet.de", "www.from.google.de"});
		return track;
	}

	MetaData createRadioTrack()
	{
		MetaData track;
		track.setId(1);
		track.setRadioStation("https://www.myradio.de", "Radio Station");
		track.setFilepath("https://www.myradio.de/stream.mp3");
		track.setCoverDownloadUrls({"https://myimage.png",
		                            "https://www.from.the.internet.de",
		                            "https://www.from.google.de"
		                           });
		return track;
	}

	void createMP3(const QString& dir, const QString& filename, bool withCover)
	{
		const auto created = Util::File::createDirectories(dir);
		QVERIFY(created);

		constexpr const auto* source = ":/test/mp3test.mp3";
		const auto newName = QString("%1/%2").arg(dir, filename);
		Util::File::deleteFiles({newName});

		QString target;
		const auto copied = Util::File::copyFile(source, dir, target);
		QVERIFY(copied);

		const auto renamed = Util::File::renameFile(target, newName);
		QVERIFY(renamed);

		QFile file(newName);
		file.setPermissions(QFile::Permission::WriteOwner | QFile::Permission::ReadOwner |
		                    QFile::Permission::WriteGroup | QFile::Permission::ReadGroup |
		                    QFile::Permission::WriteUser | QFile::Permission::ReadUser);

		Tagging::Utils::setMetaDataOfFile(createTrack(newName));
		if(withCover)
		{
			const auto success = Tagging::writeCover(newName, ":/test/logo.png");
			QVERIFY(success);
		}
	}

	void deleteCoverFile(const QString& dir)
	{
		const auto filename = QString("%1/%2").arg(dir).arg("logo.png");
		Util::File::deleteFiles({filename});
		QVERIFY(!Util::File::exists(filename));
	}

	void createCoverFile(const QString& dir)
	{
		deleteCoverFile(dir);

		const auto created = Util::File::createDirectories(dir);
		QVERIFY(created);

		QString newName;
		const auto copied = Util::File::copyFile(":/test/logo.png", dir, newName);
		QVERIFY(copied);
	}

	void deleteAllFiles(const QString& dir = QString())
	{
		auto list = QStringList {Util::coverDirectory()};
		if(!dir.isEmpty())
		{
			list << dir;
		}

		Util::File::deleteFiles(list);
	}
}

class CoverLocationTest :
	public Test::Base
{
	Q_OBJECT

	public:
		CoverLocationTest() :
			Test::Base("CoverLocationTest") {}

		~CoverLocationTest() override = default;

	private slots:
		[[maybe_unused]] void testCopy();
		[[maybe_unused]] void testInvalidLocation();
		[[maybe_unused]] void testAlbumWithoutLocalCover();
		[[maybe_unused]] void testAlbumWithLocalCover();
		[[maybe_unused]] void testAlbumWithTrackHint();
		[[maybe_unused]] void testAlbumWithTrackHintAndLocalCover();
		[[maybe_unused]] void testTrackWithoutAnyCover();
		[[maybe_unused]] void testTrackWithCover();
		[[maybe_unused]] void testTrackWithCoverFlag();
		[[maybe_unused]] void testTrackWithLocalCover();
		[[maybe_unused]] void testRadioStationDownloadUrls();
		[[maybe_unused]] void testArtist();
		[[maybe_unused]] void testEqualAlbumAndTrackHash();
};

[[maybe_unused]] void
CoverLocationTest::testCopy() // NOLINT(readability-convert-member-functions-to-static,readability-function-cognitive-complexity)
{
	auto cl1 = Location::coverLocation("AnAlbum", "AnArtist");
	cl1.setSearchTerm("some search term");
	QVERIFY(cl1.isValid());
	QVERIFY(!cl1.hash().isEmpty());
	QVERIFY(!cl1.identifier().isEmpty());
	QVERIFY(!cl1.hashPath().isEmpty());
	QVERIFY(!coverLocationToString(cl1).isEmpty());
	QVERIFY(!cl1.searchTerm().isEmpty());
	QVERIFY(!cl1.searchUrls().isEmpty());
	QVERIFY(Util::File::isImageFile(cl1.alternativePath()));

	auto cl2 = cl1;
	QVERIFY(cl2.isValid() == cl1.isValid());
	QVERIFY(cl2.hash() == cl1.hash());
	QVERIFY(cl2.identifier() == cl1.identifier());
	QVERIFY(cl2.hashPath() == cl1.hashPath());
	QVERIFY(coverLocationToString(cl2) == coverLocationToString(cl1));
	QVERIFY(cl2.localPath() == cl1.localPath());
	QVERIFY(cl2.searchTerm() == cl1.searchTerm());

	auto cl3 = Location(cl1);
	QVERIFY(cl3.isValid() == cl1.isValid());
	QVERIFY(cl3.hash() == cl1.hash());
	QVERIFY(cl3.identifier() == cl1.identifier());
	QVERIFY(cl3.hashPath() == cl1.hashPath());
	QVERIFY(coverLocationToString(cl3) == coverLocationToString(cl1));
	QVERIFY(cl3.localPath() == cl1.localPath());
	QVERIFY(cl3.searchTerm() == cl1.searchTerm());

	const auto cl4 = std::move(cl2);
	QVERIFY(cl4.isValid() == cl1.isValid());
	QVERIFY(cl4.hash() == cl1.hash());
	QVERIFY(cl4.identifier() == cl1.identifier());
	QVERIFY(cl4.hashPath() == cl1.hashPath());
	QVERIFY(coverLocationToString(cl4) == coverLocationToString(cl1));
	QVERIFY(cl4.localPath() == cl1.localPath());
	QVERIFY(cl4.searchTerm() == cl1.searchTerm());

	const auto cl5 = Location(std::move(cl3));
	QVERIFY(cl5.isValid() == cl1.isValid());
	QVERIFY(cl5.hash() == cl1.hash());
	QVERIFY(cl5.identifier() == cl1.identifier());
	QVERIFY(cl5.hashPath() == cl1.hashPath());
	QVERIFY(coverLocationToString(cl5) == coverLocationToString(cl1));
	QVERIFY(cl5.localPath() == cl1.localPath());
	QVERIFY(cl5.searchTerm() == cl1.searchTerm());
}

[[maybe_unused]] void
CoverLocationTest::testInvalidLocation() // NOLINT(readability-convert-member-functions-to-static,readability-function-cognitive-complexity)
{
	const auto invalidLocation = Location::invalidLocation();

	QVERIFY(!invalidLocation.isValid());
	QVERIFY(invalidLocation.preferredPath() == ":/Icons/logo.png");
	QVERIFY(invalidLocation.preferredPath() == Location::invalidPath());
	QVERIFY(!invalidLocation.hasAudioFileSource());
	QVERIFY(invalidLocation.audioFileSource().isEmpty());
	QVERIFY(invalidLocation.audioFileTarget().isEmpty());
	QVERIFY(invalidLocation.localPathHints().isEmpty());
	QVERIFY(invalidLocation.localPath().isEmpty());
	QVERIFY(invalidLocation.alternativePath().isEmpty());
	QVERIFY(invalidLocation.hash().isEmpty());
	QVERIFY(invalidLocation.hashPath().isEmpty());
	QVERIFY(invalidLocation.searchUrls().isEmpty());
	QVERIFY(invalidLocation.searchTerm().isEmpty());
}

[[maybe_unused]] void
CoverLocationTest::testAlbumWithoutLocalCover() // NOLINT(readability-function-cognitive-complexity)
{
	constexpr const auto* Dir = "AlbumWithoutLocalCover";
	deleteAllFiles(tempPath(Dir));

	const auto album = createAlbum(tempPath(Dir));

	auto coverLocation = Cover::Location::coverLocation(album);

	QVERIFY(coverLocation.isValid());

	QVERIFY(!coverLocation.hasAudioFileSource());
	QVERIFY(coverLocation.audioFileSource().isEmpty());
	QVERIFY(coverLocation.audioFileTarget().isEmpty());

	QVERIFY(coverLocation.alternativePath() == Util::coverDirectory("alt_1b1e6a93de4b46e1bd46ce7d8f9d669f.png"));
	QVERIFY(coverLocation.hashPath() == Util::coverDirectory("1b1e6a93de4b46e1bd46ce7d8f9d669f"));
	QVERIFY(coverLocation.localPath().isEmpty());

	QVERIFY(coverLocation.localPathDir().isEmpty());
	QVERIFY(coverLocation.localPathHints().size() == 1);

	QVERIFY(coverLocation.preferredPath() == Location::invalidPath());

	QVERIFY(coverLocation.hasSearchUrls());
	QVERIFY(coverLocation.searchUrls().size() >= 2);
	QVERIFY(coverLocation.searchTerm().contains(album.albumArtist()));
	QVERIFY(coverLocation.searchTerm().contains(album.name()));
}

[[maybe_unused]] void CoverLocationTest::testAlbumWithLocalCover() // NOLINT(readability-function-cognitive-complexity)
{
	constexpr const auto* Dir = "AlbumWithLocalCover";
	deleteAllFiles(tempPath(Dir));

	const auto album = createAlbum(tempPath(Dir));
	createCoverFile(tempPath(Dir));

	const auto coverLocation = Cover::Location::coverLocation(album);
	QVERIFY(coverLocation.isValid());

	QVERIFY(!coverLocation.hasAudioFileSource());
	QVERIFY(coverLocation.audioFileSource().isEmpty());
	QVERIFY(coverLocation.audioFileTarget().isEmpty());

	QVERIFY(coverLocation.alternativePath() == Util::coverDirectory("alt_1b1e6a93de4b46e1bd46ce7d8f9d669f.png"));
	QVERIFY(coverLocation.hashPath() == Util::coverDirectory("1b1e6a93de4b46e1bd46ce7d8f9d669f"));
	QVERIFY(coverLocation.localPath() == Util::coverDirectory("1b1e6a93de4b46e1bd46ce7d8f9d669f"));

	QVERIFY(coverLocation.localPathDir() == tempPath(Dir));
	QVERIFY(coverLocation.localPathHints().size() == 1);

	QVERIFY(coverLocation.preferredPath() == coverLocation.hashPath());
	QVERIFY(QFileInfo(coverLocation.preferredPath()).isSymLink());

	QVERIFY(coverLocation.hasSearchUrls());
	QVERIFY(coverLocation.searchUrls().size() >= 2);
	QVERIFY(coverLocation.searchTerm().contains(album.albumArtist()));
	QVERIFY(coverLocation.searchTerm().contains(album.name()));
}

[[maybe_unused]] void CoverLocationTest::testAlbumWithTrackHint() // NOLINT(readability-function-cognitive-complexity)
{
	constexpr const auto* Dir = "AlbumWithTrackHint";
	deleteAllFiles(tempPath(Dir));

	createMP3(tempPath(Dir), "track.mp3", true);
	const auto trackPath = tempPath(QString("%1/track.mp3").arg(Dir));

	auto album = createAlbum(tempPath(Dir));
	album.setPathHint({trackPath});

	const auto coverLocation = Cover::Location::coverLocation(album);
	QVERIFY(coverLocation.isValid());

	QVERIFY(coverLocation.hasAudioFileSource());
	QVERIFY(coverLocation.audioFileSource() == trackPath);
	QVERIFY(coverLocation.audioFileTarget() == Util::coverDirectory("fromtag_1b1e6a93de4b46e1bd46ce7d8f9d669f.png"));

	QVERIFY(coverLocation.alternativePath() == Util::coverDirectory("alt_1b1e6a93de4b46e1bd46ce7d8f9d669f.png"));
	QVERIFY(coverLocation.hashPath() == Util::coverDirectory("1b1e6a93de4b46e1bd46ce7d8f9d669f"));
	QVERIFY(coverLocation.localPath().isEmpty());

	QVERIFY(coverLocation.localPathDir() == tempPath(Dir));
	QVERIFY(coverLocation.localPathHints().size() == 1);

	QVERIFY(coverLocation.preferredPath() == coverLocation.audioFileTarget());

	QVERIFY(coverLocation.hasSearchUrls());
	QVERIFY(coverLocation.searchUrls().size() >= 2);
	QVERIFY(coverLocation.searchTerm().contains(album.albumArtist()));
	QVERIFY(coverLocation.searchTerm().contains(album.name()));
}

[[maybe_unused]] void
CoverLocationTest::testAlbumWithTrackHintAndLocalCover() // NOLINT(readability-function-cognitive-complexity)
{
	constexpr const auto* Dir = "AlbumWithTrackHintAndLocalCover";
	deleteAllFiles(tempPath(Dir));

	createCoverFile(tempPath(Dir));
	createMP3(tempPath(Dir), "track.mp3", true);
	const auto trackPath = tempPath(QString("%1/track.mp3").arg(Dir));

	auto album = createAlbum(tempPath(Dir));
	album.setPathHint({tempPath(Dir), trackPath});

	const auto coverLocation = Cover::Location::coverLocation(album);
	QVERIFY(coverLocation.isValid());

	QVERIFY(coverLocation.hasAudioFileSource());
	QVERIFY(coverLocation.audioFileSource() == trackPath);
	QVERIFY(coverLocation.audioFileTarget() == Util::coverDirectory("fromtag_1b1e6a93de4b46e1bd46ce7d8f9d669f.png"));

	QVERIFY(coverLocation.alternativePath() == Util::coverDirectory("alt_1b1e6a93de4b46e1bd46ce7d8f9d669f.png"));
	QVERIFY(coverLocation.hashPath() == Util::coverDirectory("1b1e6a93de4b46e1bd46ce7d8f9d669f"));
	QVERIFY(coverLocation.localPath() == coverLocation.hashPath());

	QVERIFY(coverLocation.localPathDir() == tempPath(Dir));
	QVERIFY(coverLocation.localPathHints().size() == 2);

	QVERIFY(coverLocation.preferredPath() == coverLocation.audioFileTarget());

	QVERIFY(coverLocation.hasSearchUrls());
	QVERIFY(coverLocation.searchUrls().size() >= 2);
	QVERIFY(coverLocation.searchTerm().contains(album.albumArtist()));
	QVERIFY(coverLocation.searchTerm().contains(album.name()));
}

[[maybe_unused]] void CoverLocationTest::testTrackWithoutAnyCover() // NOLINT(readability-function-cognitive-complexity)
{
	constexpr const auto* Dir = "TrackWithoutAnyCover";
	deleteAllFiles(tempPath(Dir));

	const auto track = createTrack(tempPath(QString("%1/track.mp3").arg(Dir)));
	createMP3(tempPath(Dir), "track.mp3", false);

	const auto coverLocation = Cover::Location::coverLocation(track);
	QVERIFY(coverLocation.isValid());

	QVERIFY(!coverLocation.hasAudioFileSource());
	QVERIFY(coverLocation.audioFileSource().isEmpty());
	QVERIFY(coverLocation.audioFileTarget().isEmpty());

	QVERIFY(coverLocation.alternativePath() == Util::coverDirectory("alt_1b1e6a93de4b46e1bd46ce7d8f9d669f.png"));
	QVERIFY(coverLocation.hashPath() == Util::coverDirectory("1b1e6a93de4b46e1bd46ce7d8f9d669f"));
	QVERIFY(coverLocation.localPath().isEmpty());

	QVERIFY(coverLocation.localPathDir() == tempPath(Dir));
	QVERIFY(coverLocation.localPathHints().size() == 1);

	QVERIFY(coverLocation.preferredPath() == Location::invalidPath());

	QVERIFY(coverLocation.hasSearchUrls());
	QVERIFY(coverLocation.searchUrls().size() >= 2);
	QVERIFY(coverLocation.searchTerm().contains(track.albumArtist()));
	QVERIFY(coverLocation.searchTerm().contains(track.album()));
}

[[maybe_unused]] void CoverLocationTest::testTrackWithCover() // NOLINT(readability-function-cognitive-complexity)
{
	constexpr const auto* Dir = "TrackWithCover";
	deleteAllFiles(tempPath(Dir));

	const auto track = createTrack(tempPath(QString("%1/track.mp3").arg(Dir)));
	createCoverFile(tempPath(Dir));
	createMP3(tempPath(Dir), "track.mp3", true);

	const auto coverLocation = Cover::Location::coverLocation(track);
	QVERIFY(coverLocation.isValid());

	QVERIFY(coverLocation.hasAudioFileSource());
	QVERIFY(coverLocation.audioFileSource() == track.filepath());
	QVERIFY(coverLocation.audioFileTarget() == Util::coverDirectory("fromtag_1b1e6a93de4b46e1bd46ce7d8f9d669f.png"));
	QVERIFY(!Util::File::exists(coverLocation.audioFileTarget()));

	QVERIFY(coverLocation.alternativePath() == Util::coverDirectory("alt_1b1e6a93de4b46e1bd46ce7d8f9d669f.png"));
	QVERIFY(coverLocation.hashPath() == Util::coverDirectory("1b1e6a93de4b46e1bd46ce7d8f9d669f"));
	QVERIFY(coverLocation.localPath() == Util::coverDirectory("1b1e6a93de4b46e1bd46ce7d8f9d669f"));

	QVERIFY(coverLocation.localPathDir() == tempPath(Dir));
	QVERIFY(coverLocation.localPathHints().size() == 1);

	QVERIFY(coverLocation.preferredPath() == coverLocation.audioFileTarget());
	QVERIFY(Util::File::exists(coverLocation.preferredPath()));

	QVERIFY(coverLocation.hasSearchUrls());
	QVERIFY(coverLocation.searchUrls().size() >= 2);
	QVERIFY(coverLocation.searchTerm().contains(track.albumArtist()));
	QVERIFY(coverLocation.searchTerm().contains(track.album()));
}

[[maybe_unused]] void CoverLocationTest::testTrackWithCoverFlag() // NOLINT(readability-function-cognitive-complexity)
{
	constexpr const auto* Dir = "TrackWithCoverFlag";
	deleteAllFiles(tempPath(Dir));

	auto track = createTrack(tempPath(QString("%1/track.mp3").arg(Dir)));
	createMP3(tempPath(Dir), "track.mp3", false);
	track.addCustomField("has-album-art", QString(), "1");

	const auto coverLocation = Cover::Location::coverLocation(track);
	QVERIFY(coverLocation.isValid());

	QVERIFY(coverLocation.hasAudioFileSource());
	QVERIFY(coverLocation.audioFileSource() == track.filepath());
	QVERIFY(coverLocation.audioFileTarget() == Util::coverDirectory("fromtag_1b1e6a93de4b46e1bd46ce7d8f9d669f.png"));
	QVERIFY(!Util::File::exists(coverLocation.audioFileTarget()));

	QVERIFY(coverLocation.alternativePath() == Util::coverDirectory("alt_1b1e6a93de4b46e1bd46ce7d8f9d669f.png"));
	QVERIFY(coverLocation.hashPath() == Util::coverDirectory("1b1e6a93de4b46e1bd46ce7d8f9d669f"));
	QVERIFY(coverLocation.localPath().isEmpty());

	QVERIFY(coverLocation.localPathDir() == tempPath(Dir));
	QVERIFY(coverLocation.localPathHints().size() == 1);

	QVERIFY(coverLocation.preferredPath() == Location::invalidPath());

	QVERIFY(coverLocation.hasSearchUrls());
	QVERIFY(coverLocation.searchUrls().size() >= 2);
	QVERIFY(coverLocation.searchTerm().contains(track.albumArtist()));
	QVERIFY(coverLocation.searchTerm().contains(track.album()));
}

[[maybe_unused]] void CoverLocationTest::testTrackWithLocalCover() // NOLINT(readability-function-cognitive-complexity)
{
	constexpr const auto* Dir = "TrackWithLocalCover";
	deleteAllFiles(tempPath(Dir));

	const auto track = createTrack(tempPath(QString("%1/track.mp3").arg(Dir)));
	createCoverFile(tempPath(Dir));
	createMP3(tempPath(Dir), "track.mp3", false);

	const auto coverLocation = Cover::Location::coverLocation(track);
	QVERIFY(coverLocation.isValid());

	QVERIFY(!coverLocation.hasAudioFileSource());
	QVERIFY(coverLocation.audioFileSource().isEmpty());
	QVERIFY(coverLocation.audioFileTarget().isEmpty());

	const auto alternativePath = coverLocation.alternativePath();
	QVERIFY(coverLocation.alternativePath() == Util::coverDirectory("alt_1b1e6a93de4b46e1bd46ce7d8f9d669f.png"));
	QVERIFY(coverLocation.hashPath() == Util::coverDirectory("1b1e6a93de4b46e1bd46ce7d8f9d669f"));
	QVERIFY(coverLocation.localPath() == Util::coverDirectory("1b1e6a93de4b46e1bd46ce7d8f9d669f"));

	QVERIFY(coverLocation.localPathDir() == tempPath(Dir));
	QVERIFY(coverLocation.localPathHints().size() == 1);

	QVERIFY(coverLocation.preferredPath() == coverLocation.hashPath());
	QVERIFY(Util::File::exists(coverLocation.preferredPath()));

	QVERIFY(coverLocation.hasSearchUrls());
	QVERIFY(coverLocation.searchUrls().size() >= 2);
	QVERIFY(coverLocation.searchTerm().contains(track.albumArtist()));
	QVERIFY(coverLocation.searchTerm().contains(track.album()));
}

[[maybe_unused]] void
CoverLocationTest::testRadioStationDownloadUrls() // NOLINT(readability-convert-member-functions-to-static)
{
	const auto track = createRadioTrack();
	const auto coverLocation = Cover::Location::coverLocation(track);
	const auto searchUrls = coverLocation.searchUrls();
	const auto searchTerm = coverLocation.searchTerm();

	QVERIFY(searchUrls.size() >= 3);
	QVERIFY(searchUrls[0].identifier() == "direct");
	QVERIFY(searchUrls[0].url() == "https://myimage.png");
	QVERIFY(searchUrls[1].identifier() == "website");
	QVERIFY(searchUrls[1].url() == "https://www.from.the.internet.de");
	QVERIFY(searchUrls[2].identifier() == "website");
	QVERIFY(searchUrls[2].url() == "https://www.from.google.de");
	QVERIFY(searchTerm == "Radio Station");
}

[[maybe_unused]] void
CoverLocationTest::testArtist() // NOLINT(readability-convert-member-functions-to-static,readability-function-cognitive-complexity)
{
	deleteAllFiles();

	const auto artist = createArtist();

	const auto coverLocation = Cover::Location::coverLocation(artist);
	QVERIFY(coverLocation.isValid());

	QVERIFY(!coverLocation.hasAudioFileSource());
	QVERIFY(coverLocation.audioFileSource().isEmpty());
	QVERIFY(coverLocation.audioFileTarget().isEmpty());

	QVERIFY(coverLocation.alternativePath() == Util::coverDirectory("alt_artist_0441f9e2d94c39a70e21b83829259aa4.png"));

	QVERIFY(coverLocation.hashPath() == Util::coverDirectory("artist_0441f9e2d94c39a70e21b83829259aa4"));
	QVERIFY(coverLocation.localPath().isEmpty());

	QVERIFY(coverLocation.localPathDir().isEmpty());
	QVERIFY(coverLocation.localPathHints().isEmpty());

	QVERIFY(coverLocation.preferredPath() == Cover::Location::invalidLocation().preferredPath());
	QVERIFY(coverLocation.hasSearchUrls());
	QVERIFY(coverLocation.searchUrls().size() >= 2);
	QVERIFY(coverLocation.searchTerm() == artist.name());
}

[[maybe_unused]] void
CoverLocationTest::testEqualAlbumAndTrackHash() // NOLINT(readability-convert-member-functions-to-static)
{
	constexpr const auto* AlbumName = "Halfway to Heaven";
	constexpr const auto* ArtistName = "Europe";
	auto album = Album {};
	album.setArtists({ArtistName});
	album.setName(AlbumName);

	auto track = MetaData {};
	track.setYear(1992); // NOLINT(readability-magic-numbers)
	track.setArtist(ArtistName);
	track.setTitle("The Final Countdown");
	track.setAlbum(AlbumName);

	const auto coverLocationAlbum = Cover::Location::coverLocation(album);
	const auto coverLocationTrack = Cover::Location::coverLocation(track);

	const auto hashAlbum = coverLocationAlbum.hash();
	const auto hashPathAlbum = coverLocationAlbum.hashPath();

	const auto hashTrack = coverLocationTrack.hash();
	const auto hashPathTrack = coverLocationTrack.hashPath();

	QVERIFY(hashAlbum == hashTrack);
	QVERIFY(hashPathAlbum == hashPathTrack);
}

QTEST_MAIN(CoverLocationTest)

#include "CoverLocationTest.moc"

