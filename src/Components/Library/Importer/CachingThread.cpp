/* ImportCachingThread.cpp */

/* Copyright (C) 2011-2020  Lucio Carreras
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

#include "Components/Directories/DirectoryReader.h"

#include "Utils/Utils.h"
#include "Utils/Algorithm.h"
#include "Utils/FileUtils.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Logger/Logger.h"

#include <QDir>
#include <QProcess>

using Library::CachingThread;

namespace Algorithm=Util::Algorithm;

struct CachingThread::Private
{
	QString			library_path;
	QStringList		archive_dirs;
	QStringList		file_list;

	ImportCachePtr	cache=nullptr;

	int				progress;
	bool			cancelled;

	Private(const QStringList& file_list, const QString& library_path) :
		library_path(library_path),
		file_list(file_list),
		progress(0),
		cancelled(false)
	{
		cache = std::shared_ptr<ImportCache>(new ImportCache(library_path));
	}
};

CachingThread::CachingThread(const QStringList& file_list, const QString& library_path, QObject *parent) :
	QThread(parent)
{
	m = Pimpl::make<CachingThread::Private>(file_list, library_path);

	this->setObjectName("CachingThread" + Util::random_string(4));
}

CachingThread::~CachingThread() = default;

bool CachingThread::scan_archive(const QString& temp_dir, const QString& binary, const QStringList& args, const QList<int>& success_codes)
{
#ifndef Q_OS_UNIX
	return false;
#endif

	QDir dir(temp_dir);
	int ret = QProcess::execute(binary, args);
	if(ret < 0)
	{
		sp_log(Log::Error, this) << binary << " not found or crashed";
	}

	else if(!success_codes.contains(ret))
	{
		sp_log(Log::Error, this) << binary << " exited with error " << ret;
		return false;
	}

	else if(ret > 0)
	{
		sp_log(Log::Warning, this) << binary << " exited with warning " << ret;
	}

	QStringList entries = dir.entryList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);
	for(const QString& e : entries)
	{
		QString filename = dir.absoluteFilePath(e);
		if(Util::File::is_dir(filename))
		{
			scan_dir(filename);
		}

		else if(Util::File::is_file(filename))
		{
			add_file(filename, temp_dir);
		}
	}

	return true;
}


QString CachingThread::create_temp_dir()
{
	QDir dir(QDir::tempPath() + "/sayonara/import/" +  Util::random_string(16));
	QString abs_dir = dir.absolutePath();

	bool b = Util::File::create_directories(abs_dir);
	if(!b)
	{
		sp_log(Log::Warning, this) << "Cannot create temp directory " << abs_dir;
		return QString();
	}

	m->archive_dirs << abs_dir;

	return abs_dir;
}


bool CachingThread::scan_rar(const QString& rar_file)
{
#ifndef Q_OS_UNIX
	return false;
#endif

	QString temp_dir = create_temp_dir();
	return scan_archive(temp_dir, "rar", {"x", rar_file, temp_dir});
}

bool CachingThread::scan_zip(const QString& zip_file)
{
#ifndef Q_OS_UNIX
	return false;
#endif

	QString temp_dir = create_temp_dir();
	return scan_archive(temp_dir, "unzip", {zip_file, "-d", temp_dir}, QList<int>{0, 1, 2});
}

bool CachingThread::scan_tgz(const QString& tgz)
{
#ifndef Q_OS_UNIX
	return false;
#endif

	QString temp_dir = create_temp_dir();
	return scan_archive(temp_dir, "tar", {"xzf", tgz, "-C", temp_dir});
}

void CachingThread::scan_dir(const QString& dir)
{
	DirectoryReader dr(QStringList({"*"}));
	QStringList files;

	dr.scan_files_recursive(dir, files);
	sp_log(Log::Crazy, this) << "Found " << files.size() << " files";

	for(const QString& dir_file : Algorithm::AsConst(files))
	{
		add_file(dir_file, dir);
	}
}

void CachingThread::add_file(const QString& file, const QString& relative_dir)
{
	m->cache->add_file(file, relative_dir);
	update_progress();
}

void CachingThread::run()
{
	m->cache->clear();
	m->progress = 0;
	emit sig_progress(0);

	sp_log(Log::Develop, this) << "Read files";

	for(const QString& filename : Algorithm::AsConst(m->file_list))
	{
		if(m->cancelled) {
			m->cache->clear();
			return;
		}

		if(Util::File::is_dir(filename))
		{
			scan_dir(filename);
		}

		else if(Util::File::is_file(filename))
		{
			QString ext = Util::File::get_file_extension(filename);
			if(ext.compare("rar", Qt::CaseInsensitive) == 0)
			{
				bool success = scan_rar(filename);
				if(!success) {
					sp_log(Log::Warning, this) << "Cannot scan rar";
				}
			}

			else if(ext.compare("zip", Qt::CaseInsensitive) == 0)
			{
				bool success = scan_zip(filename);
				if(!success) {
					sp_log(Log::Warning, this) << "Cannot scan zip";
				}
			}

			else if((ext.compare("tar.gz", Qt::CaseInsensitive) == 0) || (ext.compare("tgz", Qt::CaseInsensitive) == 0))
			{
				bool success = scan_tgz(filename);
				if(!success) {
					sp_log(Log::Warning, this) << "Cannot scan zip";
				}
			}

			else
			{
				add_file(filename);
			}
		}
	}

	emit sig_progress(-1);
}

void CachingThread::update_progress()
{
	m->progress++;
	emit sig_progress(m->progress);
}


void CachingThread::change_metadata(const MetaDataList& v_md_old, const MetaDataList& v_md_new)
{
	if(m->cache) {
		m->cache->change_metadata(v_md_old, v_md_new);
	}

	else{
		sp_log(Log::Debug, this) << "Could not change metadata because cache was not created yet";
	}
}

QStringList CachingThread::temporary_files() const
{
	return m->archive_dirs;
}

Library::ImportCachePtr CachingThread::cache() const
{
	return m->cache;
}

void CachingThread::cancel()
{
	m->cancelled = true;
}

bool CachingThread::is_cancelled() const
{
	return m->cancelled;
}

