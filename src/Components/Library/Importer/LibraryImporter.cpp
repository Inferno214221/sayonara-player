/* LibraryImporter.cpp */

/* Copyright (C) 2011-2019  Lucio Carreras
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
#include "CachingThread.h"
#include "CopyThread.h"
#include "Components/Library/LocalLibrary.h"

#include "Components/Tagging/ChangeNotifier.h"

#include "Utils/FileUtils.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Message/Message.h"
#include "Utils/Logger/Logger.h"

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
	QString						src_dir;
	QStringList					temporary_files;

	LocalLibrary*				library=nullptr;
	CachingThread*				cache_thread=nullptr;
	CopyThread*					copy_thread=nullptr;
	ImportCachePtr				import_cache=nullptr;

	DB::Connector*				db=nullptr;

	Importer::ImportStatus		status;

	Private(LocalLibrary* library) :
		library(library),
		db(DB::Connector::instance()),
		status(Importer::ImportStatus::NoTracks)
	{}

	void delete_temporary_files()
	{
		Util::File::delete_files(temporary_files);
	}
};

Importer::Importer(LocalLibrary* library) :
	QObject(library)
{
	m = Pimpl::make<Private>(library);

	Tagging::ChangeNotifier* md_change_notifier = Tagging::ChangeNotifier::instance();
	connect(md_change_notifier, &Tagging::ChangeNotifier::sig_metadata_changed,
			this, &Importer::metadata_changed);
}

Importer::~Importer() {}

void Importer::import_files(const QStringList& files, const QString& target_dir)
{
	emit_status(ImportStatus::Caching);

	if(!target_dir.isEmpty())
	{
		emit sig_target_dir_changed(target_dir);
	}

	CachingThread* thread = new CachingThread(files, m->library->library_path());
	connect(thread, &CachingThread::finished, this, &Importer::caching_thread_finished);
	connect(thread, &CachingThread::sig_progress, this, &Importer::sig_progress_no_percent);
	connect(thread, &CachingThread::destroyed, this, [=]()
	{
		m->cache_thread = nullptr;
	});

	m->cache_thread = thread;
	thread->start();
}


// preload thread has cached everything, but ok button has not been clicked yet
void Importer::caching_thread_finished()
{
	MetaDataList v_md;
	CachingThread* thread = static_cast<CachingThread*>(sender());

	m->temporary_files << thread->temporary_files();
	m->import_cache = thread->cache();

	if(!m->import_cache)
	{
		emit_status(ImportStatus::NoTracks);
	}

	else
	{
		v_md = m->import_cache->soundfiles();
	}

	if(v_md.isEmpty() || thread->is_cancelled())
	{
		emit_status(ImportStatus::NoTracks);
	}

	else
	{
		emit_status(ImportStatus::WaitForUser);
	}

	emit sig_progress_no_percent(-1);
	emit sig_got_metadata(v_md);

	thread->deleteLater();
}


// fired if ok was clicked in dialog
void  Importer::accept_import(const QString& target_dir)
{
	emit_status(ImportStatus::Importing);

	CopyThread* copy_thread = new CopyThread(target_dir, m->import_cache, this);
	connect(copy_thread, &CopyThread::sig_progress, this, &Importer::sig_progress);
	connect(copy_thread, &CopyThread::finished, this, &Importer::copy_thread_finished);
	connect(copy_thread, &CachingThread::destroyed, this, [=]()
	{
		m->copy_thread = nullptr;
	});

	m->copy_thread = copy_thread;
	copy_thread->start();
}


void Importer::copy_thread_finished()
{
	CopyThread* copy_thread = static_cast<CopyThread*>(sender());

	MetaDataList v_md = copy_thread->get_copied_metadata();

	emit_status(ImportStatus::WaitForUser);
	m->delete_temporary_files();

	// no tracks were copied or rollback was finished
	if(v_md.isEmpty()) {
		emit_status(ImportStatus::NoTracks);
		copy_thread->deleteLater();
		return;
	}

	// copy was cancelled
	sp_log(Log::Debug, this) << "Copy folder thread finished " << m->copy_thread->was_cancelled();
	if(copy_thread->was_cancelled())
	{
		copy_thread->set_mode(CopyThread::Mode::Rollback);
		copy_thread->start();

		emit_status(ImportStatus::Rollback);
		return;
	}

	// store to db
	DB::Connector* db = DB::Connector::instance();
	DB::LibraryDatabase* lib_db = db->library_db(m->library->library_id(), db->db_id());

	bool success = lib_db->store_metadata(v_md);
	int n_files_copied = copy_thread->get_n_copied_files();
	int n_files_to_copy = m->import_cache->files().size();

	// error and success messages
	if(success)
	{
		m->db->clean_up();

		QString str = "";
		if(n_files_to_copy == n_files_copied) {
			str =  tr("All files could be imported");
		}

		else {
			str = tr("%1 of %2 files could be imported")
					.arg(n_files_copied)
					.arg(n_files_to_copy);
		}

		Message::info(str);

		emit_status(ImportStatus::Imported);

		Tagging::ChangeNotifier::instance()->change_metadata(MetaDataList(), MetaDataList());
	}

	else {
		QString warning = tr("Cannot import tracks");
		Message::warning(warning);
	}

	copy_thread->deleteLater();
}


void Importer::metadata_changed()
{
	auto changed_metadata = Tagging::ChangeNotifier::instance()->changed_metadata();

	if(m->cache_thread){
		m->cache_thread->change_metadata(changed_metadata.first, changed_metadata.second);
	}
}


// fired if cancel button was clicked in dialog
void Importer::cancel_import()
{
	emit_status(ImportStatus::Cancelled);

	if(m->cache_thread && m->cache_thread->isRunning()){
		m->cache_thread->cancel();
	}

	else if(m->copy_thread && m->copy_thread->isRunning()){
		m->copy_thread->cancel();
	}
}

void Importer::reset()
{
	cancel_import();
	m->delete_temporary_files();
}

void Importer::emit_status(Importer::ImportStatus status)
{
	m->status = status;
	emit sig_status_changed(m->status);
}

Importer::ImportStatus Importer::status() const
{
	return m->status;
}
