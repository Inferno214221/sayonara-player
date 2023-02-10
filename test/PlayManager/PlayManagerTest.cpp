/*
 * Copyright (C) 2011-2021 Michael Lugmair
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

#include "test/Common/SayonaraTest.h"

#include "Components/PlayManager/PlayManager.h"
#include "Components/Playlist/Playlist.h"
#include "Components/Playlist/PlaylistHandler.h"
#include "Components/Playlist/PlaylistLoader.h"
#include "Components/Notification/NotificationHandler.h"
#include "Utils/MetaData/MetaData.h"
#include "Utils/Playlist/CustomPlaylist.h"
#include "Utils/Settings/Settings.h"

#include <QList>
#include <QSignalSpy>

// access working directory with Test::Base::tempPath("somefile.txt");

namespace
{
	MetaData createRadioTrack(const QString& url, const QString& name)
	{
		MetaData track;
		track.setRadioStation(url, name);
		track.setFilepath(url);

		return track;
	}

	class PlaylistLoaderMock : public Playlist::Loader
	{
		public:
			int getLastPlaylistIndex() const override { return 0; }

			int getLastTrackIndex() const override { return 0; }

			const QList<CustomPlaylist>& playlists() const override { return mPlaylists; }

		private:
			QList<CustomPlaylist> mPlaylists;
	};
}

class PlayManagerTest :
	public Test::Base
{
	Q_OBJECT

	public:
		PlayManagerTest() :
			Test::Base("PlayManagerTest")
		{
			qRegisterMetaType<MetaData>();
		}

	private slots:
		void testRadioHistory();
		void testCurrentTrack();
};

void PlayManagerTest::testRadioHistory()
{
	qRegisterMetaType<MetaData>();
	SetSetting(Set::Stream_ShowHistory, true);
	auto* playManager = PlayManager::create(nullptr);

	const auto title1 = QStringLiteral("title1");
	const auto title2 = QStringLiteral("some other title");
	const auto title3 = QStringLiteral("and here another one");
	const auto title4 = QStringLiteral("last one");

	const auto streamUrl = QStringLiteral("https://path.to.radio.mp3");

	auto track1 = createRadioTrack(streamUrl, title1);
	auto track2 = createRadioTrack(streamUrl, title2);
	auto track3 = createRadioTrack(streamUrl, title3);
	auto track4 = createRadioTrack(streamUrl, title4);

	QSignalSpy spy(playManager, &PlayManager::sigStreamFinished);

	playManager->changeCurrentTrack(track1, 0); //        _, _, _ -> 1*, _, _
	QVERIFY(playManager->currentTrack().title() == title1);

	playManager->changeCurrentMetadata(track2); // emit   1, _, _ -> 1, 2*, _
	QVERIFY(playManager->currentTrack().title() == title2);

	playManager->changeCurrentMetadata(track1); // ignore 1, 2, _ -> 1, 2, 1*
	QVERIFY(playManager->currentTrack().title() == title1);

	playManager->changeCurrentMetadata(track2); // ignore 1, 2, 1 -> 2*, 2, 1
	QVERIFY(playManager->currentTrack().title() == title2);

	playManager->changeCurrentMetadata(track1); // ignore 2, 2, 1 -> 2, 1*, 1
	QVERIFY(playManager->currentTrack().title() == title1);

	playManager->changeCurrentMetadata(track3); // emit   2, 1, 1 -> 2, 1, 3*
	QVERIFY(playManager->currentTrack().title() == title3);

	playManager->changeCurrentMetadata(track1); // ignore 2, 1, 3 -> 1*, 1, 3
	QVERIFY(playManager->currentTrack().title() == title1);

	playManager->changeCurrentMetadata(track4); // emit   1, 1, 2 -> 1, 4*, 3
	QVERIFY(playManager->currentTrack().title() == title4);

	playManager->changeCurrentMetadata(track1); // ignore 1, 4, 3 -> 1, 4, 1*
	QVERIFY(playManager->currentTrack().title() == title1);

	QVERIFY(spy.count() == 3);
}

void PlayManagerTest::testCurrentTrack()
{
	auto track = MetaData{};
	track.setTitle("Title");
	track.setFilepath("/path/to/file.mp3");

	auto* playManager = PlayManager::create(nullptr);
	playManager->changeCurrentTrack(track, 0);

	QVERIFY(playManager->currentTrack().filepath() == track.filepath());
	QVERIFY(playManager->currentTrack().title() == track.title());

	playManager->stop();

	QVERIFY(playManager->currentTrack().filepath().isEmpty());
	QVERIFY(playManager->currentTrack().title().isEmpty());
}

//Q_DECLARE_METATYPE(MetaData)
QTEST_GUILESS_MAIN(PlayManagerTest)

#include "PlayManagerTest.moc"
