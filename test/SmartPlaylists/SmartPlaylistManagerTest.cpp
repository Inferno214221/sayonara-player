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
#include "Common/FileSystemMock.h"

#include "Components/SmartPlaylists/SmartPlaylistManager.h"
#include "Components/SmartPlaylists/SmartPlaylistCreator.h"
#include "Components/SmartPlaylists/SmartPlaylistByYear.h"
#include "Components/SmartPlaylists/SmartPlaylistByRating.h"

#include "Components/Playlist/Playlist.h"

#include "Interfaces/PlaylistInterface.h"

#include "Utils/Algorithm.h"

// access working directory with Test::Base::tempPath("somefile.txt");

namespace
{
	constexpr const LibraryId libraryId = -1;

	SmartPlaylistPtr playlistByType(const SmartPlaylists::Type type, const SmartPlaylistManager& manager)
	{
		const auto allSmartPlaylists = manager.smartPlaylists();
		const auto index = Util::Algorithm::indexOf(allSmartPlaylists, [t = type](const auto& item) {
			return (item->type() == t);
		});

		return allSmartPlaylists[index];
	}

	QList<int> extractValues(const std::shared_ptr<SmartPlaylist>& smartPlaylist)
	{
		auto values = QList<int> {};
		for(auto i = 0; i < smartPlaylist->count(); i++)
		{
			values << smartPlaylist->value(i);
		}

		return values;
	}

	void deleteAllPlaylists(SmartPlaylistManager& manager)
	{
		const auto allSmartPlaylists = manager.smartPlaylists();
		for(const auto& smartPlaylist: allSmartPlaylists)
		{
			manager.deletePlaylist(Spid(smartPlaylist->id()));
		}

		QVERIFY(manager.smartPlaylists().count() == 0);
	}
}

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
		                   bool /*temporary*/, Playlist::LocalPathPlaylistCreator* /*creator*/) override { return 0; }

		int createPlaylist(const CustomPlaylist& /*customPlaylist*/) override { return 0; }

		int createEmptyPlaylist(bool /*override*/) override { return 0; }

		int createCommandLinePlaylist(const QStringList& /*pathList*/,
		                              Playlist::LocalPathPlaylistCreator* /*creator*/) override
		{
			return 0;
		}

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

[[maybe_unused]] void
SmartPlaylistManagerTest::testInsert() // NOLINT(readability-function-cognitive-complexity,readability-convert-member-functions-to-static)
{
	auto fileSystem = std::make_shared<Test::FileSystemMock>();
	auto manager = SmartPlaylistManager(new PlaylistCreatorMock(), fileSystem);

	const auto smartPlaylists = std::array {
		std::tuple {SmartPlaylists::Type::Year, QList<int> {2000, 2011}, false, 1},
		std::tuple {SmartPlaylists::Type::Rating, QList<int> {1, 4}, true, 2}
	};

	QVERIFY(manager.smartPlaylists().count() == 0);

	for(const auto& [type, values, randomize, expectedCount]: smartPlaylists)
	{
		const auto createdSmartPlaylist =
			SmartPlaylists::createFromType(type, -1, values, randomize, libraryId, fileSystem);
		manager.insertPlaylist(createdSmartPlaylist);

		const auto allSmartPlaylists = manager.smartPlaylists();
		const auto smartPlaylist = playlistByType(type, manager);
		const auto spid = Spid(smartPlaylist->id());

		QVERIFY(allSmartPlaylists.size() == expectedCount);
		QVERIFY(smartPlaylist->name() == createdSmartPlaylist->name());
		QVERIFY(smartPlaylist->id() >= 0);
		QVERIFY(smartPlaylist->isRandomized() == createdSmartPlaylist->isRandomized());

		for(auto i = 0; i < smartPlaylist->count(); i++)
		{
			QVERIFY(manager.smartPlaylist(spid)->value(i) == values[i]);
		}

		auto newManager = SmartPlaylistManager(new PlaylistCreatorMock(), fileSystem);
		QVERIFY(newManager.smartPlaylists().count() == manager.smartPlaylists().count());
		QVERIFY(newManager.smartPlaylist(spid)->id() == manager.smartPlaylist(spid)->id());
		QVERIFY(newManager.smartPlaylist(spid)->name() == manager.smartPlaylist(spid)->name());
		QVERIFY(newManager.smartPlaylist(spid)->isRandomized() == manager.smartPlaylist(spid)->isRandomized());
	}

	deleteAllPlaylists(manager);
}

