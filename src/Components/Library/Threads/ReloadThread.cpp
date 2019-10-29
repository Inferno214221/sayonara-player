/* ReloadThread.cpp */

/* Copyright (C) 2011-2019 Lucio Carreras
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


/*
 * ReloadThread.cpp
 *
 *  Created on: Jun 19, 2011
 *      Author: Lucio Carreras
 */

#define N_FILES_TO_STORE 500

#include "ReloadThread.h"

#include "Components/Directories/DirectoryReader.h"
#include "Components/Covers/CoverLocation.h"

#include "Database/Connector.h"
#include "Database/Library.h"
#include "Database/LibraryDatabase.h"
#include "Database/CoverConnector.h"

#include "Utils/Tagging/Tagging.h"
#include "Utils/Utils.h"
#include "Utils/FileUtils.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/MetaData/Genre.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Language/Language.h"
#include "Utils/Set.h"

#include <utility>

using Library::ReloadThread;
namespace FileUtils=::Util::File;

struct ReloadThread::Private
{
	QString					library_path;
	MetaDataList			v_md;
	Library::ReloadQuality	quality;
	LibraryId				library_id;

	bool					paused;
	bool					running;
	bool					may_run;

	Private() :
		quality(Library::ReloadQuality::Fast),
		library_id(-1),
		paused(false),
		running(false),
		may_run(true)
	{}
};

ReloadThread::ReloadThread(QObject *parent) :
	QThread(parent),
	DB::ConnectorConsumer()
{
	m = Pimpl::make<Private>();
	m->library_path = GetSetting(Set::Lib_Path);

	this->setObjectName("ReloadThread" + ::Util::random_string(4));
}

ReloadThread::~ReloadThread()
{
	this->stop();
	while(this->isRunning()){
		::Util::sleep_ms(50);
	}
}

static
bool compare_md(const MetaData& md1, const MetaData& md2)
{
	if(md1.genre_ids().count() != md2.genre_ids().count()){
		return false;
	}

	auto it1 = md1.genre_ids().begin();
	auto it2 = md2.genre_ids().begin();

	int count = md1.genre_ids().count();

	for(int i=count - 1; i>=0; i--, it1++, it2++)
	{
		if(*it1 != *it2){
			return false;
		}
	}

	return (md1.title() == md2.title() &&
			md1.album() == md2.album() &&
			md1.artist() == md2.artist() &&
			md1.year() == md2.year() &&
			md1.rating() == md2.rating() &&
			md1.discnumber() == md2.discnumber() &&
			md1.track_number() == md2.track_number() &&
			md1.album_artist() == md2.album_artist() &&
			md1.album_artist_id() == md2.album_artist_id()
	);
}

bool ReloadThread::get_and_save_all_files(const QHash<QString, MetaData>& md_map_lib)
{
	QString library_path = m->library_path;

	if(library_path.isEmpty() || !FileUtils::exists(library_path)) {
		return false;
	}

	DB::Connector* db = db_connector();
	DB::Library* db_library = db->library_connector();

	QDir dir(library_path);

	MetaDataList v_md_to_store;

	sp_log(Log::Develop, this) << "Scanning Library path: " << dir.absolutePath() << "...";
	QStringList files = get_files_recursive(dir);
	sp_log(Log::Develop, this) << "  Found " << files.size() << " files (Not all of them are soundfiles)";

	int n_files = files.size();
	int cur_idx_files=0;

	for(const QString& filepath : files)
	{
		if(!m->may_run){
			return false;
		}

		bool file_was_read = false;
		MetaData md(filepath);
		md.library_id = m->library_id;

		const MetaData& md_lib = md_map_lib[filepath];

		int progress = (cur_idx_files++ * 100) / n_files;
		emit sig_reloading_library(Lang::get(Lang::ReloadLibrary).triplePt(), progress);

		if(md_lib.id >= 0) // found in library
		{
			if(m->quality == Library::ReloadQuality::Fast){
				continue;
			}

			// fetch some metadata and check if we have the same data already in library in the next step
			file_was_read = Tagging::Utils::getMetaDataOfFile(md, Tagging::Quality::Dirty);
			if(!file_was_read) {
				continue;
			}

			// file is already in library
			if( md_lib.duration_ms() > 1000 && md_lib.duration_ms() < 3600000 && compare_md(md, md_lib)){
				continue;
			}
		}

		file_was_read = Tagging::Utils::getMetaDataOfFile(md, Tagging::Quality::Quality);
		if(file_was_read)
		{
			v_md_to_store << md;

			if(v_md_to_store.size() >= N_FILES_TO_STORE)
			{
				store_metadata_block(v_md_to_store);
				v_md_to_store.clear();
			}
		}
	}

	store_metadata_block(v_md_to_store);
	v_md_to_store.clear();

	sp_log(Log::Develop, this) << "Updating album artists... ";
	db_library->add_album_artists();

	sp_log(Log::Develop, this) << "Create indexes... ";
	db_library->create_indexes();

	sp_log(Log::Develop, this) << "Clean up... ";
	db->clean_up();

	return true;
}


