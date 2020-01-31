/* FileOperations.cpp */

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

#include "FileOperations.h"

#include "Components/Library/LocalLibrary.h"
#include "Components/LibraryManagement/LibraryManager.h"

#include "Database/LibraryDatabase.h"
#include "Database/Connector.h"

#include "Utils/Utils.h"
#include "Utils/Algorithm.h"
#include "Utils/FileUtils.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Library/LibraryInfo.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Tagging/Tagging.h"

#include <QDir>
#include <QFile>
#include <QString>
#include <QStringList>

namespace Algorithm=Util::Algorithm;

struct FileOperationThread::Private
{
	QList<StringPair> successful_paths;

	QStringList source_paths;
	QString target;

	FileOperationThread::Mode mode;
	LibraryId target_library_id;

	Private(LibraryId target_library_id, const QStringList& source_paths_in, const QString& target, FileOperationThread::Mode mode) :
		target(target),
		mode(mode),
		target_library_id(target_library_id)
	{
		Algorithm::transform(source_paths_in, this->source_paths, [](const QString& path)
		{
			return Util::File::clean_filename(path);
		});
	}
};

FileOperationThread::FileOperationThread(QObject* parent, LibraryId target_library_id, const QStringList& source_dirs, const QString& target, FileOperationThread::Mode mode) :
	QThread(parent)
{
	m = Pimpl::make<Private>(target_library_id, source_dirs, target, mode);
}

FileOperationThread::~FileOperationThread()
{
	auto* db = DB::Connector::instance();
	db->close_db();
}

LibraryId FileOperationThread::target_library() const
{
	return m->target_library_id;
}

QList<StringPair> FileOperationThread::successful_paths() const
{
	return m->successful_paths;
}

void FileOperationThread::run()
{
	m->successful_paths.clear();
	if(m->source_paths.isEmpty() || m->target.isEmpty()){
		return;
	}

	for(const QString& source_path : Algorithm::AsConst(m->source_paths))
	{
		bool b = false;
		QFileInfo info(source_path);
		QString new_name;
		if(info.isDir())
		{
			switch(m->mode)
			{
				case Mode::Copy:
					b = Util::File::copy_dir(source_path, m->target, new_name);
					break;
				case Mode::Move:
					b = Util::File::move_dir(source_path, m->target, new_name);
					break;
				case Mode::Rename:
					b = Util::File::rename_dir(source_path, m->target);
					new_name = m->target;
					break;
				default: break;
			}
		}

		else if(info.isFile())
		{
			switch(m->mode)
			{
				case Mode::Copy:
					b = Util::File::copy_file(source_path, m->target, new_name);
					break;
				case Mode::Move:
					b = Util::File::move_file(source_path, m->target, new_name);
					break;
				case Mode::Rename:
					b = Util::File::rename_file(source_path, m->target);
					new_name = m->target;
					break;
				default: break;
			}
		}

		if(b)
		{
			m->successful_paths << StringPair(source_path, new_name);
		}
	}
}


struct FileDeleteThread::Private
{
	QStringList paths;

	Private(const QStringList& paths) :
		paths(paths)
	{}
};

FileDeleteThread::FileDeleteThread(QObject* parent, const QStringList& paths) :
	QThread(parent)
{
	m = Pimpl::make<Private>(paths);
}

FileDeleteThread::~FileDeleteThread() = default;

QStringList FileDeleteThread::paths() const
{
	return m->paths;
}

void FileDeleteThread::run()
{
	Util::File::delete_files(m->paths);
}

FileOperations::FileOperations(QObject *parent) :
	QObject(parent)
{}

FileOperations::~FileOperations() = default;

bool FileOperations::rename_path(const QString& path, const QString& new_name)
{
	Library::Info target_info = Library::Manager::instance()->library_info_by_path(new_name);
	LibraryId target_id = target_info.id();

	auto* t = new FileOperationThread(this, target_id, {path}, new_name, FileOperationThread::Rename);
	connect(t, &QThread::started, this, &FileOperations::sig_started);
	connect(t, &QThread::finished, this, &FileOperations::move_thread_finished);

	t->start();

	return true;
}

bool FileOperations::move_paths(const QStringList& paths, const QString& target_dir)
{
	sp_log(Log::Info, this) << "Move " << paths << " to " << target_dir;

	Library::Info target_info = Library::Manager::instance()->library_info_by_path(target_dir);
	LibraryId target_id = target_info.id();

	auto* t = new FileOperationThread(this, target_id, paths, target_dir, FileOperationThread::Move);
	connect(t, &QThread::started, this, &FileOperations::sig_started);
	connect(t, &QThread::finished, this, &FileOperations::move_thread_finished);

	t->start();

	return true;
}

void FileOperations::move_thread_finished()
{
	auto* thread = static_cast<FileOperationThread*>(sender());
	const QList<StringPair> moved_paths = thread->successful_paths();

	auto* db = DB::Connector::instance();
	DB::LibraryDatabase* library_db = db->library_db(-1, db->db_id());

	QMap<QString, QString> path_map;
	for(const StringPair& path : moved_paths)
	{
		path_map[path.first] = path.second;
		sp_log(Log::Debug, this) << "Successfully moved " << path.first << " to " << path.second;
	}

	library_db->renameFilepaths(path_map, thread->target_library());

	thread->deleteLater();
	emit sig_finished();
}

