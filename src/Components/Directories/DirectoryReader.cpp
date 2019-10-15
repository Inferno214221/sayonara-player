/* DirectoryReader.cpp */

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

#include "DirectoryReader.h"

#include "Database/Connector.h"
#include "Database/LibraryDatabase.h"

#include "Utils/FileUtils.h"
#include "Utils/Utils.h"
#include "Utils/Algorithm.h"
#include "Utils/Tagging/Tagging.h"
#include "Utils/Parser/PlaylistParser.h"
#include "Utils/MetaData/MetaData.h"
#include "Utils/MetaData/MetaDataList.h"

#include <QDirIterator>
#include <QFileInfo>
#include <QStringList>
#include <QDir>

namespace Algorithm=Util::Algorithm;

struct DirectoryReader::Private
{
	QStringList		name_filters;

	Private(const QStringList& filter) :
		name_filters(filter)
	{}
};

DirectoryReader::DirectoryReader(const QStringList& filter)
{
	m = Pimpl::make<Private>(filter);
}

DirectoryReader::DirectoryReader() :
	DirectoryReader(QStringList())
{}

DirectoryReader::~DirectoryReader() = default;

void DirectoryReader::set_filter(const QStringList & filter)
{
	m->name_filters = filter;
}

void DirectoryReader::set_filter(const QString& filter)
{
	m->name_filters.clear();
	m->name_filters << filter;
}


void DirectoryReader::scan_files_recursive(const QDir& base_dir_orig, QStringList& files) const
{
	QDir base_dir(base_dir_orig);

	const QStringList tmp_files = base_dir.entryList(m->name_filters, QDir::Filters(QDir::Files | QDir::NoDotAndDotDot));
	const QStringList dirs = base_dir.entryList(QDir::Filters(QDir::Dirs | QDir::NoDotAndDotDot));

	for(const QString& dir : dirs)
	{
		base_dir.cd(dir);
		scan_files_recursive(base_dir, files);
		base_dir.cdUp();
	}

	for(const QString& filename : tmp_files){
		files << base_dir.absoluteFilePath(filename);
	}
}

void DirectoryReader::scan_files(const QDir& base_dir, QStringList& files) const
{
	const QStringList tmp_files = base_dir.entryList
	(
		m->name_filters,
		QDir::Filters(QDir::Files | QDir::NoDotAndDotDot)
	);

	for(const QString& filename : tmp_files)
	{
		files << base_dir.absoluteFilePath(filename);
	}
}


MetaDataList DirectoryReader::scan_metadata(const QStringList& lst)
{
	MetaDataList v_md;
	QStringList sound_files, playlist_files;

	// fetch sound and playlist files
	QStringList filter;
	filter << Util::soundfile_extensions();
	filter << Util::playlist_extensions();

	set_filter(filter);

	for( const QString& str : lst)
	{
		if(!Util::File::exists(str)) {
			continue;
		}

		if(Util::File::is_dir(str))
		{
			QDir dir(str);
			dir.cd(str);

			QStringList files;
			scan_files_recursive(dir, files);
			for(const QString& file : Algorithm::AsConst(files)){
				if(Util::File::is_soundfile(file)){
					sound_files << file;
				}
			}
		}

		else if(Util::File::is_soundfile(str)){
			sound_files << str;
		}

		else if(Util::File::is_playlistfile(str)) {
			playlist_files << str;
		}
	}

	DB::Connector* db = DB::Connector::instance();
	DB::LibraryDatabase* lib_db = db->library_db(-1, 0);

	lib_db->getMultipleTracksByPath(sound_files, v_md);

	for(auto it=v_md.begin(); it != v_md.end(); it++)
	{
		if( it->id >= 0 )
		{
				continue;
		}

		it->is_extern = true;
		if(!Tagging::Utils::getMetaDataOfFile(*it))
		{
			it->set_title(it->filepath());
			continue;
		}
	}

	for(const QString& playlist_file : Algorithm::AsConst(playlist_files))
	{
		v_md << PlaylistParser::parse_playlist(playlist_file);
	}

	return v_md;
}


QStringList DirectoryReader::find_files_rec(const QDir& dir_orig, const QString& filename)
{
	if(dir_orig.canonicalPath().isEmpty()){
		return QStringList();
	}

	if(filename.isEmpty()){
		return QStringList();
	}

	QDir dir(dir_orig);

	const QStringList dirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
	const QStringList files = dir.entryList(QDir::Files);

	QStringList ret;
	for(const QString& d : dirs)
	{
		if(d.isEmpty()){
			continue;
		}

		if(dir.cd(d)) {
			ret += find_files_rec(dir, filename);
			dir.cdUp();
		}
	}

	for(const QString& file : files)
	{
		if(file.contains(filename)){
			ret += dir.absoluteFilePath(file);
		}
	}

	return ret;
}
