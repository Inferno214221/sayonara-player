/* CopyFolderThread.cpp */

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

#include "CopyThread.h"
#include "Components/Library/Importer/ImportCache.h"

#include "Utils/Utils.h"
#include "Utils/Algorithm.h"
#include "Utils/FileUtils.h"
#include "Utils/FileSystem.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Logger/Logger.h"

#include <QFile>
#include <QDir>

namespace Algorithm = Util::Algorithm;

namespace
{
	constexpr const auto* ClassName = "Library::CopyThread";

	bool copyFile(const QString& filename, const QString& targetFilename, const Util::FileSystemPtr& fileSystem)
	{
		spLog(Log::Debug, ClassName) << "copy " << filename << " to \n\t" << targetFilename;

		if(fileSystem->exists(targetFilename))
		{
			spLog(Log::Info, ClassName) << "Overwrite " << targetFilename;
			fileSystem->deleteFiles({targetFilename});
		}

		const auto success = fileSystem->copyFile(filename, targetFilename);
		if(!success)
		{
			spLog(Log::Warning, ClassName) << "Copy error";
		}

		return success;
	}

	MetaDataList appendCacheTrack(MetaData cacheTrack, const QString& targetFilename, MetaDataList copiedTracks)
	{
		if(cacheTrack.isValid())
		{
			spLog(Log::Debug, ClassName) << "Set new filename: " << targetFilename;
			cacheTrack.setFilepath(targetFilename);
			copiedTracks << std::move(cacheTrack);
		}

		return copiedTracks;
	}

	QStringList extractFilepaths(const MetaDataList& tracks)
	{
		QStringList result;
		Util::Algorithm::transform(tracks, result, [](const auto& track) {
			return track.filepath();
		});

		return result;
	}
}

using Library::CopyThread;

struct CopyThread::Private
{
	ImportCachePtr importCache;
	Util::FileSystemPtr fileSystem;
	MetaDataList tracks;
	QString targetDir;
	CopyThread::Mode mode {CopyThread::Mode::Copy};
	bool cancelled {false};

	Private(ImportCachePtr importCache, Util::FileSystemPtr fileSystem, QString targetDir) :
		importCache {std::move(importCache)},
		fileSystem {std::move(fileSystem)},
		targetDir {std::move(targetDir)} {}
};

CopyThread::CopyThread(const QString& targetDir, const ImportCachePtr& importCache,
                       const Util::FileSystemPtr& fileSystem, QObject* parent) :
	QThread(parent),
	m {Pimpl::make<Private>(importCache, fileSystem, targetDir)} {}

CopyThread::~CopyThread() = default;

void CopyThread::emitPercent()
{
	const auto percent = (m->tracks.count() * 100) / m->importCache->count();
	emit sigProgress(percent);
}

void CopyThread::copy()
{
	m->tracks.clear();

	const auto files = m->importCache->files();
	for(const auto& filename: files)
	{
		if(m->cancelled)
		{
			return;
		}

		const auto targetFilename = m->importCache->targetFilename(filename, m->targetDir);
		if(targetFilename.isEmpty())
		{
			continue;
		}

		if(!copyFile(filename, targetFilename, m->fileSystem))
		{
			continue;
		}

		m->tracks = appendCacheTrack(m->importCache->metadata(filename), targetFilename, std::move(m->tracks));

		emitPercent();
	}
}

void CopyThread::rollback()
{
	auto copiedPaths = extractFilepaths(m->tracks);
	while(copiedPaths.count() > 0)
	{
		const auto filename = copiedPaths.takeLast();
		m->fileSystem->deleteFiles({filename});

		emitPercent();
	}
}

void CopyThread::run()
{
	if(m->mode == Mode::Copy)
	{
		copy();
	}

	else if(m->mode == Mode::Rollback)
	{
		rollback();
	}

	m->cancelled = false;
}

void CopyThread::cancel() { m->cancelled = true; }

MetaDataList CopyThread::copiedMetadata() const { return m->tracks; }

bool CopyThread::wasCancelled() const { return m->cancelled; }

int CopyThread::copiedFileCount() const { return m->tracks.count(); }

void CopyThread::setMode(CopyThread::Mode mode) { m->mode = mode; }
