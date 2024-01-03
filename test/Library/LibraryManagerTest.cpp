/* LibraryManagerTest.cpp
 *
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

#include "test/Common/SayonaraTest.h"

#include "Components/LibraryManagement/LibraryManager.h"
#include "Components/Playlist/LibraryPlaylistInteractor.h"
#include "Utils/Library/LibraryInfo.h"
#include "Utils/FileUtils.h"

#include <map>
#include <memory>

// access working directory with Test::Base::tempPath("somefile.txt");

class LibraryPlaylistInteractorStub :
	public LibraryPlaylistInteractor
{
	public:
		~LibraryPlaylistInteractorStub() override = default;

		void createPlaylist(const QStringList&, bool) override {}

		void createPlaylist(const MetaDataList&, bool) override {}

		void append(const MetaDataList&) override {}

		void insertAfterCurrentTrack(const MetaDataList&) override {}
};

class LibraryManagerTest :
	public Test::Base
{
	Q_OBJECT

	public:
		LibraryManagerTest() :
			Test::Base("LibraryManagerTest") {}

	private:

		std::shared_ptr<Library::Manager> createLibraryManager()
		{
			auto deleter = [&](Library::Manager* libraryManager) {
				removeAllLibraries(libraryManager);
				delete libraryManager;
			};

			auto* libraryPlaylistInteractor = new LibraryPlaylistInteractorStub();
			auto* libraryManager = Library::Manager::create(libraryPlaylistInteractor);
			
			return std::shared_ptr<Library::Manager>(libraryManager, deleter);
		}

		void removeAllLibraries(Library::Manager* manager)
		{
			auto allLibraries = manager->allLibraries();
			for(const auto& library: allLibraries)
			{
				manager->removeLibrary(library.id());
			}

			QVERIFY(manager->allLibraries().isEmpty());
			QVERIFY(manager->count() == 0);
		}

		void createLibraries(std::shared_ptr<Library::Manager> manager, int count)
		{
			removeAllLibraries(manager.get());

			for(int i = 0; i < count; i++)
			{
				const auto name = QString("Library%1").arg(i);
				const auto dir = Test::Base::tempPath(name);
				manager->addLibrary(name, dir);
			}

			QVERIFY(manager->allLibraries().size() == count);
			QVERIFY(manager->count() == count);
		}

	private slots:
		void addTest();
		void infoTest();
		void removeTest();
		void renameTest();
		void moveTest();
};

void LibraryManagerTest::addTest()
{
	auto manager = createLibraryManager();
	QVERIFY(manager->allLibraries().isEmpty());

	for(int i = 0; i < 3; i++)
	{
		const auto name = QString("Library%1").arg(i);
		const auto dir = Test::Base::tempPath(name);
		const auto id = manager->addLibrary(name, dir);
		QVERIFY(id >= 0);
		QVERIFY(manager->allLibraries().size() == (i + 1));
		QVERIFY(manager->count() == i + 1);
	}

	{ // already available library
		const auto allLibraries = manager->allLibraries();
		const auto library = allLibraries[1];
		const auto id = manager->addLibrary(library.name(), library.path());
		QVERIFY(id == -1);
		QVERIFY(manager->allLibraries().size() == 3);
	}

	{ // empty name
		const auto dir = Test::Base::tempPath("Empty");
		const auto id = manager->addLibrary(QString(), dir);
		QVERIFY(id == -1);
		QVERIFY(manager->allLibraries().size() == 3);
	}

	{ // SubDir of already available library
		const auto dir = Test::Base::tempPath("Library1/Subdir");
		const auto id = manager->addLibrary("SubLib", dir);
		QVERIFY(id == -1);
		QVERIFY(manager->allLibraries().size() == 3);
	}
}

void LibraryManagerTest::infoTest()
{
	auto manager = createLibraryManager();
	QVERIFY(manager->count() == 0);

	for(int i = 0; i < 3; i++)
	{
		const auto name = QString("Library%1").arg(i);
		const auto dir = Test::Base::tempPath(name);
		const auto id = manager->addLibrary(name, dir);

		const auto info = manager->libraryInfo(id);
		QVERIFY(info.name() == name);
		QVERIFY(info.id() == id);
		QVERIFY(Util::File::isSamePath(info.path(), dir));
		QVERIFY(info.valid());
	}
}

void LibraryManagerTest::removeTest()
{
	auto manager = createLibraryManager();
	createLibraries(manager, 3);

	auto allLibraries = manager->allLibraries();
	QVERIFY(allLibraries.size() == 3);

	{ // remove library
		const auto id = allLibraries[1].id();
		const auto success = manager->removeLibrary(id);
		QVERIFY(success);

		allLibraries = manager->allLibraries();
		QVERIFY(allLibraries.size() == 2);
		QVERIFY(allLibraries[0].id() != id);
		QVERIFY(allLibraries[1].id() != id);
	}

	{ // remove invalid id
		const auto success = manager->removeLibrary(-4);
		QVERIFY(!success);
		allLibraries = manager->allLibraries();
		QVERIFY(allLibraries.size() == 2);
	}

	{ // delete library two times
		const auto id = allLibraries[0].id();
		auto success = manager->removeLibrary(id);
		QVERIFY(success);

		allLibraries = manager->allLibraries();
		QVERIFY(allLibraries.size() == 1);
		success = manager->removeLibrary(id);
		QVERIFY(!success);
		QVERIFY(allLibraries.size() == 1);
	}
}

void LibraryManagerTest::renameTest()
{
	auto manager = createLibraryManager();
	createLibraries(manager, 3);

	auto allLibraries = manager->allLibraries();
	QVERIFY(allLibraries.size() == 3);

	{ // rename
		const auto id = allLibraries[0].id();
		manager->renameLibrary(id, "New Name");
		QVERIFY(manager->libraryInfo(id).name() == "New Name");
		allLibraries = manager->allLibraries();
	}

	{ // empty name
		const auto id = allLibraries[1].id();
		const auto oldName = allLibraries[1].name();
		const auto success = manager->renameLibrary(id, QString());
		QVERIFY(!success);
		QVERIFY(manager->libraryInfo(id).name() == oldName);
		allLibraries = manager->allLibraries();
	}

	{ // already existing name
		const auto id = allLibraries[1].id();
		const auto oldName = allLibraries[1].name();
		const auto existingName = allLibraries[0].name();
		const auto success = manager->renameLibrary(id, existingName);
		QVERIFY(!success);
		QVERIFY(manager->libraryInfo(id).name() == oldName);
		allLibraries = manager->allLibraries();
	}
}

void LibraryManagerTest::moveTest()
{
	auto manager = createLibraryManager();

	{ // move forward
		createLibraries(manager, 4);
		const auto oldLibraries = manager->allLibraries();
		manager->moveLibrary(0, 2);
		const auto newLibraries = manager->allLibraries();
		QVERIFY(newLibraries[0].id() == oldLibraries[1].id());
		QVERIFY(newLibraries[1].id() == oldLibraries[2].id());
		QVERIFY(newLibraries[2].id() == oldLibraries[0].id());
		QVERIFY(newLibraries[3].id() == oldLibraries[3].id());
	}

	{ // move to the end
		createLibraries(manager, 4);
		const auto oldLibraries = manager->allLibraries();
		manager->moveLibrary(0, 3);
		const auto newLibraries = manager->allLibraries();
		QVERIFY(newLibraries[0].id() == oldLibraries[1].id());
		QVERIFY(newLibraries[1].id() == oldLibraries[2].id());
		QVERIFY(newLibraries[2].id() == oldLibraries[3].id());
		QVERIFY(newLibraries[3].id() == oldLibraries[0].id());
	}

	{ // move back
		createLibraries(manager, 4);
		const auto oldLibraries = manager->allLibraries();
		const auto success = manager->moveLibrary(2, 0);
		QVERIFY(success);
		const auto newLibraries = manager->allLibraries();
		QVERIFY(newLibraries[0].id() == oldLibraries[2].id());
		QVERIFY(newLibraries[1].id() == oldLibraries[0].id());
		QVERIFY(newLibraries[2].id() == oldLibraries[1].id());
		QVERIFY(newLibraries[3].id() == oldLibraries[3].id());
		removeAllLibraries(manager.get());
		QVERIFY(manager->count() == 0);
	}

	{ // same position
		createLibraries(manager, 4);
		const auto oldLibraries = manager->allLibraries();
		const auto success = manager->moveLibrary(2, 2);
		QVERIFY(success);
		const auto newLibraries = manager->allLibraries();
		QVERIFY(newLibraries[0].id() == oldLibraries[0].id());
		QVERIFY(newLibraries[1].id() == oldLibraries[1].id());
		QVERIFY(newLibraries[2].id() == oldLibraries[2].id());
		QVERIFY(newLibraries[3].id() == oldLibraries[3].id());
		removeAllLibraries(manager.get());
		QVERIFY(manager->count() == 0);
	}

	{ // invalid position (negative)
		createLibraries(manager, 4);
		const auto oldLibraries = manager->allLibraries();
		const auto success = manager->moveLibrary(2, -2);
		QVERIFY(!success);
		const auto newLibraries = manager->allLibraries();
		QVERIFY(newLibraries[0].id() == oldLibraries[0].id());
		QVERIFY(newLibraries[1].id() == oldLibraries[1].id());
		QVERIFY(newLibraries[2].id() == oldLibraries[2].id());
		QVERIFY(newLibraries[3].id() == oldLibraries[3].id());
		removeAllLibraries(manager.get());
		QVERIFY(manager->count() == 0);
	}

	{ // invalid position (too far)
		createLibraries(manager, 4);
		const auto oldLibraries = manager->allLibraries();
		const auto success = manager->moveLibrary(2, 4);
		QVERIFY(!success);
		const auto newLibraries = manager->allLibraries();
		QVERIFY(newLibraries[0].id() == oldLibraries[0].id());
		QVERIFY(newLibraries[1].id() == oldLibraries[1].id());
		QVERIFY(newLibraries[2].id() == oldLibraries[2].id());
		QVERIFY(newLibraries[3].id() == oldLibraries[3].id());
		removeAllLibraries(manager.get());
		QVERIFY(manager->count() == 0);
	}
}

QTEST_GUILESS_MAIN(LibraryManagerTest)

#include "LibraryManagerTest.moc"