[[maybe_unused]] void
SmartPlaylistManagerTest::testEdit() // NOLINT(readability-function-cognitive-complexity,readability-convert-member-functions-to-static)
{
	auto fileSystem = std::make_shared<Test::FileSystemMock>();
	auto manager = SmartPlaylistManager(new PlaylistCreatorMock(), fileSystem);

	const auto smartPlaylists = std::array {
		std::tuple {SmartPlaylists::Type::Year, QList<int> {2000, 2011}, true, 1},
		std::tuple {SmartPlaylists::Type::Rating, QList<int> {1, 4}, false, 2}
	};

	for(const auto& [type, values, isRandomized, expectedCount]: smartPlaylists)
	{
		const auto createdSmartPlaylist =
			SmartPlaylists::createFromType(type, -1, values, isRandomized, libraryId, fileSystem);
		manager.insertPlaylist(createdSmartPlaylist);
		QVERIFY(manager.smartPlaylists().count() == expectedCount);

		const auto allSmartPlaylists = manager.smartPlaylists();

		const auto smartPlaylist = playlistByType(type, manager);
		const auto spid = Spid(smartPlaylist->id());

		const auto oldName = smartPlaylist->name();
		const auto oldId = smartPlaylist->id();
		const auto oldValues = extractValues(smartPlaylist);
		const auto wasRandomized = smartPlaylist->isRandomized();

		for(auto i = 0; i < smartPlaylist->count(); i++)
		{
			smartPlaylist->setValue(i, values[i] + 1);
		}

		smartPlaylist->setRandomized(!smartPlaylist->isRandomized());

		manager.updatePlaylist(spid, smartPlaylist);

		QVERIFY(manager.smartPlaylist(spid)->name() != oldName);
		QVERIFY(manager.smartPlaylist(spid)->name() == smartPlaylist->name());
		QVERIFY(manager.smartPlaylist(spid)->id() == oldId);
		QVERIFY(manager.smartPlaylist(spid)->value(0) == oldValues[0] + 1);
		QVERIFY(manager.smartPlaylist(spid)->value(1) == oldValues[1] + 1);
		QVERIFY(manager.smartPlaylist(spid)->isRandomized() == !wasRandomized);

		QVERIFY(manager.smartPlaylists().count() == expectedCount);

		auto newManager = SmartPlaylistManager(new PlaylistCreatorMock(), fileSystem);
		QVERIFY(newManager.smartPlaylists().count() == manager.smartPlaylists().count());
		QVERIFY(newManager.smartPlaylist(spid)->id() == smartPlaylist->id());
		QVERIFY(newManager.smartPlaylist(spid)->name() == smartPlaylist->name());
		QVERIFY(newManager.smartPlaylist(spid)->isRandomized() == smartPlaylist->isRandomized());
	}

	deleteAllPlaylists(manager);
}

[[maybe_unused]] void SmartPlaylistManagerTest::testDelete() // NOLINT(readability-convert-member-functions-to-static)
{
	auto fileSystem = std::make_shared<Test::FileSystemMock>();
	auto manager = SmartPlaylistManager(new PlaylistCreatorMock(), fileSystem);
	auto smartPlaylist =
		std::make_shared<SmartPlaylistByYear>(-1, 2003, 2011, true, libraryId); // NOLINT(readability-magic-numbers)

	QVERIFY(manager.smartPlaylists().count() == 0);
	manager.insertPlaylist(smartPlaylist);
	QVERIFY(manager.smartPlaylists().count() == 1);

	const auto allSmartPlaylists = manager.smartPlaylists();
	const auto spid = Spid(allSmartPlaylists[0]->id());
	manager.deletePlaylist(spid);

	QVERIFY(manager.smartPlaylists().isEmpty());

	auto newManager = SmartPlaylistManager(new PlaylistCreatorMock(), fileSystem);
	QVERIFY(newManager.smartPlaylists().count() == manager.smartPlaylists().count());
}

[[maybe_unused]] void SmartPlaylistManagerTest::testSelect() // NOLINT(readability-convert-member-functions-to-static)
{
	auto fileSystem = std::make_shared<Test::FileSystemMock>();
	auto* playlistCreator = new PlaylistCreatorMock();
	auto manager = SmartPlaylistManager(playlistCreator, fileSystem);

	// NOLINTNEXTLINE(readability-magic-numbers)
	auto smartPlaylist = std::make_shared<SmartPlaylistByYear>(-1, 2003, 2011, true, libraryId);

	manager.insertPlaylist(smartPlaylist);

	const auto allSmartPlaylists = manager.smartPlaylists();
	const auto spid = Spid(allSmartPlaylists[0]->id());
	manager.selectPlaylist(spid);

	const auto playlists = playlistCreator->playlists();
	QVERIFY(playlists.count() == 1);
	QVERIFY(playlists[0]->name() == smartPlaylist->name());
}

QTEST_GUILESS_MAIN(SmartPlaylistManagerTest)

#include "SmartPlaylistManagerTest.moc"
