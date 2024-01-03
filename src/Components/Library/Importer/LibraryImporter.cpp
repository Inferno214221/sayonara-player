/* LibraryImporter.cpp */

/* Copyright (C) 2011-2024 Michael Lugmair (Lucio Carreras)
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
#include "Database/LibraryDatabase.h"
#include "Utils/Algorithm.h"
#include "Utils/ArchiveExtractor.h"
#include "Utils/DirectoryReader.h"
#include "Utils/FileSystem.h"
#include "Utils/FileUtils.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Message/Message.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Tagging/TagReader.h"

#include <QStringList>
#include <QThread>

namespace
{
	QStringList filterFilesToImport(QStringList files, const QString& libraryPath)
	{
		files.erase(std::remove_if(files.begin(), files.end(), [&](const auto& file) {
			return (Util::File::isSubdir(file, libraryPath) ||
			        Util::File::isSamePath(file, libraryPath));
		}), files.end());

		return files;
	}

	void emitSuccessMessage(const bool success, const int copiedFilecount, const int cacheFilecount)
	{
		if(success)
		{
			const auto message = (cacheFilecount == copiedFilecount)
			                     ? QObject::tr("All files could be imported")
			                     : QObject::tr("%1 of %2 files could be imported")
				                     .arg(copiedFilecount)
				                     .arg(cacheFilecount);

			Message::info(message);
		}

		else
		{
			Message::warning(QObject::tr("Cannot import tracks"));
		}
	}

	bool isProcessorRunning(QObject* processor)
	{
		return processor && processor->thread() && processor->thread()->isRunning();
	}
}

namespace Library
{
	struct Importer::Private
	{
		QString sourceDirectory;
		QStringList temporaryFiles;
		DB::LibraryDatabase* libraryDatabase;
		CopyProcessor* copyProcessor {nullptr};
		CacheProcessor* cacheProcessor {nullptr};
		ImportCachePtr importCache {nullptr};
		Importer::ImportStatus status {Importer::ImportStatus::NoTracks};
		Util::FileSystemPtr fileSystem;
		Tagging::TagReaderPtr tagReader;
		Tagging::ChangeNotifier* taggingChangeNotifier {Tagging::ChangeNotifier::instance()};

		Private(DB::LibraryDatabase* libraryDatabase, Util::FileSystemPtr fileSystem, Tagging::TagReaderPtr tagReader) :
			libraryDatabase {libraryDatabase},
			fileSystem {std::move(fileSystem)},
			tagReader {std::move(tagReader)} {}
	};

	Importer::Importer(DB::LibraryDatabase* libraryDatabase, Util::FileSystemPtr fileSystem,
	                   Tagging::TagReaderPtr tagReader, QObject* parent) :
		QObject(parent),
		m {Pimpl::make<Private>(libraryDatabase, std::move(fileSystem), std::move(tagReader))}
	{
		connect(m->taggingChangeNotifier, &Tagging::ChangeNotifier::sigMetadataChanged, this, [&]() {
			if(m->importCache)
			{
				m->importCache->changeMetadata(m->taggingChangeNotifier->changedMetadata());
			}
		});
	}

	Importer::~Importer() = default;

	void Importer::import(const QString& libraryPath, const QStringList& files, const QString& targetDir)
	{
		emit sigTargetDirectoryChanged(targetDir);

		const auto filesToBeImported = filterFilesToImport(files, libraryPath);
		if(filesToBeImported.isEmpty())
		{
			emitStatus(ImportStatus::NoValidTracks);
			return;
		}

		startCaching(filesToBeImported, libraryPath);
	}

	void Importer::startCaching(const QStringList& files, const QString& libraryPath)
	{
		emitStatus(ImportStatus::Caching);

		auto* thread = new QThread {};
		m->cacheProcessor = CacheProcessor::create(files,
		                                           libraryPath,
		                                           m->tagReader,
		                                           Util::ArchiveExtractor::create(),
		                                           Util::DirectoryReader::create(m->fileSystem, m->tagReader),
		                                           m->fileSystem);

		m->cacheProcessor->moveToThread(thread);

		connect(thread, &QThread::started, m->cacheProcessor, &CacheProcessor::cacheFiles);
		connect(m->cacheProcessor, &CacheProcessor::sigFinished, thread, &QThread::quit);
		connect(m->cacheProcessor, &CacheProcessor::sigFinished, this, &Importer::cachingProcessorFinished);
		connect(thread, &QThread::finished, thread, &QObject::deleteLater);

		thread->start();
	}

	void Importer::cachingProcessorFinished()
	{
		const auto cachingResult = m->cacheProcessor->cacheResult();
		const auto wasCancelled = m->cacheProcessor->wasCancelled();

		m->temporaryFiles = cachingResult.temporaryFiles;
		m->importCache = cachingResult.cache;

		m->cacheProcessor->deleteLater();
		m->cacheProcessor = nullptr;

		if(!m->importCache || wasCancelled)
		{
			emitStatus(ImportStatus::NoTracks);
			return;
		}

		const auto tracks = m->importCache->soundfiles();
		const auto status = tracks.isEmpty()
		                    ? ImportStatus::NoTracks
		                    : ImportStatus::CachingFinished;

		emitStatus(status);
	}

	void Importer::copy(const QString& targetDir)
	{
		emitStatus(ImportStatus::Importing);

		auto* thread = new QThread {};
		m->copyProcessor = CopyProcessor::create(targetDir, m->importCache, m->fileSystem);
		m->copyProcessor->moveToThread(thread);

		connect(thread, &QThread::started, m->copyProcessor, &CopyProcessor::copy);
		connect(m->copyProcessor, &CopyProcessor::sigFinished, thread, &QThread::quit);
		connect(m->copyProcessor, &CopyProcessor::sigFinished, this, &Importer::copyProcessorFinished);
		connect(thread, &QThread::finished, thread, &QObject::deleteLater);

		thread->start();
	}

	void Importer::copyProcessorFinished()
	{
		m->fileSystem->deleteFiles(m->temporaryFiles);

		spLog(Log::Debug, this) << "Copy folder thread finished " << m->copyProcessor->wasCancelled();

		if(!m->copyProcessor->wasCancelled())
		{
			const auto tracks = m->copyProcessor->copiedMetadata();
			storeTracksInLibrary(tracks, m->copyProcessor->copiedFileCount());
			m->copyProcessor->deleteLater();
			m->copyProcessor = nullptr;
		}

		else
		{
			rollback();
		}
	}

	void Importer::rollback()
	{
		emitStatus(ImportStatus::Rollback);

		auto* thread = new QThread {};
		m->copyProcessor->moveToThread(thread);

		connect(thread, &QThread::started, m->copyProcessor, &CopyProcessor::rollback);
		connect(m->copyProcessor, &CopyProcessor::sigFinished, thread, &QThread::quit);
		connect(m->copyProcessor, &CopyProcessor::sigFinished, this, [&]() {
			m->copyProcessor->deleteLater();
			m->copyProcessor = nullptr;
		});
		connect(thread, &QThread::finished, thread, &QObject::deleteLater);

		thread->start();
	}

	void Importer::storeTracksInLibrary(const MetaDataList& tracks, const int copiedFiles)
	{
		if(tracks.isEmpty())
		{
			emitStatus(ImportStatus::NoTracks);
			return;
		}

		const auto success = m->libraryDatabase->storeMetadata(tracks);
		const auto status = success ? ImportStatus::Imported : ImportStatus::Cancelled;

		emitStatus(status);
		emitSuccessMessage(success, copiedFiles, m->importCache->soundFileCount());

		if(success)
		{
			m->taggingChangeNotifier->clearChangedMetadata();
		}
	}

	void Importer::cancelImport()
	{
		if(isProcessorRunning(m->cacheProcessor))
		{
			m->cacheProcessor->cancel();
		}

		else if(isProcessorRunning(m->copyProcessor))
		{
			m->copyProcessor->cancel();
		}
	}

	void Importer::reset()
	{
		cancelImport();
		m->fileSystem->deleteFiles(m->temporaryFiles);
	}

	void Importer::emitStatus(const Importer::ImportStatus status)
	{
		m->status = status;
		emit sigStatusChanged(m->status);
	}

	Importer::ImportStatus Importer::status() const { return m->status; }

	MetaDataList Importer::cachedTracks() const
	{
		return m->importCache ? m->importCache->soundfiles() : MetaDataList {};
	}
}