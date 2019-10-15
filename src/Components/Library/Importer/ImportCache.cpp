/* ImportCache.cpp */

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

#include "ImportCache.h"
#include "Components/LibraryManagement/LibraryManager.h"
#include "Utils/Library/LibraryInfo.h"
#include "Utils/FileUtils.h"
#include "Utils/MetaData/MetaDataList.h"

#include <QHash>
#include <QString>
#include <QStringList>

using Library::ImportCache;

struct ImportCache::Private
{
	QString					library_path;
	MetaDataList			v_md;
	QHash<QString, MetaData> src_md_map;
	QHash<QString, QString>	src_dst_map;
	QStringList				files;

	Private(const QString& library_path) :
		library_path(library_path)
	{}

	Private(const Private& other) :
		CASSIGN(library_path),
		CASSIGN(v_md),
		CASSIGN(src_md_map),
		CASSIGN(src_dst_map),
		CASSIGN(files)
	{}

	Private& operator=(const Private& other)
	{
		ASSIGN(library_path);
		ASSIGN(v_md);
		ASSIGN(src_md_map);
		ASSIGN(src_dst_map);
		ASSIGN(files);

		return *this;
	}
};

ImportCache::ImportCache(const QString& library_path)
{
	m = Pimpl::make<Private>(library_path);
}

ImportCache::ImportCache(const ImportCache& other)
{
	m = Pimpl::make<Private>(*(other.m));
}

ImportCache::~ImportCache() {}

ImportCache& ImportCache::operator=(const ImportCache& other)
{
	*m = *(other.m);

	return *this;
}

void ImportCache::clear()
{
	m->files.clear();
	m->v_md.clear();
	m->src_dst_map.clear();
}

void ImportCache::add_soundfile(const MetaData& md)
{
	if(md.filepath().isEmpty()){
		return;
	}

	m->v_md << md;
	m->src_md_map[md.filepath()] = md;
}

void ImportCache::add_standard_file(const QString& filename)
{
	add_standard_file(filename, QString());
}

void ImportCache::add_standard_file(const QString& filename, const QString& parent_dir)
{
	if(filename.isEmpty()){
		return;
	}

	m->files << filename;

	QString pure_src_filename = Util::File::get_filename_of_path(filename);
	QString target_subdir;

	if(!parent_dir.isEmpty())
	{
		QString file_dir = Util::File::get_parent_directory(filename);
		QString sub_dir = file_dir.remove(Util::File::get_absolute_filename(parent_dir));
		QString pure_srcdir = Util::File::get_filename_of_path(parent_dir);

		target_subdir = pure_srcdir + "/" + sub_dir + "/";
	}

	m->src_dst_map[filename] = target_subdir + pure_src_filename;
}

QStringList ImportCache::files() const
{
	return m->files;
}

MetaDataList ImportCache::soundfiles() const
{
	return m->v_md;
}

QString ImportCache::target_filename(const QString &src_filename, const QString& target_directory) const
{
	if(m->library_path.isEmpty()){
		return QString();
	}

	return m->library_path + "/" + target_directory + "/" + m->src_dst_map[src_filename];
}

MetaData ImportCache::metadata(const QString& filename) const
{
	return m->src_md_map[filename];
}


void ImportCache::change_metadata(const MetaDataList& v_md_old, const MetaDataList& v_md_new)
{
	Q_UNUSED(v_md_old)

	m->v_md = v_md_new;

	for(const MetaData& md : v_md_new)
	{
		m->src_md_map[md.filepath()] = md;
	}
}
