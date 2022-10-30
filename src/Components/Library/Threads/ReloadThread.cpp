/* ReloadThread.cpp */

/* Copyright (C) 2011-2020 Michael Lugmair (Lucio Carreras)
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


/*
 * ReloadThread.cpp
 *
 *  Created on: Jun 19, 2011
 *      Author: Michael Lugmair (Lucio Carreras)
 */

#define N_FILES_TO_STORE 500

#include "ReloadThread.h"

#include "Components/Covers/CoverLocation.h"

#include "Database/Connector.h"
#include "Database/Library.h"
#include "Database/LibraryDatabase.h"
#include "Database/CoverConnector.h"

#include "Utils/Tagging/Tagging.h"
#include "Utils/Tagging/TaggingCover.h"
#include "Utils/Utils.h"
#include "Utils/DirectoryReader.h"
#include "Utils/FileUtils.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/MetaData/Genre.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Language/Language.h"
#include "Utils/Set.h"

#include <utility>

using Library::ReloadThread;
namespace FileUtils = ::Util::File;

struct ReloadThread::Private
{
	QString libraryPath;
	MetaDataList tracks;
	Library::ReloadQuality quality;
	LibraryId libraryId;

	bool paused;
	bool running;
	bool mayRun;

	Private() :
		quality(Library::ReloadQuality::Fast),
		libraryId(-1),
		paused(false),
		running(false),
		mayRun(true) {}
};

ReloadThread::ReloadThread(QObject* parent) :
	QThread(parent)
{
	m = Pimpl::make<Private>();
	m->libraryPath = GetSetting(Set::Lib_Path);
}

ReloadThread::~ReloadThread()
{
	this->stop();
	while(this->isRunning())
	{
		::Util::sleepMs(50);
	}
}

static
bool compare_md(const MetaData& md1, const MetaData& md2)
{
	if(md1.genreIds().count() != md2.genreIds().count())
	{
		return false;
	}

	auto it1 = md1.genreIds().begin();
	auto it2 = md2.genreIds().begin();

	int count = md1.genreIds().count();

	for(int i = count - 1; i >= 0; i--, it1++, it2++)
	{
		if(*it1 != *it2)
		{
			return false;
		}
	}

	return (md1.title() == md2.title() &&
	        md1.album() == md2.album() &&
	        md1.artist() == md2.artist() &&
	        md1.year() == md2.year() &&
	        md1.rating() == md2.rating() &&
	        md1.discnumber() == md2.discnumber() &&
	        md1.trackNumber() == md2.trackNumber() &&
	        md1.albumArtist() == md2.albumArtist() &&
	        md1.albumArtistId() == md2.albumArtistId()
	);
}

bool ReloadThread::getAndSaveAllFiles(const QHash<QString, MetaData>& pathMetadataMap)
{
	const QString libraryPath = m->libraryPath;
	if(libraryPath.isEmpty() || !FileUtils::exists(libraryPath))
	{
		return false;
	}

	auto* db = DB::Connector::instance();
	DB::Library* libraryDatabase = db->libraryConnector();

	QDir dir(libraryPath);

	MetaDataList tracksToStore;

	spLog(Log::Develop, this) << "Scanning Library path: " << dir.absolutePath() << "...";
	QStringList files = getFilesRecursive(dir);
	spLog(Log::Develop, this) << "  Found " << files.size() << " files (Not all of them are soundfiles)";

	int fileCount = files.size();
	int currentFileIndex = 0;

	for(const QString& filepath: files)
	{
		if(!m->mayRun)
		{
			return false;
		}

		bool fileWasRead = false;
		MetaData md(filepath);
		md.setLibraryid(m->libraryId);

		const MetaData& md_lib = pathMetadataMap[filepath];

		int progress = (currentFileIndex++ * 100) / fileCount;
		emit sigReloadingLibrary(Lang::get(Lang::ReloadLibrary).triplePt(), progress);

		if(md_lib.id() >= 0) // found in library
		{
			if(m->quality == Library::ReloadQuality::Fast)
			{
				continue;
			}

			// fetch some metadata and check if we have the same data already in library in the next step
			fileWasRead = Tagging::Utils::getMetaDataOfFile(md, Tagging::Quality::Dirty);
			if(!fileWasRead)
			{
				continue;
			}

			// file is already in library
			if(md_lib.durationMs() > 1000 && md_lib.durationMs() < 3600000 && compare_md(md, md_lib))
			{
				continue;
			}
		}

		fileWasRead = Tagging::Utils::getMetaDataOfFile(md, Tagging::Quality::Quality);
		if(fileWasRead)
		{
			tracksToStore << md;

			if(tracksToStore.size() >= N_FILES_TO_STORE)
			{
				storeMetadataBlock(tracksToStore);
				tracksToStore.clear();
			}
		}
	}

	storeMetadataBlock(tracksToStore);
	tracksToStore.clear();

	spLog(Log::Develop, this) << "Updating album artists... ";
	libraryDatabase->addAlbumArtists();

	spLog(Log::Develop, this) << "Create indexes... ";
	libraryDatabase->createIndexes();

	spLog(Log::Develop, this) << "Clean up... ";

	return true;
}