bool FileOperations::copy_paths(const QStringList& paths, const QString& target_dir)
{
	sp_log(Log::Info, this) << "Try to copy " << paths << " to " << target_dir;

	Library::Info target_info = Library::Manager::instance()->library_info_by_path(target_dir);
	LibraryId target_id = target_info.id();

	auto* t = new FileOperationThread(this, target_id, paths, target_dir, FileOperationThread::Copy);
	connect(t, &QThread::started, this, &FileOperations::sig_started);
	connect(t, &QThread::finished, this, &FileOperations::copy_thread_finished);

	t->start();

	return true;
}

void FileOperations::copy_thread_finished()
{
	auto* thread = static_cast<FileOperationThread*>(sender());

	LibraryId target_id = thread->target_library();

	LocalLibrary* library = Library::Manager::instance()->library_instance(target_id);
	if(library) {
		library->reload_library(false, Library::ReloadQuality::Fast);
	}

	thread->deleteLater();
	emit sig_finished();
}

bool FileOperations::delete_paths(const QStringList& paths)
{
	sp_log(Log::Info, this) << "Try to delete " << paths;

	auto* t = new FileDeleteThread(this, paths);
	connect(t, &QThread::started, this, &FileOperations::sig_started);
	connect(t, &QThread::finished, this, &FileOperations::delete_thread_finished);

	t->start();

	return true;
}

void FileOperations::delete_thread_finished()
{
	auto* delete_thread = static_cast<FileDeleteThread*>(sender());

	auto* db = DB::Connector::instance();
	DB::LibraryDatabase* lib_db = db->library_db(-1, 0);

	MetaDataList v_md;
	lib_db->getAllTracksByPaths(delete_thread->paths(), v_md);
	lib_db->deleteTracks(v_md);

	delete_thread->deleteLater();
	emit sig_finished();
}


QStringList FileOperations::supported_tag_replacements()
{
	return QStringList
	{
		"<title>", "<album>", "<artist>", "<year>", "<bitrate>", "<tracknum>", "<disc>"
	};
}

static QString replace_tag(const QString& expression, const MetaData& md)
{
	QString ret(expression);
	ret.replace("<title>", md.title());
	ret.replace("<album>", md.album());
	ret.replace("<artist>", md.artist());
	ret.replace("<year>", QString::number(md.year()));
	ret.replace("<bitrate>", QString::number(md.bitrate() / 1000));

	QString s_track_nr = QString::number(md.track_number());
	if(md.track_number() < 10)
	{
		s_track_nr.prepend("0");
	}

	ret.replace("<tracknum>", s_track_nr);
	ret.replace("<disc>", QString::number(int(md.discnumber())));

	return ret;
}

static QString increment_filename(const QString& filename)
{
	if(!Util::File::exists(filename)){
		return filename;
	}

	QString dir, pure_filename;
	Util::File::split_filename(filename, dir, pure_filename);

	const QString ext = Util::File::get_file_extension(filename);

	for(int i=1; i<1000; i++)
	{
		QString pure_new_name_nr = pure_filename + "-" + QString::number(i);
		QString full_new_name_nr = dir + "/" + pure_new_name_nr + "." + ext;
		if(!Util::File::exists(full_new_name_nr))
		{
			return full_new_name_nr;
		}
	}

	return QString();
}

bool FileOperations::rename_by_expression(const QString& old_name, const QString& expression)
{
	auto* db = DB::Connector::instance();
	DB::LibraryDatabase* library_db = db->library_db(-1, db->db_id());

	MetaData md = library_db->getTrackByPath(Util::File::clean_filename(old_name));
	if(md.id() < 0) {
		Tagging::Utils::getMetaDataOfFile(md);
	}

	const QString pure_filename = replace_tag(expression, md);
	if(pure_filename.isEmpty()) {
		sp_log(Log::Error, this) << "Target filename is empty";
		return false;
	}

	if(pure_filename.contains("<") || pure_filename.contains(">")){
		sp_log(Log::Error, this) << "<, > are not allowed. Maybe an invalid tag was specified?";
		return false;
	}

	const QString dir = Util::File::get_parent_directory(md.filepath());
	const QString ext = Util::File::get_file_extension(md.filepath());

	QString full_new_name = dir + "/" + pure_filename + "." + ext;
	full_new_name = increment_filename(full_new_name);

	if(md.id() < 0) {
		return Util::File::rename_file(full_new_name, old_name);
	}

	else
	{
		bool success = Util::File::rename_file(old_name, full_new_name);
		if(success)
		{
			md.set_filepath(full_new_name);
			success = library_db->updateTrack(md);
			if(!success){
				Util::File::rename_file(full_new_name, old_name);
			}
		}

		return success;
	}
}
