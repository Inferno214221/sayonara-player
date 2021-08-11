/* ImportCachingThread.cpp */

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

#include "CachingThread.h"

#include "Components/Tagging/ChangeNotifier.h"

#include "Utils/Algorithm.h"
#include "Utils/DirectoryReader.h"
#include "Utils/FileUtils.h"
#include "Utils/Logger/Logger.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/StandardPaths.h"
#include "Utils/Utils.h"

#include <QDir>
#include <QProcess>

using Library::CachingThread;

namespace Algorithm = Util::Algorithm;

struct CachingThread::Private
{
	QStringList archiveDirectories;
	QStringList sourceFiles;

	ImportCachePtr cache = nullptr;

	int progress;
	bool cancelled;

	Private(const QStringList& sourceFiles, const QString& libraryPath) :
		sourceFiles(sourceFiles),
		progress(0),
		cancelled(false)
	{
		cache = std::make_shared<ImportCache>(libraryPath);
	}
};

CachingThread::CachingThread(const QStringList& sourceFiles, const QString& libraryPath,
                             QObject* parent) :
	QThread(parent)
{
	m = Pimpl::make<CachingThread::Private>(sourceFiles, libraryPath);
	connect(Tagging::ChangeNotifier::instance(),
	        &Tagging::ChangeNotifier::sigMetadataChanged,
	        this,
	        &CachingThread::metadataChanged);
}

CachingThread::~CachingThread() = default;

bool
CachingThread::scanArchive(const QString& tempDir, const QString& binary, const QStringList& args,
                           const QList<int>& successCodes)
{
#ifndef Q_OS_UNIX
	return false;
#endif

	QDir dir(tempDir);
	int ret = QProcess::execute(binary, args);
	if(ret < 0)
	{
		spLog(Log::Error, this) << binary << " not found or crashed";
	}

	else if(!successCodes.contains(ret))
	{
		spLog(Log::Error, this) << binary << " exited with error " << ret;
		return false;
	}

	else if(ret > 0)
	{
		spLog(Log::Warning, this) << binary << " exited with warning " << ret;
	}

	const auto entries = dir.entryList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);
	for(const auto& entry : entries)
	{
		const auto filename = dir.absoluteFilePath(entry);
		if(Util::File::isDir(filename))
		{
			scanDirectory(filename);
		}

		else if(Util::File::isFile(filename))
		{
			addFile(filename, tempDir);
		}
	}

	return true;
}

QString CachingThread::createTempDirectory()
{
	const auto tempDirectory = QString("%1/%2")
		.arg(Util::tempPath("import"))
		.arg(Util::randomString(16));

	const auto success = Util::File::createDirectories(tempDirectory);
	if(!success)
	{
		spLog(Log::Warning, this) << "Cannot create temp directory " << tempDirectory;
		return QString();
	}

	m->archiveDirectories << tempDirectory;

	return tempDirectory;
}

bool CachingThread::scanRarArchive(const QString& rarFile)
{
#ifndef Q_OS_UNIX
	return false;
#endif

	const auto tempDirectory = createTempDirectory();
	return scanArchive(tempDirectory, "rar", {"x", rarFile, tempDirectory});
}

bool CachingThread::scanZipArchive(const QString& zipFile)
{
#ifndef Q_OS_UNIX
	return false;
#endif

	const auto tempDirectory = createTempDirectory();
	return scanArchive(tempDirectory,
	                   "unzip",
	                   {zipFile, "-d", tempDirectory},
	                   QList<int> {0, 1, 2});
}

bool CachingThread::scanTgzArchive(const QString& tgz)
{
#ifndef Q_OS_UNIX
	return false;
#endif

	const auto tempDirectory = createTempDirectory();
	return scanArchive(tempDirectory, "tar", {"xzf", tgz, "-C", tempDirectory});
}

void CachingThread::scanDirectory(const QString& dir)
{
	const auto files = DirectoryReader::scanFilesRecursively(dir, QStringList{"*"});

	spLog(Log::Crazy, this) << "Found " << files.size() << " files";

	QDir upperDir(dir);
	{
		// Example:
		// dir = /dir/we/want/to/import
		// files:
		//	/dir/we/want/to/import/file1
		//	/dir/we/want/to/import/file2
		//	/dir/we/want/to/import/deeper/file1
		// -> cache:
		// we want the 'import' directory in the target
		// directory, too and not only its contents
		upperDir.cdUp();
	}

	for(const auto& file : files)
	{
		addFile(file, upperDir.absolutePath());
	}
}

void CachingThread::addFile(const QString& file, const QString& relativeDir)
{
	m->cache->addFile(file, relativeDir);
	emit sigCachedFilesChanged();
}

void CachingThread::run()
{
	m->cache->clear();
	m->progress = 0;

	emit sigCachedFilesChanged();

	spLog(Log::Develop, this) << "Read files";

	for(const auto& filename : Algorithm::AsConst(m->sourceFiles))
	{
		if(m->cancelled)
		{
			m->cache->clear();
			return;
		}

		if(Util::File::isDir(filename))
		{
			scanDirectory(filename);
		}

		else if(Util::File::isFile(filename))
		{
			const QString extension = Util::File::getFileExtension(filename);
			if(extension.compare("rar", Qt::CaseInsensitive) == 0)
			{
				const auto success = scanRarArchive(filename);
				if(!success)
				{
					spLog(Log::Warning, this) << "Cannot scan rar";
				}
			}

			else if(extension.compare("zip", Qt::CaseInsensitive) == 0)
			{
				const auto success = scanZipArchive(filename);
				if(!success)
				{
					spLog(Log::Warning, this) << "Cannot scan zip";
				}
			}

			else if((extension.compare("tar.gz", Qt::CaseInsensitive) == 0) ||
			        (extension.compare("tgz", Qt::CaseInsensitive) == 0))
			{
				bool success = scanTgzArchive(filename);
				if(!success)
				{
					spLog(Log::Warning, this) << "Cannot scan zip";
				}
			}

			else
			{
				addFile(filename);
			}
		}
	}
}

void CachingThread::metadataChanged()
{
	auto* cn = Tagging::ChangeNotifier::instance();
	m->cache->changeMetadata(cn->changedMetadata());
}

QStringList CachingThread::temporaryFiles() const
{
	return m->archiveDirectories;
}

int CachingThread::cachedFileCount() const
{
	return m->cache->count();
}

int CachingThread::soundfileCount() const
{
	return m->cache->soundFileCount();
}

Library::ImportCachePtr CachingThread::cache() const
{
	return m->cache;
}

void CachingThread::cancel()
{
	m->cancelled = true;
}

bool CachingThread::isCancelled() const
{
	return m->cancelled;
}