void ReloadThread::storeMetadataBlock(const MetaDataList& v_md)
{
	using StringSet = ::Util::Set<QString>;

	auto* db = DB::Connector::instance();
	DB::Covers* coverDatabase = db->coverConnector();
	DB::LibraryDatabase* libraryDatabase = db->libraryDatabase(m->libraryId, db->databaseId());

	{
		spLog(Log::Develop, this) << N_FILES_TO_STORE << " tracks reached. Commit chunk to DB";
		bool success = libraryDatabase->storeMetadata(v_md);
		spLog(Log::Develop, this) << "  Success? " << success;
	}

	{
		QString status = tr("Looking for covers");
		spLog(Log::Develop, this) << "Adding Covers...";

		StringSet all_hashes = coverDatabase->getAllHashes();

		db->transaction();

		int idx = 0;
		for(const MetaData& md: v_md)
		{
			int progress = ((idx++) * 100) / v_md.count();
			emit sigReloadingLibrary(status, progress);

			Cover::Location cl = Cover::Location::coverLocation(md);
			if(!cl.isValid())
			{
				continue;
			}

			const QString hash = cl.hash();
			if(!all_hashes.contains(hash))
			{
				const QPixmap cover(cl.preferredPath());
				if(!cover.isNull())
				{
					coverDatabase->insertCover(hash, cover);
					all_hashes.insert(hash);
				}
			}
		}

		db->commit();
	}
}

QStringList ReloadThread::getFilesRecursive(QDir baseDirectory)
{
	QStringList ret;
	{
		spLog(Log::Crazy, this) << "Reading all files from " << baseDirectory.absolutePath();
		QString parent_dir, pure_dir_name;
		FileUtils::splitFilename(baseDirectory.absolutePath(), parent_dir, pure_dir_name);

		QString message = tr("Reading files") + ": " + pure_dir_name;
		emit sigReloadingLibrary(message, 0);
	}

	QStringList soundfileExtensions = ::Util::soundfileExtensions();
	QStringList subDirectories = baseDirectory.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
	spLog(Log::Crazy, this) << "  Found " << subDirectories.size() << " subdirectories";

	for(const QString& dir: subDirectories)
	{
		bool success = baseDirectory.cd(dir);

		if(!success)
		{
			continue;
		}

		spLog(Log::Crazy, this) << "  Jump into subdir " << baseDirectory.absolutePath();
		ret << getFilesRecursive(baseDirectory);

		baseDirectory.cdUp();
	}

	QStringList subFiles = baseDirectory.entryList(soundfileExtensions, QDir::Files);
	spLog(Log::Crazy, this) << "  Found " << subFiles.size() << " audio files in " << baseDirectory.absolutePath();
	if(subFiles.isEmpty())
	{
		return ret;
	}

	QStringList validSubFiles = filterValidFiles(baseDirectory, subFiles);
	spLog(Log::Crazy, this) << "  " << validSubFiles.size() << " of them are valid.";

	ret << validSubFiles;

	return ret;
}

QStringList ReloadThread::filterValidFiles(const QDir& baseDir, const QStringList& sub_files)
{
	QStringList lst;
	for(const QString& filename: sub_files)
	{
		QString absolutePath = baseDir.absoluteFilePath(filename);
		QFileInfo info(absolutePath);

		if(!info.exists())
		{
			spLog(Log::Warning, this) << "File " << absolutePath << " does not exist. Skipping...";
			continue;
		}

		if(!info.isFile())
		{
			spLog(Log::Warning, this) << "Error: File " << absolutePath << " is not a file. Skipping...";
			continue;
		}

		lst << absolutePath;
	}

	return lst;
}

void ReloadThread::pause()
{
	m->paused = true;
}

void ReloadThread::goon()
{
	m->paused = false;
}

void ReloadThread::stop()
{
	m->mayRun = false;
}

bool ReloadThread::isRunning() const
{
	return m->running;
}

void ReloadThread::setQuality(Library::ReloadQuality quality)
{
	m->quality = quality;
}

void ReloadThread::run()
{
	if(m->libraryPath.isEmpty())
	{
		spLog(Log::Warning, this) << "No Library path given";
		return;
	}

	if(m->running)
	{
		return;
	}

	auto* db = DB::Connector::instance();
	DB::LibraryDatabase* libraryDatabase = db->libraryDatabase(m->libraryId, 0);

	m->mayRun = true;
	m->running = true;
	m->paused = false;

	MetaDataList tracks, tracksToDelete, tracksToUpdate;
	QHash<QString, MetaData> v_md_map;

	emit sigReloadingLibrary(tr("Deleting orphaned tracks") + "...", 0);

	libraryDatabase->deleteInvalidTracks(m->libraryPath, tracksToUpdate);
	if(!m->mayRun)
	{
		return;
	}

	libraryDatabase->storeMetadata(tracksToUpdate);
	if(!m->mayRun)
	{
		return;
	}

	libraryDatabase->getAllTracks(tracks);

	spLog(Log::Debug, this) << "Have " << tracks.size() << " tracks already in library";

	// find orphaned tracks in library && delete them
	for(const MetaData& md: tracks)
	{
		if(!FileUtils::checkFile(md.filepath()))
		{
			tracksToDelete << std::move(md);
		}

		else
		{
			v_md_map[md.filepath()] = md;
		}

		if(!m->mayRun)
		{
			return;
		}
	}

	spLog(Log::Debug, this) << "  " << tracksToDelete.size() << " of them are not on disk anymore";
	libraryDatabase->deleteTracks(tracksToDelete.trackIds());

	if(!m->mayRun)
	{
		return;
	}

	getAndSaveAllFiles(v_md_map);

	m->paused = false;
	m->running = false;
}

void ReloadThread::setLibrary(LibraryId libraryId, const QString& libraryPath)
{
	m->libraryPath = libraryPath;
	m->libraryId = libraryId;
}

