#include "PlaylistTestUtils.h"

#include "test/Common/SayonaraTest.h"
#include "test/Common/PlayManagerMock.h"

#include "Components/Playlist/Playlist.h"
#include "Components/Playlist/PlaylistModifiers.h"
#include "Utils/Algorithm.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Set.h"
#include "Utils/Utils.h"

using PL = Playlist::Playlist;

namespace
{
	inline std::shared_ptr<Playlist::Playlist>
	createPlaylist(int index, int min, int max, const QString& name, PlayManager* playManager)
	{
		const auto tracks = Test::Playlist::createTrackList(min, max);
		auto playlist = std::make_shared<PL>(index, name, playManager);
		playlist->createPlaylist(tracks);

		return playlist;
	}
}

class PlaylistTest :
	public Test::Base
{
	Q_OBJECT

	public:
		PlaylistTest() :
			Test::Base("PlaylistTest"),
			m_playManager {new PlayManagerMock()} {}

		~PlaylistTest() override
		{
			delete m_playManager;
		}

	private:
		PlayManager* m_playManager;

	private slots:
		void jumpTest();
		void modifyTest();
		void insertTest();
		void trackIndexWithoutDisabledTest();
		void uniqueIdTest();
};

void PlaylistTest::jumpTest()
{
	MetaDataList tracks = Test::Playlist::createTrackList(0, 100);

	auto playlist = Playlist::Playlist(1, "Hallo", m_playManager);
	auto currentTrack = Playlist::currentTrack(playlist);
	QVERIFY(playlist.changeTrack(0) == false);
	QVERIFY(playlist.index() == 1);
	QVERIFY(runningTime(playlist) == 0);
	QVERIFY(playlist.currentTrackIndex() == -1);
	QVERIFY(!currentTrack.has_value());

	playlist.createPlaylist(tracks);
	currentTrack = Playlist::currentTrack(playlist);
	QVERIFY(playlist.tracks().size() == 100);
	QVERIFY(playlist.currentTrackIndex() == -1);
	QVERIFY(!currentTrack.has_value());

	const auto success = playlist.changeTrack(40);
	QVERIFY(success);

	currentTrack = Playlist::currentTrack(playlist);
	QVERIFY(playlist.currentTrackIndex() == 40);
	QVERIFY(currentTrack.has_value());
	QVERIFY(currentTrack->id() == 40);

	playlist.fwd();
	currentTrack = Playlist::currentTrack(playlist);
	QVERIFY(playlist.currentTrackIndex() == 41);
	QVERIFY(currentTrack.has_value());
	QVERIFY(currentTrack->id() == 41);

	playlist.stop();
	currentTrack = Playlist::currentTrack(playlist);
	QVERIFY(playlist.currentTrackIndex() == -1);
	QVERIFY(!currentTrack.has_value());
}

void PlaylistTest::modifyTest()
{
	auto tracks = Test::Playlist::createTrackList(0, 100);
	int currentIndex;

	auto playlist = PL(1, "Hallo", m_playManager);
	playlist.createPlaylist(tracks);
	const auto& plTracks = playlist.tracks();

	auto uniqueIds = plTracks.unique_ids();

	playlist.changeTrack(50);
	QVERIFY(playlist.currentTrackIndex() == 50);

	IndexSet indexes;
	{ // move indices before cur track
		{
			indexes << 1;
			indexes << 2;
			indexes << 3;
			indexes << 4;
		}

		Playlist::moveTracks(playlist, indexes, 75);
		currentIndex = playlist.currentTrackIndex();
		QVERIFY(currentIndex == 46);
	}

	{ // move before, after and with current track
		indexes.clear();
		{
			indexes << 65;        // new 12
			indexes << 32;        // new 10
			indexes << 46;        // new 11
			indexes << 6;        // new 9
		}

		Playlist::moveTracks(playlist, indexes, 10);
		currentIndex = playlist.currentTrackIndex();
		QVERIFY(currentIndex == 11);
	}

	{ // move current track
		indexes.clear();
		{
			indexes << 11;        // new 20 - indexes.size() = 18
			indexes << 12;        // new 19
		}

		Playlist::moveTracks(playlist, indexes, 20);
		currentIndex = playlist.currentTrackIndex();
		QVERIFY(currentIndex == 18);
	}

	{ // check if uids haven't changed
		auto currentUniqueIds = plTracks.unique_ids();
		QVERIFY(uniqueIds != currentUniqueIds);

		std::sort(uniqueIds.begin(), uniqueIds.end());
		std::sort(currentUniqueIds.begin(), currentUniqueIds.end());
		QVERIFY(uniqueIds == currentUniqueIds);
	}

	{ // remove a few tracks
		indexes.clear();
		{
			indexes << 1;
			indexes << 2;
			indexes << 3;
			indexes << 4;
		}

		Playlist::removeTracks(playlist, indexes);
		currentIndex = playlist.currentTrackIndex();
		QVERIFY(currentIndex == 14);
	}

	{ // finally, remove current track
		indexes.clear();
		{
			indexes << 4;
			indexes << 6;
			indexes << 9;
			indexes << 14;
			indexes << 19;
		}
	}

	Playlist::removeTracks(playlist, indexes);
	currentIndex = playlist.currentTrackIndex();
	QVERIFY(currentIndex == -1);
}

