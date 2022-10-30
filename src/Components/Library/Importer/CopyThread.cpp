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

#include "Utils/Utils.h"
#include "Utils/Algorithm.h"
#include "Utils/FileUtils.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Logger/Logger.h"

#include <QFile>
#include <QDir>

namespace Algorithm = Util::Algorithm;

using Library::CopyThread;

struct CopyThread::Private
{
	MetaDataList tracks;
	QString targetDir;
	QStringList copiedFiles;
	bool cancelled;

	ImportCachePtr cache = nullptr;
	CopyThread::Mode mode;

	Private(ImportCachePtr c) :
		cache(c) {}
};

CopyThread::CopyThread(const QString& targetDir, ImportCachePtr cache, QObject* parent) :
	QThread(parent)
{
	m = Pimpl::make<CopyThread::Private>(cache);
	m->targetDir = targetDir;

	clear();
}

CopyThread::~CopyThread() = default;

void CopyThread::clear()
{
	m->tracks.clear();
	m->mode = Mode::Copy;
	m->copiedFiles.clear();
	m->cancelled = false;
}

void CopyThread::emitPercent()
{
	int percent = (m->copiedFiles.size() * 100000) / m->cache->count();
	emit sigProgress(percent / 1000);
}

void CopyThread::copy()
{
	clear();

	const QStringList files = m->cache->files();

	for(const QString& filename: files)
	{
		if(m->cancelled)
		{
			return;
		}

		const QString targetFilename = m->cache->targetFilename(filename, m->targetDir);
		if(targetFilename.isEmpty())
		{
			continue;
		}

		const QString targetDir = Util::File::getParentDirectory(targetFilename);
		bool success = Util::File::createDirectories(targetDir);
		if(!success)
		{
			continue;
		}

		spLog(Log::Debug, this) << "copy " << filename << " to \n\t" << targetFilename;
		if(Util::File::exists(targetFilename))
		{
			spLog(Log::Info, this) << "Overwrite " << targetFilename;
			Util::File::deleteFiles({targetFilename});
		}

		success = QFile::copy(filename, targetFilename);
		if(!success)
		{
			spLog(Log::Warning, this) << "Copy error";
			continue;
		}

		m->copiedFiles << targetFilename;

		MetaData md(m->cache->metadata(filename));
		if(!md.filepath().isEmpty())
		{
			spLog(Log::Debug, this) << "Set new filename: " << targetFilename;
			md.setFilepath(targetFilename);
			m->tracks << md;
		}

		emitPercent();
	}
}

void CopyThread::rollback()
{
	int n_operations = m->copiedFiles.count();
	while(m->copiedFiles.size() > 0)
	{
		QString filename = m->copiedFiles.takeLast();
		QFile file(filename);
		file.remove();

		int percent = (m->copiedFiles.size() * 100000) / n_operations;

		emit sigProgress(percent / 1000);
	}
}

void CopyThread::run()
{
	m->cancelled = false;
	if(m->mode == Mode::Copy)
	{
		copy();
	}

	else if(m->mode == Mode::Rollback)
	{
		rollback();
	}
}

void CopyThread::cancel() { m->cancelled = true; }

MetaDataList CopyThread::copiedMetadata() const { return m->tracks; }

bool CopyThread::wasCancelled() const { return m->cancelled; }

int CopyThread::copiedFileCount() const { return m->copiedFiles.count(); }

void CopyThread::setMode(CopyThread::Mode mode) { m->mode = mode; }