void ReloadThread::store_metadata_block(const MetaDataList& v_md)
{
	using StringSet=::Util::Set<QString>;

	DB::Connector* db = db_connector();
	DB::Covers* db_covers = db->cover_connector();
	DB::LibraryDatabase* lib_db = db->library_db(m->library_id, db->db_id());

	{
		sp_log(Log::Develop, this) << N_FILES_TO_STORE << " tracks reached. Commit chunk to DB";
		bool success = lib_db->store_metadata(v_md);
		sp_log(Log::Develop, this) << "  Success? " << success;
	}

	{
		QString status = tr("Looking for covers");
		sp_log(Log::Develop, this) << "Adding Covers...";

		StringSet all_hashes = db_covers->get_all_hashes();

		db->transaction();

		int idx=0;
		for(const MetaData& md : v_md)
		{
			int progress = ((idx++) * 100) / v_md.count();
			emit sig_reloading_library(status, progress);

			Cover::Location cl = Cover::Location::cover_location(md, false);
			if(!cl.is_valid()){
				continue;
			}

			QString hash = cl.hash();
			if(!all_hashes.contains(hash))
			{
				QPixmap cover(cl.preferred_path());
				db_covers->insert_cover(hash, cover);
				all_hashes.insert(hash);
			}
		}

		db->commit();
	}
}


QStringList ReloadThread::get_files_recursive(QDir base_dir)
{
	QStringList ret;
	{
		sp_log(Log::Crazy, this) << "Reading all files from " << base_dir.absolutePath();
		QString parent_dir, pure_dir_name;
		FileUtils::split_filename(base_dir.absolutePath(), parent_dir, pure_dir_name);

		QString message = tr("Reading files") + ": " + pure_dir_name;
		emit sig_reloading_library(message, 0);
	}

	QStringList soundfile_exts = ::Util::soundfile_extensions();
	QStringList sub_dirs = base_dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
	sp_log(Log::Crazy, this) << "  Found " << sub_dirs.size() << " subdirectories";

	for(const QString& dir : sub_dirs)
	{
		bool success = base_dir.cd(dir);

		if(!success){
			continue;
		}

		sp_log(Log::Crazy, this) << "  Jump into subdir " << base_dir.absolutePath();
		ret << get_files_recursive(base_dir);

		base_dir.cdUp();
	}

	QStringList sub_files = base_dir.entryList(soundfile_exts, QDir::Files);
	sp_log(Log::Crazy, this) << "  Found " << sub_files.size() << " audio files in " << base_dir.absolutePath();
	if(sub_files.isEmpty()) {
		return ret;
	}

	QStringList valid_sub_files = process_sub_files(base_dir, sub_files);
	sp_log(Log::Crazy, this) << "  " << valid_sub_files.size() << " of them are valid.";

	ret << valid_sub_files;

	return ret;
}


QStringList ReloadThread::process_sub_files(const QDir& base_dir, const QStringList& sub_files)
{
	QStringList lst;
	for(const QString& filename : sub_files)
	{
		QString abs_path = base_dir.absoluteFilePath(filename);
		QFileInfo info(abs_path);

		if(!info.exists()){
			sp_log(Log::Warning, this) << "File " << abs_path << " does not exist. Skipping...";
			continue;
		}

		if(!info.isFile()){
			sp_log(Log::Warning, this) << "Error: File " << abs_path << " is not a file. Skipping...";
			continue;
		}

		lst << abs_path;
	}

	return lst;
}


void ReloadThread::pause()
{
	m->paused = true;
}

void ReloadThread::goon()
{
	m->paused = false;
}

void ReloadThread::stop()
{
	m->may_run = false;
}

bool ReloadThread::is_running() const
{
	return m->running;
}

void ReloadThread::set_quality(Library::ReloadQuality quality)
{
	m->quality = quality;
}

void ReloadThread::run()
{
	if(m->library_path.isEmpty())
	{
		sp_log(Log::Warning, this) << "No Library path given";
		return;
	}

	if(m->running){
		return;
	}

	DB::Connector* db = db_connector();
	DB::LibraryDatabase* lib_db = db->library_db(m->library_id, 0);

	m->may_run = true;
	m->running = true;
	m->paused = false;

	MetaDataList v_md, v_to_delete, v_md_needs_update;
	QHash<QString, MetaData> v_md_map;

	emit sig_reloading_library(tr("Deleting orphaned tracks") + "...", 0);

	lib_db->deleteInvalidTracks(m->library_path, v_md_needs_update);
	if(!m->may_run){
		return;
	}

	lib_db->store_metadata(v_md_needs_update);
	if(!m->may_run){
		return;
	}

	lib_db->getAllTracks(v_md);

	sp_log(Log::Debug, this) << "Have " << v_md.size() << " tracks already in library";

	// find orphaned tracks in library && delete them
	for(const MetaData& md : v_md)
	{
		if(!FileUtils::check_file(md.filepath()))
		{
			v_to_delete << std::move(md);
		}

		else{
			v_md_map[md.filepath()] = md;
		}

		if(!m->may_run){
			return;
		}
	}

	sp_log(Log::Debug, this) << "  " << v_to_delete.size() << " of them are not on disk anymore";
	lib_db->deleteTracks(v_to_delete);

	if(!m->may_run){
		return;
	}

	get_and_save_all_files(v_md_map);

	m->paused = false;
	m->running = false;
}

void ReloadThread::set_library(LibraryId library_id, const QString& library_path)
{
	m->library_path = library_path;
	m->library_id = library_id;
}

