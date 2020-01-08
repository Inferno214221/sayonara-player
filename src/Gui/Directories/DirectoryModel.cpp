
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

#include "DirectoryModel.h"

#include "Gui/Utils/SearchableWidget/MiniSearcher.h"

#include "Utils/Set.h"
#include "Utils/Algorithm.h"
#include "Utils/FileUtils.h"
#include "Utils/Library/SearchMode.h"
#include "Utils/Library/LibraryInfo.h"
#include "Utils/Library/Filter.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Logger/Logger.h"

#include "Components/LibraryManagement/LibraryManager.h"
#include "Components/Library/LocalLibrary.h"

#include "Database/LibraryDatabase.h"
#include "Database/Connector.h"

#include <QDirIterator>
#include <QPair>

namespace Algorithm=Util::Algorithm;
using StringPair=QPair<QString, QString>;

struct DirectoryModel::Private
{
	Library::Info	library_info;
	QStringList		found_strings;
	int				cur_idx;
	bool			search_only_dirs;

	Util::Set<QString> all_dirs;
	Util::Set<QString> all_files;

	Private()
	{
		search_only_dirs = false;
		cur_idx = -1;
	}
};

DirectoryModel::DirectoryModel(QObject* parent) :
	SearchableModel<QFileSystemModel>(parent)
{
	m = Pimpl::make<Private>();
}

DirectoryModel::~DirectoryModel() = default;

void DirectoryModel::search_only_dirs(bool b)
{
	if(b != m->search_only_dirs){
		m->cur_idx = 0;
	}

	m->search_only_dirs = b;
}

void DirectoryModel::set_library(const Library::Info& info)
{
	m->library_info = info;
	setRootPath(info.path());
}

Library::Info DirectoryModel::library_info() const
{
	return m->library_info;
}

void DirectoryModel::create_file_list(const QString& substr)
{
	m->all_files.clear();
	m->all_dirs.clear();

	Library::Filter filter;
	filter.set_filtertext(substr, search_mode());
	filter.set_mode(Library::Filter::Mode::Filename);

	LibraryId lib_id = m->library_info.id();

	auto* db = DB::Connector::instance();
	DB::LibraryDatabase* library_db = db->library_db(lib_id, 0);

	MetaDataList v_md;
	library_db->getAllTracksBySearchString(filter, v_md);

	for(const MetaData& md : v_md)
	{
		auto [d, f] = Util::File::split_filename(md.filepath());
		m->all_files << md.filepath();
		m->all_dirs << d;
	}
}

QModelIndexList DirectoryModel::search_results(const QString& substr)
{
	QModelIndexList ret;

	m->found_strings.clear();
	m->cur_idx = -1;

	create_file_list(substr);
	if(m->all_files.isEmpty()){
		return QModelIndexList();
	}

	QString cvt_search_string = Library::Utils::convert_search_string(substr, search_mode());

	for(const QString& dir : m->all_dirs)
	{
		QString dir_cvt(dir);
		dir_cvt.remove(m->library_info.path());
		dir_cvt = Library::Utils::convert_search_string(dir_cvt, search_mode());

		if(dir_cvt.contains(cvt_search_string))
		{
			m->found_strings << dir;
		}
	}

	if(!m->search_only_dirs)
	{
		for(const QString& file : m->all_files)
		{
			QString file_cvt(file);
			file_cvt.remove(m->library_info.path());
			file_cvt = Library::Utils::convert_search_string(file_cvt, search_mode());

			if(file_cvt.contains(cvt_search_string))
			{
				auto [d, f] = Util::File::split_filename(file);
				m->found_strings << d;
			}
		}
	}

	QString str;
	if(m->found_strings.size() > 0)
	{
		str = m->found_strings.first();
		m->cur_idx = 0;
	}

	for(const QString& found_str : m->found_strings)
	{
		QModelIndex found_idx = index(found_str);
		ret << found_idx;
	}

	return ret;
}

Qt::ItemFlags DirectoryModel::flags(const QModelIndex& index) const
{
	if(index.isValid()){
		return (QFileSystemModel::flags(index) | Qt::ItemIsDropEnabled);
	}

	return (QFileSystemModel::flags(index) & ~Qt::ItemIsDropEnabled);
}