void PlaylistTest::insertTest()
{
	auto playlist = PL(1, "Hallo", m_playManager);
	playlist.createPlaylist(MetaDataList());

	{
		const auto tracks = Test::Playlist::createTrackList(0, 3);
		Playlist::insertTracks(playlist, tracks, 20);

		const auto playlistTracks = playlist.tracks();
		QVERIFY(Playlist::count(playlist) == 3);
		QVERIFY(Playlist::count(playlist) == playlistTracks.count());

		for(int i = 0; i < playlistTracks.count(); i++)
		{
			QVERIFY(playlistTracks[i].id() == i);
		}

		Playlist::clear(playlist);
		QVERIFY(Playlist::count(playlist) == 0);
	}

	{
		const auto tracks = Test::Playlist::createTrackList(0, 3);
		Playlist::insertTracks(playlist, tracks, -1);

		const auto playlistTracks = playlist.tracks();
		QVERIFY(Playlist::count(playlist) == 3);
		QVERIFY(Playlist::count(playlist) == playlistTracks.count());

		for(int i = 0; i < playlistTracks.count(); i++)
		{
			QVERIFY(playlistTracks[i].id() == i);
		}
	}

	{
		const auto tracks = Test::Playlist::createTrackList(3, 4);
		Playlist::insertTracks(playlist, tracks, -1);

		const auto playlistTracks = playlist.tracks();
		QVERIFY(Playlist::count(playlist) == 4);
		QVERIFY(Playlist::count(playlist) == playlistTracks.count());

		QVERIFY(playlistTracks.first().id() == 3);
	}

	{
		const auto tracks = Test::Playlist::createTrackList(4, 5);
		Playlist::insertTracks(playlist, tracks, 3);

		const auto playlistTracks = playlist.tracks();
		QVERIFY(Playlist::count(playlist) == 5);
		QVERIFY(Playlist::count(playlist) == playlistTracks.count());

		QVERIFY(playlistTracks[3].id() == 4);
	}

	{
		const auto tracks = Test::Playlist::createTrackList(5, 6);
		Playlist::insertTracks(playlist, tracks, Playlist::count(playlist));

		const auto playlistTracks = playlist.tracks();
		QVERIFY(Playlist::count(playlist) == 6);
		QVERIFY(Playlist::count(playlist) == playlistTracks.count());

		QVERIFY(playlistTracks.last().id() == 5);
	}
}

void PlaylistTest::trackIndexWithoutDisabledTest()
{
	{ // empty playlist
		auto playlist = PL(1, "Hallo", m_playManager);
		QVERIFY(Playlist::currentTrackWithoutDisabled(playlist) == -1);
	}

	{ // non-active playlist
		auto playlist = createPlaylist(1, 0, 10, "hallo", m_playManager);
		QVERIFY(Playlist::currentTrackWithoutDisabled(*playlist) == -1);
	}

	{ // test all enabled
		auto playlist = createPlaylist(1, 0, 10, "hallo", m_playManager);
		playlist->changeTrack(4);

		QVERIFY(playlist->currentTrackIndex() == 4);
		QVERIFY(Playlist::currentTrackWithoutDisabled(*playlist) == 4);
	}

	{ // test invalid index
		auto playlist = createPlaylist(1, 0, 10, "hallo", m_playManager);

		playlist->changeTrack(-1);
		QVERIFY(Playlist::currentTrackWithoutDisabled(*playlist) == -1);

		playlist->changeTrack(100);
		QVERIFY(Playlist::currentTrackWithoutDisabled(*playlist) == -1);
	}

	{ // all disabled except current index
		const auto currentIndex = 4;
		auto tracks = Test::Playlist::createTrackList(0, 10);
		for(auto& track: tracks)
		{
			track.setDisabled(true);
		}

		tracks[currentIndex].setDisabled(false);

		auto playlist = PL(1, "Hallo", m_playManager);
		playlist.createPlaylist(tracks);
		playlist.changeTrack(currentIndex);
		QVERIFY(Playlist::currentTrackWithoutDisabled(playlist) == 0);
	}

	{ // all enabled except current index
		const auto currentIndex = 4;
		auto tracks = Test::Playlist::createTrackList(0, 10);

		tracks[currentIndex].setDisabled(true);

		auto playlist = PL(1, "Hallo", m_playManager);
		playlist.createPlaylist(tracks);

		playlist.changeTrack(currentIndex);
		QVERIFY(Playlist::currentTrackWithoutDisabled(playlist) == -1);
	}

	{ // test some disabled
		auto tracks = Test::Playlist::createTrackList(0, 10);
		tracks[0].setDisabled(true);
		tracks[2].setDisabled(true);
		tracks[4].setDisabled(true);
		tracks[6].setDisabled(true);
		// enabled: 1, 3, 5, 7, 9

		auto playlist = PL(1, "Hallo", m_playManager);
		playlist.createPlaylist(tracks);

		playlist.changeTrack(4);
		QVERIFY(Playlist::currentTrackWithoutDisabled(playlist) == -1);

		playlist.changeTrack(5);
		QVERIFY(Playlist::currentTrackWithoutDisabled(playlist) == 2);
	}
}

void PlaylistTest::uniqueIdTest()
{
	auto playlist = createPlaylist(1, 0, 10, "hallo", m_playManager);

	auto uniqueIds = QList<UniqueId> {};
	Util::Algorithm::transform(playlist->tracks(), uniqueIds, [](const MetaData& track) {
		return track.uniqueId();
	});

	Playlist::reverse(*playlist);

	auto newUniqueIds = QList<UniqueId> {};
	Util::Algorithm::transform(playlist->tracks(), newUniqueIds, [](const auto& track) {
		return track.uniqueId();
	});

	QVERIFY(uniqueIds != newUniqueIds);
	std::reverse(uniqueIds.begin(), uniqueIds.end());

	QVERIFY(uniqueIds == newUniqueIds);
}

QTEST_GUILESS_MAIN(PlaylistTest)

#include "PlaylistTest.moc"

