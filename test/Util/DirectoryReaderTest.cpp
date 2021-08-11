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
 
#include "SayonaraTest.h"

#include "Database/TestTracks.h"

#include "Utils/DirectoryReader.h"
#include "Utils/FileUtils.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Tagging/Tagging.h"
#include "Utils/Algorithm.h"

// access working directory with Test::Base::tempPath("somefile.txt");

namespace
{
	void createFileStructure(const QString& basePath)
	{
		auto success = Util::File::createDirectories(QString("%1/path/to/somewhere/else").arg(basePath));
		success &= Util::File::createDirectories(QString("%1/path/to/another/dir").arg(basePath));

		if(!success)
		{
			throw "Could not create directories";
		}
	}

	void setPermissions(const QString& path)
	{
		const auto permission = (
			QFileDevice::Permission::ReadOwner |
			QFileDevice::Permission::ReadGroup |
			QFileDevice::Permission::ReadOther |
			QFileDevice::Permission::ReadUser |
			QFileDevice::Permission::WriteOwner |
			QFileDevice::Permission::WriteGroup |
			QFileDevice::Permission::WriteOther |
			QFileDevice::Permission::WriteUser
		);

		QFile::setPermissions(path, permission);
	}

	QString createMp3File(const QString& path)
	{
		QString newName;

		auto success = Util::File::copyFile(":/test/mp3test.mp3",  path, newName);
		if(!success)
		{
			throw "could not create mp3 file";
		}

		setPermissions(newName);
		return newName;
	}

	QString createOggFile(const QString& path)
	{
		QString newName;

		auto success = Util::File::copyFile(":/test/oggtest.ogg",  path, newName);
		if(!success)
		{
			throw "could not create ogg file";
		}

		setPermissions(newName);
		return newName;
	}

	QStringList createTrackFiles(const QString& basePath)
	{
		createFileStructure(basePath);

		const auto tracks = Test::createTracks();

		QStringList names;
		names << createMp3File(QString("%1/path").arg(basePath));
		names << createMp3File(QString("%1/path/to").arg(basePath));
		names << createMp3File(QString("%1/path/to/somewhere").arg(basePath));
		names << createMp3File(QString("%1/path/to/somewhere/else").arg(basePath));
		names << createMp3File(QString("%1/path/to/another").arg(basePath));
		names << createMp3File(QString("%1/path/to/another/dir").arg(basePath));

		names << createOggFile(QString("%1/path").arg(basePath));
		names << createOggFile(QString("%1/path/to").arg(basePath));
		names << createOggFile(QString("%1/path/to/somewhere").arg(basePath));
		names << createOggFile(QString("%1/path/to/somewhere/else").arg(basePath));
		names << createOggFile(QString("%1/path/to/another").arg(basePath));
		names << createOggFile(QString("%1/path/to/another/dir").arg(basePath));

		for(int i=0; i<names.size(); i++)
		{
			auto track = tracks[i];
			track.setFilepath(names[i]);

			const auto success = Tagging::Utils::setMetaDataOfFile(track);
			if(!success)
			{
				throw "Could not set metadata";
			}
		}

		return names;
	}
}

class DirectoryReaderTest : 
    public Test::Base
{
    Q_OBJECT

    public:
        DirectoryReaderTest() :
            Test::Base("DirectoryReaderTest")
        {
	        m_names = createTrackFiles(Test::Base::tempPath());
        }

    private slots:
        void testScanFilesInDirectory();
        void testScanRecursively();
        void testScanMetadata();

	private:
		QStringList m_names;
};

void DirectoryReaderTest::testScanFilesInDirectory()
{
	const auto baseDir = Test::Base::tempPath();

	{
		const auto dir = QString("%1/path").arg(baseDir);
		const auto files = DirectoryReader::scanFilesInDirectory(dir);
		QVERIFY(files.count() == 2);

		const auto hasMp3 = Util::Algorithm::contains(files, [](const auto& filename) {
			return Util::File::getFilenameOfPath(filename) == "mp3test.mp3";
		});
		QVERIFY(hasMp3);

		const auto hasOgg = Util::Algorithm::contains(files, [](const auto& filename) {
			return Util::File::getFilenameOfPath(filename) == "oggtest.ogg";
		});
		QVERIFY(hasOgg);
	}

	{
		const auto dir = QString("%1/path").arg(baseDir);
		const auto files = DirectoryReader::scanFilesInDirectory(dir, QStringList() << "*.mp3");
		QVERIFY(files.count() == 1);
		QVERIFY(files[0] == QString("%1/mp3test.mp3").arg(dir));
	}

	{
		const auto dir = QString("%1/path").arg(baseDir);
		const auto files = DirectoryReader::scanFilesInDirectory(dir, QStringList() << "*.ogg");
		QVERIFY(files.count() == 1);
		QVERIFY(files[0] == QString("%1/oggtest.ogg").arg(dir));
	}

	{
		const auto dir = QString("%1/path").arg(baseDir);
		const auto files = DirectoryReader::scanFilesInDirectory(dir, QStringList() << "*.m4a");
		QVERIFY(files.count() == 0);
	}
}

void DirectoryReaderTest::testScanRecursively()
{
	const auto baseDir = Test::Base::tempPath();

	{ // there's some db file in the directory, so files > m_names
		const auto files = DirectoryReader::scanFilesRecursively(baseDir);
		QVERIFY(files.count() > m_names.count());
		for(const auto& name : m_names)
		{
			QVERIFY(files.contains(name));
		}
	}

	{
		const auto files = DirectoryReader::scanFilesRecursively(baseDir, QStringList() << "*.mp3" << "*.ogg");
		QVERIFY(files.count() == m_names.count());
		for(const auto& name : m_names)
		{
			QVERIFY(files.contains(name));
		}
	}

	{
		const auto files = DirectoryReader::scanFilesRecursively(baseDir, QStringList() << "*.ogg");
		QVERIFY(files.count() == 6);
		for(const auto& file : files)
		{
			QVERIFY(m_names.contains(file));
		}
	}
}

void DirectoryReaderTest::testScanMetadata()
{
	const auto tracks = DirectoryReader::scanMetadata(m_names);
	const auto generatedTracks = Test::createTracks();

	for(const auto& track : tracks)
	{
		Util::Algorithm::contains(generatedTracks, [&](const auto& generatedTrack){
			return (track.title() == generatedTrack.title());
		});
	}
}

QTEST_GUILESS_MAIN(DirectoryReaderTest)
#include "DirectoryReaderTest.moc"
