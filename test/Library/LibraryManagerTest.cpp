#include "SayonaraTest.h"
#include "Components/LibraryManagement/LibraryManager.h"
#include "Utils/Library/LibraryInfo.h"
#include "Utils/FileUtils.h"

#include <map>
// access working directory with Test::Base::tempPath("somefile.txt");

class LibraryManagerTest : 
    public Test::Base
{
    Q_OBJECT

    public:
        LibraryManagerTest() :
            Test::Base("LibraryManagerTest")
        {
    	    removeAllLibraries();
        }

	private:
		void removeAllLibraries()
		{
			auto* manager = Library::Manager::instance();
			auto allLibraries = manager->allLibraries();
			for(const auto& library : allLibraries)
			{
				manager->removeLibrary(library.id());
			}

			QVERIFY(manager->allLibraries().isEmpty());
			QVERIFY(manager->count() == 0);
		}

		void createLibraries(int count)
		{
			removeAllLibraries();

			auto manager = Library::Manager::instance();

			for(int i=0; i<count; i++)
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
	auto manager = Library::Manager::instance();
	QVERIFY(manager->allLibraries().isEmpty());

	for(int i=0; i<3; i++)
	{
		const auto name = QString("Library%1").arg(i);
		const auto dir = Test::Base::tempPath(name);
		const auto id = manager->addLibrary(name, dir);
		QVERIFY(id >= 0);
		QVERIFY(manager->allLibraries().size() == (i+1));
		QVERIFY(manager->count() == i+1);
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

	removeAllLibraries();
}

void LibraryManagerTest::infoTest()
{
	auto manager = Library::Manager::instance();
	QVERIFY(manager->count() == 0);

	for(int i=0; i<3; i++)
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

	removeAllLibraries();
}

void LibraryManagerTest::removeTest()
{
	auto manager = Library::Manager::instance();
	createLibraries(3);

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

	removeAllLibraries();
}

void LibraryManagerTest::renameTest()
{
	auto manager = Library::Manager::instance();
	createLibraries(3);

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

	removeAllLibraries();
}

void LibraryManagerTest::moveTest()
{
	auto manager = Library::Manager::instance();

	{ // move forward
		createLibraries(4);
		const auto oldLibraries = manager->allLibraries();
		manager->moveLibrary(0, 2);
		const auto newLibraries = manager->allLibraries();
		QVERIFY(newLibraries[0].id() == oldLibraries[1].id());
		QVERIFY(newLibraries[1].id() == oldLibraries[2].id());
		QVERIFY(newLibraries[2].id() == oldLibraries[0].id());
		QVERIFY(newLibraries[3].id() == oldLibraries[3].id());
	}

	{ // move to the end
		createLibraries(4);
		const auto oldLibraries = manager->allLibraries();
		manager->moveLibrary(0, 3);
		const auto newLibraries = manager->allLibraries();
		QVERIFY(newLibraries[0].id() == oldLibraries[1].id());
		QVERIFY(newLibraries[1].id() == oldLibraries[2].id());
		QVERIFY(newLibraries[2].id() == oldLibraries[3].id());
		QVERIFY(newLibraries[3].id() == oldLibraries[0].id());
	}

	{ // move back
		createLibraries(4);
		const auto oldLibraries = manager->allLibraries();
		const auto success = manager->moveLibrary(2, 0);
		QVERIFY(success);
		const auto newLibraries = manager->allLibraries();
		QVERIFY(newLibraries[0].id() == oldLibraries[2].id());
		QVERIFY(newLibraries[1].id() == oldLibraries[0].id());
		QVERIFY(newLibraries[2].id() == oldLibraries[1].id());
		QVERIFY(newLibraries[3].id() == oldLibraries[3].id());
		removeAllLibraries();
	}

	{ // same position
		createLibraries(4);
		const auto oldLibraries = manager->allLibraries();
		const auto success = manager->moveLibrary(2, 2);
		QVERIFY(success);
		const auto newLibraries = manager->allLibraries();
		QVERIFY(newLibraries[0].id() == oldLibraries[0].id());
		QVERIFY(newLibraries[1].id() == oldLibraries[1].id());
		QVERIFY(newLibraries[2].id() == oldLibraries[2].id());
		QVERIFY(newLibraries[3].id() == oldLibraries[3].id());
		removeAllLibraries();
	}

	{ // invalid position (negative)
		createLibraries(4);
		const auto oldLibraries = manager->allLibraries();
		const auto success = manager->moveLibrary(2, -2);
		QVERIFY(!success);
		const auto newLibraries = manager->allLibraries();
		QVERIFY(newLibraries[0].id() == oldLibraries[0].id());
		QVERIFY(newLibraries[1].id() == oldLibraries[1].id());
		QVERIFY(newLibraries[2].id() == oldLibraries[2].id());
		QVERIFY(newLibraries[3].id() == oldLibraries[3].id());
		removeAllLibraries();
	}

	{ // invalid position (too far)
		createLibraries(4);
		const auto oldLibraries = manager->allLibraries();
		const auto success = manager->moveLibrary(2, 4);
		QVERIFY(!success);
		const auto newLibraries = manager->allLibraries();
		QVERIFY(newLibraries[0].id() == oldLibraries[0].id());
		QVERIFY(newLibraries[1].id() == oldLibraries[1].id());
		QVERIFY(newLibraries[2].id() == oldLibraries[2].id());
		QVERIFY(newLibraries[3].id() == oldLibraries[3].id());
		removeAllLibraries();
	}
}

QTEST_GUILESS_MAIN(LibraryManagerTest)
#include "LibraryManagerTest.moc"
