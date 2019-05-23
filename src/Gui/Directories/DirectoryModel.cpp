
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

#include "DirectoryModel.h"

#include "Gui/Utils/SearchableWidget/MiniSearcher.h"

#include "Utils/Set.h"
#include "Utils/Utils.h"
#include "Utils/FileUtils.h"
#include "Utils/Library/SearchMode.h"
#include "Utils/Library/LibraryInfo.h"
#include "Utils/Library/Filter.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Logger/Logger.h"

#include "Components/Library/LibraryManager.h"
#include "Components/Library/LocalLibrary.h"

#include "Database/LibraryDatabase.h"
#include "Database/Connector.h"

#include <QDirIterator>
#include <QPair>


using StringPair=QPair<QString, QString>;

struct DirectoryModel::Private
{
	QStringList	found_strings;
	int			cur_idx;
	bool		search_only_dirs;

	QList<StringPair> all_dirs;
	QList<StringPair> all_files;

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

DirectoryModel::~DirectoryModel() {}

void DirectoryModel::search_only_dirs(bool b)
{
	if(b != m->search_only_dirs){
		m->cur_idx = 0;
	}

	m->search_only_dirs = b;
}


void DirectoryModel::create_file_list(const QString& substr)
{
	m->all_files.clear();
	m->all_dirs.clear();

	Library::Filter filter;
	filter.set_filtertext(substr, search_mode());
	filter.set_mode(Library::Filter::Mode::Filename);

	DB::Connector* db = DB::Connector::instance();
	DB::LibraryDatabases library_dbs = db->library_dbs();

	for(DB::LibraryDatabase* lib_db : library_dbs)
	{
		LibraryId lib_id = lib_db->library_id();
		if(lib_id < 0){
			continue;
		}

		Library::Info info = Library::Manager::instance()->library_info(lib_id);

		MetaDataList v_md;
		lib_db->getAllTracksBySearchString(filter, v_md);

		for(const MetaData& md : v_md)
		{
			QString sym_filepath = md.filepath();
			sym_filepath.replace(info.path(), info.symlink_path());

			m->all_files << StringPair(
								sym_filepath,
								QString(sym_filepath).remove(info.symlink_path() + "/")
								);

			QString parent_dir, sym_parent_dir, pure_filename;

			Util::File::split_filename(md.filepath(), parent_dir, pure_filename);
			Util::File::split_filename(sym_filepath, sym_parent_dir, pure_filename);

			bool contains = Util::contains(m->all_dirs, [&sym_parent_dir](const StringPair& sp){
				return (sym_parent_dir.compare(sp.first) == 0);
			});

			if(!contains)
			{
				while(sym_parent_dir != info.symlink_path())
				{
					m->all_dirs << StringPair(
									   sym_parent_dir,
									   QString(sym_parent_dir).remove(info.symlink_path() + "/")
									   );

					Util::File::split_filename(QString(sym_parent_dir), sym_parent_dir, pure_filename);
				}
			}
		}
	}

	std::sort(m->all_dirs.begin(), m->all_dirs.end(), [](const StringPair& sp1, const StringPair& sp2){
		return (sp1.first.toLower() < sp2.first.toLower());
	});
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

	auto it_all_dirs = m->all_dirs.begin();

	while(it_all_dirs != m->all_dirs.end())
	{
		QString dir_cvt = Library::Utils::convert_search_string(it_all_dirs->second, search_mode());

		if(dir_cvt.contains(cvt_search_string))
		{
			m->found_strings << it_all_dirs->first;
		}

		it_all_dirs++;
	}

	if(!m->search_only_dirs)
	{
		auto it_all_files = m->all_files.begin();

		while(it_all_files != m->all_files.end())
		{
			QString file_cvt = Library::Utils::convert_search_string(it_all_files->second, search_mode());
			if(file_cvt.contains(cvt_search_string))
			{
				QString f, d;
				Util::File::split_filename(it_all_files->first, d, f);
				m->found_strings << d;
			}

			it_all_files++;
		}
	}

	QString str;
	if(m->found_strings.size() > 0)
	{
		m->found_strings.removeDuplicates();

		std::sort(m->found_strings.begin(), m->found_strings.end(), [](const QString& str1 , const QString& str2){
			return (str1.toLower() < str2.toLower());
		});

		str = m->found_strings.first();
		m->cur_idx = 0;
	}

	for(const QString& found_str : m->found_strings)
	{
		QModelIndex found_idx = index(found_str);
		ret << found_idx;

		sp_log(Log::Debug, this) << "Data@found idx: " << found_idx.data().toString() << " Data to search: " << found_str;
		if(canFetchMore(found_idx)){
			fetchMore(found_idx);
		}

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

LibraryId DirectoryModel::library_id(const QModelIndex& index) const
{
	QString sympath = filePath(index);
	Library::Info info = Library::Manager::instance()->library_info_by_sympath(sympath);
	return info.id();
}

QString DirectoryModel::filepath_origin(const QModelIndex& index) const
{
	QString sympath = filePath(index);
	Library::Info info = Library::Manager::instance()->library_info_by_sympath(sympath);

	QString ret(sympath);
	ret.replace(info.symlink_path(), info.path());

	return ret;
}

bool DirectoryModel::is_root(const QModelIndex& index) const
{
	QString dir_path = filePath(index);
	QDir d(dir_path);
	d.cdUp();
	QString d_abs = d.absolutePath();

	QDir r(rootPath());
	QString r_abs = r.absolutePath();
	return(
		Util::File::clean_filename(d_abs) ==
		Util::File::clean_filename(r_abs)
	);
}

