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
#include "Utils/Utils.h"

#include <QDir>

using Library::CachingThread;

struct CachingThread::Private
{
	QString			library_path;
	ImportCachePtr	cache=nullptr;
	QStringList		file_list;
	bool			cancelled;

	void read_files()
	{
		sp_log(Log::Develop, this) << "Read files";

		DirectoryReader dr;
		dr.set_filter("*");

		for(const QString& filename : ::Util::AsConst(file_list))
		{
			if(cancelled) {
				cache->clear();
				return;
			}

			if(Util::File::is_dir(filename))
			{
				sp_log(Log::Crazy, this) << "Read Directory " << filename;

				QStringList dir_files;
				QDir dir(filename);

				dr.files_in_directory_recursive(dir, dir_files);
				sp_log(Log::Crazy, this) << "Found " << dir_files.size() << " files";

				for(const QString& dir_file : ::Util::AsConst(dir_files))
				{
					cache->add_standard_file(dir_file, filename);
				}
			}

			else {
				cache->add_standard_file(filename);
			}
		}
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
	m = Pimpl::make<CachingThread::Private>();

	m->cache = std::shared_ptr<ImportCache>(new ImportCache(library_path));
	m->library_path = library_path;
	m->file_list = file_list;
	m->cancelled = false;

	this->setObjectName("CachingThread" + Util::random_string(4));
}

CachingThread::~CachingThread() {}

void CachingThread::run()
{
	m->cache->clear();

	m->read_files();
	m->extract_soundfiles();

	emit sig_progress( -1 );
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

