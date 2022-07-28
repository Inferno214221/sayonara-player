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
#include "Common/PlayManagerMock.h"

#include "Components/SmartPlaylists/SmartPlaylistManager.h"
#include "Components/SmartPlaylists/SmartPlaylistByYear.h"
#include "Components/SmartPlaylists/SmartPlaylistByRating.h"

#include "Components/Playlist/Playlist.h"

#include "Interfaces/PlaylistInterface.h"

// access working directory with Test::Base::tempPath("somefile.txt");

class PlaylistCreatorMock :
	public PlaylistCreator
{
	public:
		~PlaylistCreatorMock() override = default;

		PlaylistPtr playlist(int playlistIndex) override { return mPlaylists[playlistIndex]; }

		PlaylistPtr playlistById(int /*playlistId*/) override { return {}; }

		[[nodiscard]] QString requestNewPlaylistName(const QString& /*prefix*/) const override { return {}; }

		int createPlaylist(const MetaDataList& /*tracks*/, const QString& name, bool /*temporary*/) override
		{
			auto playlist = std::make_shared<Playlist::Playlist>(mPlaylists.count(), name, new PlayManagerMock());
			mPlaylists.push_back(playlist);
			return mPlaylists.count() - 1;
		}

		int createPlaylist(const QStringList& /*pathList*/, const QString& /*name*/,
		                   bool /*temporary*/) override { return 0; }

		int createPlaylist(const CustomPlaylist& /*customPlaylist*/) override { return 0; }

		int createEmptyPlaylist(bool /*override*/) override { return 0; }

		int createCommandLinePlaylist(const QStringList& /*pathList*/) override { return 0; }

		[[nodiscard]] QList<PlaylistPtr> playlists() const { return mPlaylists; }

	private:
		QList<PlaylistPtr> mPlaylists;
};

class SmartPlaylistManagerTest :
	public Test::Base
{
	Q_OBJECT

	public:
		SmartPlaylistManagerTest() :
			Test::Base("SmartPlaylistManagerTest") {}

	private slots:
		[[maybe_unused]] void testInsert();
		[[maybe_unused]] void testEdit();
		[[maybe_unused]] void testDelete();
		[[maybe_unused]] void testSelect();
};

[[maybe_unused]] void SmartPlaylistManagerTest::testInsert()
{
	auto manager = SmartPlaylistManager(new PlaylistCreatorMock());
	auto smartPlaylist1 = std::make_shared<SmartPlaylistByYear>(-1, 2000, 2011);
	auto smartPlaylist2 = std::make_shared<SmartPlaylistByRating>(-1, 1, 4);

	QVERIFY(manager.smartPlaylists().count() == 0);

	manager.insertPlaylist(smartPlaylist1);
	manager.insertPlaylist(smartPlaylist2);

	QVERIFY(manager.smartPlaylist(0)->name() == smartPlaylist1->name());
	QVERIFY(manager.smartPlaylist(0)->id() >= 0);
	QVERIFY(manager.smartPlaylist(0)->value(0) == 2000);
	QVERIFY(manager.smartPlaylist(0)->value(1) == 2011);

	QVERIFY(manager.smartPlaylist(1)->name() == smartPlaylist2->name());
	QVERIFY((manager.smartPlaylist(1)->id() > 0) && (manager.smartPlaylist(1)->id() != manager.smartPlaylist(0)->id()));
	QVERIFY(manager.smartPlaylist(1)->value(0) == 1);
	QVERIFY(manager.smartPlaylist(1)->value(1) == 4);

	QVERIFY(manager.smartPlaylists().count() == 2);

	auto newManager = SmartPlaylistManager(new PlaylistCreatorMock());
	QVERIFY(newManager.smartPlaylists().count() == manager.smartPlaylists().count());
	QVERIFY(newManager.smartPlaylist(0)->id() == manager.smartPlaylist(0)->id());
	QVERIFY(newManager.smartPlaylist(1)->id() == manager.smartPlaylist(1)->id());

	manager.deletePlaylist(0);
	manager.deletePlaylist(0);
}

void SmartPlaylistManagerTest::testEdit()
{
	auto manager = SmartPlaylistManager(new PlaylistCreatorMock());

	auto smartPlaylist = std::make_shared<SmartPlaylistByYear>(-1, 2002, 2011);

	manager.insertPlaylist(smartPlaylist);
	QVERIFY(manager.smartPlaylists().count() == 1);

	const auto oldName = smartPlaylist->name();
	const auto oldId = manager.smartPlaylist(0)->id();

	auto newSmartPlaylist = manager.smartPlaylist(0);
	newSmartPlaylist->setValue(0, 2005);
	newSmartPlaylist->setValue(1, 2015);

	manager.updatePlaylist(0, newSmartPlaylist);

	QVERIFY(manager.smartPlaylist(0)->name() != oldName);
	QVERIFY(manager.smartPlaylist(0)->name() == newSmartPlaylist->name());
	QVERIFY(manager.smartPlaylist(0)->id() == oldId);
	QVERIFY(manager.smartPlaylist(0)->value(0) == 2005);
	QVERIFY(manager.smartPlaylist(0)->value(1) == 2015);

	QVERIFY(manager.smartPlaylists().count() == 1);

	auto newManager = SmartPlaylistManager(new PlaylistCreatorMock());
	QVERIFY(newManager.smartPlaylists().count() == manager.smartPlaylists().count());
	QVERIFY(newManager.smartPlaylist(0)->id() == manager.smartPlaylist(0)->id());
	QVERIFY(newManager.smartPlaylist(0)->name() == manager.smartPlaylist(0)->name());

	manager.deletePlaylist(0);
}

void SmartPlaylistManagerTest::testDelete()
{
	auto manager = SmartPlaylistManager(new PlaylistCreatorMock());
	auto smartPlaylist = std::make_shared<SmartPlaylistByYear>(-1, 2003, 2011);

	manager.insertPlaylist(smartPlaylist);
	manager.deletePlaylist(0);

	QVERIFY(manager.smartPlaylists().isEmpty());

	auto newManager = SmartPlaylistManager(new PlaylistCreatorMock());
	QVERIFY(newManager.smartPlaylists().count() == manager.smartPlaylists().count());
}

void SmartPlaylistManagerTest::testSelect()
{
	auto* playlistCreator = new PlaylistCreatorMock();
	auto manager = SmartPlaylistManager(playlistCreator);
	auto smartPlaylist = std::make_shared<SmartPlaylistByYear>(-1, 2003, 2011);

	manager.insertPlaylist(smartPlaylist);
	manager.selectPlaylist(0);

	const auto playlists = playlistCreator->playlists();
	QVERIFY(playlists.count() == 1);
	QVERIFY(playlists[0]->name() == smartPlaylist->name());
}

QTEST_GUILESS_MAIN(SmartPlaylistManagerTest)

#include "SmartPlaylistManagerTest.moc"
