/* ImportCachingThread.cpp */

/* Copyright (C) 2011-2017  Lucio Carreras
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
#include "Utils/FileUtils.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Tagging/Tagging.h"
#include "Utils/Logger/Logger.h"

#include <QDir>
#include <QProcess>

using Library::CachingThread;

struct CachingThread::Private
{
	QString			library_path;
	QStringList		rar_dirs;
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

	void extract_soundfiles()
	{
		sp_log(Log::Develop, this) << "Extract soundfiles";
		const QStringList files = cache->files();

		for(const QString& filename : files)
		{
			if(Util::File::is_soundfile(filename))
			{
				MetaData md(filename);

				bool success = Tagging::Utils::getMetaDataOfFile(md);
				if(success){
					cache->add_soundfile(md);
				}
			}
		}
	}
};

CachingThread::CachingThread(const QStringList& file_list, const QString& library_path, QObject *parent) :
	QThread(parent)
{
	m = Pimpl::make<CachingThread::Private>(file_list, library_path);

	this->setObjectName("CachingThread" + Util::random_string(4));
}

CachingThread::~CachingThread() {}

bool CachingThread::scan_rar(const QString& rar_file)
{
#ifndef Q_OS_UNIX
	return false;
#endif

	static const QString rar_binary = "rar";

	QDir rar_dir(QDir::tempPath() + "/sayonara/import/" +  Util::random_string(16));
	QString rar_path = rar_dir.absolutePath();

	bool b = Util::File::create_directories(rar_path);
	if(!b){
		return false;
	}

	m->rar_dirs << rar_path;

	int ret = QProcess::execute(rar_binary, {"x", rar_file, rar_dir.absolutePath()});
	if(ret < 0){
		sp_log(Log::Warning, this) << "rar not found or crashed";
	}

	else if(ret > 0){
		sp_log(Log::Warning, this) << "rar exited with error " << ret;
		return false;
	}

	QStringList entries = rar_dir.entryList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);
	for(const QString& e : entries)
	{
		QString filename = rar_dir.absoluteFilePath(e);
		if(Util::File::is_dir(filename))
		{
			scan_dir(filename);
		}

		else if(Util::File::is_file(filename))
		{
			add_file(filename, rar_path);
		}
	}

	return true;
}

bool CachingThread::scan_zip(const QString& zip)
{
	return true;
}

void CachingThread::scan_dir(const QString& dir)
{
	DirectoryReader dr(QStringList({"*"}));
	QStringList files;

	dr.scan_files_recursive(dir, files);
	sp_log(Log::Crazy, this) << "Found " << files.size() << " files";

	for(const QString& dir_file : ::Util::AsConst(files))
	{
		add_file(dir_file, dir);
	}
}

void CachingThread::add_file(const QString& file, const QString& relative_dir)
{
	m->cache->add_standard_file(file, relative_dir);
	update_progress();
}

void CachingThread::run()
{
	m->cache->clear();
	m->progress = 0;
	emit sig_progress(0);

	sp_log(Log::Develop, this) << "Read files";

	for(const QString& filename : ::Util::AsConst(m->file_list))
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
				if(!success){
					sp_log(Log::Warning, this) << "Cannot scan rar";
				}
			}

			else {
				add_file(filename);
			}
		}
	}

	m->extract_soundfiles();

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
	return m->rar_dirs;
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

