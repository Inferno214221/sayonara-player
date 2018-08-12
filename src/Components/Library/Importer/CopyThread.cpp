/* CopyFolderThread.cpp */

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

#include "CopyThread.h"
#include "Utils/Utils.h"
#include "Utils/FileUtils.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Logger/Logger.h"

#include <QFile>
#include <QDir>

using Library::CopyThread;

struct CopyThread::Private
{
	MetaDataList	v_md;
	QString			target_dir;
	QStringList		lst_copied_files;
	int				copied_files;

	int				percent;
	bool			cancelled;

	ImportCachePtr		cache=nullptr;
	CopyThread::Mode	mode;

	Private(ImportCachePtr c) : cache(c) {}
};


CopyThread::CopyThread(const QString& target_dir, ImportCachePtr cache, QObject *parent) :
	QThread(parent)
{
	m = Pimpl::make<CopyThread::Private>(cache);
	m->target_dir = target_dir;

	this->setObjectName("CopyThread" + Util::random_string(4));

	clear();
}

CopyThread::~CopyThread() {}

void CopyThread::clear()
{
	m->v_md.clear();
	m->copied_files = 0;
	m->lst_copied_files.clear();
	m->mode = Mode::Copy;
	m->percent = 0;
	m->cancelled = false;
}


void CopyThread::emit_percent(int i, int n)
{
	int percent = (i * 100000) / n;
	m->percent = percent / 1000;

	emit sig_progress(m->percent);
}


void CopyThread::copy()
{
	clear();

	QStringList files = m->cache->files();

	for(const QString& filename : files){
		if(m->cancelled){
			return;
		}

		bool success;
		QString target_filename, target_dir;

		target_filename = m->cache->target_filename(filename, m->target_dir);
		if(target_filename.isEmpty()){
			continue;
		}

		target_dir = Util::File::get_parent_directory(target_filename);

		success = Util::File::create_directories(target_dir);
		if(!success){
			continue;
		}

		sp_log(Log::Debug, this) << "copy " << filename << " to \n\t" << target_filename;

		QFile f(filename);
		success = f.copy(target_filename);

		if(!success){
			continue;
		}

		MetaData md(m->cache->metadata(filename));

		if(!md.filepath().isEmpty()){
			sp_log(Log::Debug, this) << "Set new filename: " << target_filename;
			md.set_filepath(target_filename);
			m->v_md << md;
		}

		m->lst_copied_files << target_filename;
		m->copied_files++;

		emit_percent(m->copied_files, files.size());
	}
}

void CopyThread::rollback()
{
	int n_operations = m->lst_copied_files.size();
	int n_ops_todo = n_operations;

	for(const QString& f : ::Util::AsConst(m->lst_copied_files))
	{
		QFile file(f);
		file.remove();
		int percent = ((n_ops_todo--) * (m->percent * 1000)) / (n_operations);

		emit sig_progress(percent/ 1000);
	}

	m->percent = 0;
	m->copied_files = 0;
	m->lst_copied_files.clear();
}


void CopyThread::run()
{
	m->cancelled = false;
	if(m->mode == Mode::Copy){
		copy();
	}

	else if(m->mode == Mode::Rollback){
		rollback();
	}
}


void CopyThread::cancel()
{
	m->cancelled = true;
}

MetaDataList CopyThread::get_copied_metadata() const
{
	return m->v_md;
}

bool CopyThread::was_cancelled() const
{
	return m->cancelled;
}


int CopyThread::get_n_copied_files() const
{
	return m->copied_files;
}


void CopyThread::set_mode(CopyThread::Mode mode)
{
	m->mode = mode;
}

