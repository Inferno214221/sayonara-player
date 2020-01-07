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

struct DirectoryCopyThread::Private
{
	QStringList source_dirs;
	QString target_dir;
	LibraryId target_library_id;

	Private(LibraryId target_library_id, const QStringList& source_dirs, const QString& target_dir) :
		source_dirs(source_dirs),
		target_dir(target_dir),
		target_library_id(target_library_id)
	{}
};

DirectoryCopyThread::DirectoryCopyThread(QObject* parent, LibraryId target_library_id, const QStringList& source_dirs, const QString& target_dir) :
	QThread(parent)
{
	m = Pimpl::make<Private>(target_library_id, source_dirs, target_dir);
}

DirectoryCopyThread::~DirectoryCopyThread() {}

LibraryId DirectoryCopyThread::target_library() const
{
	return m->target_library_id;
}

void DirectoryCopyThread::run()
{
	for(const QString& source_dir : Algorithm::AsConst(m->source_dirs))
	{
		Util::File::copy_dir(source_dir, m->target_dir);
	}
}


struct FileCopyThread::Private
{
	QStringList source_files;
	QString target_dir;
	LibraryId target_library_id;

	Private(LibraryId target_library_id, const QStringList& source_files, const QString& target_dir) :
		source_files(source_files),
		target_dir(target_dir),
		target_library_id(target_library_id)
	{}
};


FileCopyThread::FileCopyThread(QObject* parent, LibraryId target_library_id, const QStringList& source_files, const QString& target_dir) :
	QThread(parent)
{
	m = Pimpl::make<Private>(target_library_id, source_files, target_dir);

	this->setObjectName("FileCopyThread" + Util::random_string(4));
}

FileCopyThread::~FileCopyThread() = default;

void FileCopyThread::run()
{
	Util::File::copy_files(m->source_files, m->target_dir);
}

LibraryId FileCopyThread::target_library() const
{
	return m->target_library_id;
}



FileOperations::FileOperations(QObject *parent) :
	QObject(parent)
{}

FileOperations::~FileOperations() = default;

bool FileOperations::copy_dirs(const QStringList& source_dirs, const QString& target_dir)
{
	Library::Info target_info = Library::Manager::instance()->library_info_by_path(target_dir);
	LibraryId target_id = target_info.id();

	DirectoryCopyThread* t = new DirectoryCopyThread(this, target_id, source_dirs, target_dir);
	connect(t, &QThread::started, this, &FileOperations::sig_copy_started);
	connect(t, &QThread::finished, this, &FileOperations::copy_dir_thread_finished);

	t->start();

	return true;
}


void FileOperations::copy_dir_thread_finished()
{
	auto* t = static_cast<DirectoryCopyThread*>(sender());
	LibraryId target_id = -1;
	if(t){
		target_id = t->target_library();
	}

	if(target_id >= 0)
	{
		LocalLibrary* library = Library::Manager::instance()->library_instance(target_id);
		if(library){
			library->reload_library(false, Library::ReloadQuality::Fast);
		}
	}

	emit sig_copy_finished();

	sender()->deleteLater();
}

bool FileOperations::move_dirs(const QStringList& source_dirs, const QString& target_dir)
{
	const QString cleaned_target_dir = Util::File::clean_filename(target_dir);

	sp_log(Log::Debug, this) << "Move files " << source_dirs << " to " << cleaned_target_dir;

	MetaDataList v_md, v_md_to_update;
	auto* db = DB::Connector::instance();
	DB::LibraryDatabase* library_db = db->library_db(-1, db->db_id());
	if(library_db)
	{
		library_db->getAllTracks(v_md);
	}

	for(const QString& source_dir : source_dirs)
	{
		const QString cleaned_source_dir = Util::File::clean_filename(source_dir);

		const QString new_name = Util::File::move_dir(source_dir, cleaned_target_dir);
		if(new_name.isEmpty()){
			continue;
		}

		for(MetaData md : v_md)
		{
			QString filepath = Util::File::clean_filename(md.filepath());

			if(filepath.startsWith(cleaned_source_dir + "/"))
			{
				filepath.replace(cleaned_source_dir, new_name);
				md.set_filepath(filepath);
				v_md_to_update << md;
			}
		}
	}

	library_db->updateTracks(v_md_to_update);

	return true;
}


