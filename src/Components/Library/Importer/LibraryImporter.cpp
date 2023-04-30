/* LibraryImporter.cpp */

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

#include "LibraryImporter.h"
#include "ImportCache.h"
#include "CacheProcessor.h"
#include "CopyProcessor.h"
#include "Components/Library/LocalLibrary.h"

#include "Components/Tagging/ChangeNotifier.h"

#include "Utils/ArchiveExtractor.h"
#include "Utils/DirectoryReader.h"
#include "Utils/FileUtils.h"
#include "Utils/FileSystem.h"
#include "Utils/Library/LibraryInfo.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Message/Message.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Tagging/TagReader.h"

#include "Database/Connector.h"
#include "Database/LibraryDatabase.h"

#include <QMap>
#include <QDir>

using Library::Importer;
using Library::CachingThread;
using Library::CopyThread;
using Library::ImportCachePtr;

struct Importer::Private
{
	QString sourceDirectory;
	QStringList temporaryFiles;

	LocalLibrary* library = nullptr;
	CachingThread* cachingThread = nullptr;
	CopyThread* copyThread = nullptr;
	ImportCachePtr importCache = nullptr;

	Importer::ImportStatus status;

	Private(LocalLibrary* library) :
		library(library),
		status(Importer::ImportStatus::NoTracks) {}

	void deleteTemporaryFiles()
	{
		Util::File::deleteFiles(temporaryFiles);
	}
};

Importer::Importer(LocalLibrary* library) :
	QObject(library)
{
	m = Pimpl::make<Private>(library);

	auto* cn = Tagging::ChangeNotifier::instance();
	connect(cn, &Tagging::ChangeNotifier::sigMetadataChanged, this, &Importer::metadataChanged);
}

Importer::~Importer() = default;

bool Importer::isRunning() const
{
	return (m->copyThread && m->copyThread->isRunning()) ||
	       (m->cachingThread && m->cachingThread->isRunning());
}

void Importer::importFiles(const QStringList& files, const QString& targetDir)
{
	QStringList filesToBeImported;

	for(const auto& file: files)
	{
		const auto info = m->library->info();

		if(Util::File::isSubdir(file, info.path()) ||
		   Util::File::isSamePath(file, info.path()))
		{
			continue;
		}

		filesToBeImported << files;
	}

	if(filesToBeImported.isEmpty())
	{
		emitStatus(ImportStatus::NoValidTracks);
		return;
	}

	emitStatus(ImportStatus::Caching);

	if(!targetDir.isEmpty())
	{
		emit sigTargetDirectoryChanged(targetDir);
	}

	const auto fileSystem = Util::FileSystem::create();
	auto* thread = CachingThread::create(filesToBeImported,
	                                     m->library->info().path(),
	                                     Tagging::TagReader::create(),
	                                     Util::ArchiveExtractor::create(),
	                                     Util::DirectoryReader::create(fileSystem),
	                                     fileSystem);
	connect(thread, &CachingThread::finished, this, &Importer::cachingThreadFinished);
	connect(thread, &CachingThread::sigCachedFilesChanged, this, &Importer::sigCachedFilesChanged);
	connect(thread, &CachingThread::destroyed, this, [=]() {
		m->cachingThread = nullptr;
	});

	m->cachingThread = thread;
	thread->start();
}

// preload thread has cached everything, but ok button has not been clicked yet
void Importer::cachingThreadFinished()
{
	MetaDataList tracks;
	auto* thread = static_cast<CachingThread*>(sender());

	const auto cachingResult = thread->cacheResult();
	m->temporaryFiles << cachingResult.temporaryFiles;
	m->importCache = cachingResult.cache;

	if(!m->importCache)
	{
		emitStatus(ImportStatus::NoTracks);
	}

	else
	{
		tracks = m->importCache->soundfiles();
	}

	if(tracks.isEmpty() || thread->isCancelled())
	{
		emitStatus(ImportStatus::NoTracks);
	}

	else
	{
		emitStatus(ImportStatus::CachingFinished);
	}

	emit sigMetadataCached(tracks);

	thread->deleteLater();
}

int Importer::cachedFileCount() const
{
	return m->importCache ? m->importCache->soundfiles().count() : 0;
}

// fired if ok was clicked in dialog
void Importer::acceptImport(const QString& targetDir)
{
	emitStatus(ImportStatus::Importing);

	auto* copyThread = new CopyThread(targetDir, m->importCache, Util::FileSystem::create(), this);
	connect(copyThread, &CopyThread::sigProgress, this, &Importer::sigProgress);
	connect(copyThread, &QThread::finished, this, &Importer::copyThreadFinished);
	connect(copyThread, &QThread::destroyed, this, [=]() {
		m->copyThread = nullptr;
	});

	m->copyThread = copyThread;
	copyThread->start();
}

void Importer::copyThreadFinished()
{
	auto* copyThread = static_cast<CopyThread*>(sender());

	reset();

	auto* db = DB::Connector::instance();
	DB::LibraryDatabase* libraryDatabase = db->libraryDatabase(m->library->info().id(), db->databaseId());

	MetaDataList tracks = copyThread->copiedMetadata();
	{ // no tracks were copied or rollback was finished
		if(tracks.isEmpty())
		{
			emitStatus(ImportStatus::NoTracks);
			copyThread->deleteLater();

			return;
		}
	}

	{ // copy was cancelled
		spLog(Log::Debug, this) << "Copy folder thread finished " << m->copyThread->wasCancelled();
		if(copyThread->wasCancelled())
		{
			emitStatus(ImportStatus::Rollback);

			copyThread->setMode(CopyThread::Mode::Rollback);
			copyThread->start();

			return;
		}
	}

	copyThread->deleteLater();

	bool success = libraryDatabase->storeMetadata(tracks);
	// error and success messages
	if(!success)
	{
		emitStatus(ImportStatus::Cancelled);

		QString warning = tr("Cannot import tracks");
		Message::warning(warning);
		return;
	}

	int copiedFilecount = copyThread->copiedFileCount();
	int cacheFilecount = m->importCache->count();

	QString message;
	if(cacheFilecount == copiedFilecount)
	{
		message = tr("All files could be imported");
	}

	else
	{
		message = tr("%1 of %2 files could be imported")
			.arg(copiedFilecount)
			.arg(cacheFilecount);
	}

	emitStatus(ImportStatus::Imported);
	Message::info(message);

	Tagging::ChangeNotifier::instance()->clearChangedMetadata();
}

void Importer::metadataChanged()
{
	auto* cn = Tagging::ChangeNotifier::instance();
	if(m->importCache)
	{
		m->importCache->changeMetadata(cn->changedMetadata());
	}
}

// fired if cancel button was clicked in dialog
bool Importer::cancelImport()
{
	if(m->cachingThread && m->cachingThread->isRunning())
	{
		m->cachingThread->cancel();
		emitStatus(ImportStatus::Rollback);
	}

	else if(m->copyThread && m->copyThread->isRunning())
	{
		m->copyThread->cancel();
		emitStatus(ImportStatus::Rollback);
	}

	return true;
}

void Importer::reset()
{
	cancelImport();
	m->deleteTemporaryFiles();
}

void Importer::emitStatus(Importer::ImportStatus status)
{
	m->status = status;
	emit sigStatusChanged(m->status);
}

Importer::ImportStatus Importer::status() const
{
	return m->status;
}