bool FileOperations::rename_dir(const QString& source_dir, const QString& target_dir)
{
	const QString cleaned_source_dir = Util::File::clean_filename(source_dir);
	const QString cleaned_target_dir = Util::File::clean_filename(target_dir);

	bool success = Util::File::rename_dir(source_dir, cleaned_target_dir);

	auto* db = DB::Connector::instance();
	DB::LibraryDatabase* library_db = db->library_db(-1, db->db_id());

	if(library_db)
	{
		MetaDataList v_md, v_md_to_update;
		library_db->getAllTracks(v_md);

		for(MetaData md : Util::Algorithm::AsConst(v_md))
		{
			QString filepath = Util::File::clean_filename(md.filepath());

			if(filepath.startsWith(cleaned_source_dir + "/"))
			{
				filepath.replace(cleaned_source_dir, cleaned_target_dir);
				md.set_filepath(filepath);
				v_md_to_update << md;
			}
		}

		library_db->updateTracks(v_md_to_update);
	}

	return success;
}

bool FileOperations::move_files(const QStringList& files, const QString& target_dir)
{
	auto* db = DB::Connector::instance();
	DB::LibraryDatabase* library_db = db->library_db(-1, db->db_id());

	MetaDataList v_md_to_update;
	for(const QString& file : files)
	{
		QString cleaned_filename = Util::File::clean_filename(file);
		bool success = Util::File::move_file(cleaned_filename, target_dir);
		if(!success){
			continue;
		}

		MetaData md = library_db->getTrackByPath(cleaned_filename);
		if(md.id() >= 0)
		{
			QDir d(target_dir);
			QString pure_filename = Util::File::get_filename_of_path(cleaned_filename);

			md.set_filepath( d.filePath(pure_filename) );
			v_md_to_update << md;
		}
	}

	library_db->updateTracks(v_md_to_update);
	return true;
}


bool FileOperations::copy_files(const QStringList& files, const QString& target_dir)
{
	Library::Info target_info = Library::Manager::instance()->library_info_by_path(target_dir);
	LibraryId target_id = target_info.id();

	auto* t = new FileCopyThread(this, target_id, files, target_dir);
	connect(t, &QThread::started, this, &FileOperations::sig_copy_started);
	connect(t, &QThread::finished, this, &FileOperations::copy_file_thread_finished);

	t->start();

	return true;
}

bool FileOperations::rename_file(const QString& old_name, const QString& new_name)
{
	bool success = Util::File::rename_file(old_name, new_name);
	if(!success){
		return false;
	}

	auto* db = DB::Connector::instance();
	DB::LibraryDatabase* library_db = db->library_db(-1, db->db_id());

	MetaData md = library_db->getTrackByPath(Util::File::clean_filename(old_name));
	if(md.id() < 0) {
		return Util::File::rename_file(new_name, old_name);
	}

	else
	{
		bool success = Util::File::rename_file(old_name, new_name);
		if(success)
		{
			md.set_filepath(new_name);
			success = library_db->updateTrack(md);
			if(!success){
				Util::File::rename_file(new_name, old_name);
			}
		}

		return success;
	}
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

bool FileOperations::rename_file_by_expression(const QString& old_name, const QString& expression)
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

void FileOperations::copy_file_thread_finished()
{
	auto* t = static_cast<FileCopyThread*>(sender());

	LibraryId target_id = -1;
	if(t){
		target_id = t->target_library();
	}

	if(target_id >= 0)
	{
		LocalLibrary* library = Library::Manager::instance()->library_instance(target_id);
		if(library){
			library->reload_library(false, Library::ReloadQuality::Fast);
		}
	}

	emit sig_copy_finished();

	sender()->deleteLater();
}
