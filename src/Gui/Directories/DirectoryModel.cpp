
/* Copyright (C) 2011-2020 Michael Lugmair (Lucio Carreras)
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
	Library::Info	libraryInfo;
	QStringList		foundStrings;
	int				currentIndex;
	bool			searchOnlyDirectories;

	Util::Set<QString> allDirectories;
	Util::Set<QString> allFiles;

	Private() :
		currentIndex(-1),
		searchOnlyDirectories(false)
	{}
};

DirectoryModel::DirectoryModel(QObject* parent) :
	SearchableModel<QFileSystemModel>(parent)
{
	m = Pimpl::make<Private>();
}

DirectoryModel::~DirectoryModel() = default;

void DirectoryModel::setSearchOnlyDirectories(bool b)
{
	if(b != m->searchOnlyDirectories){
		m->currentIndex = 0;
	}

	m->searchOnlyDirectories = b;
}

void DirectoryModel::setLibraryInfo(const Library::Info& info)
{
	m->libraryInfo = info;
	setRootPath(info.path());
}

Library::Info DirectoryModel::libraryInfo() const
{
	return m->libraryInfo;
}

void DirectoryModel::createFileList(const QString& substr)
{
	m->allFiles.clear();
	m->allDirectories.clear();

	Library::Filter filter;
	filter.setFiltertext(substr, searchMode());
	filter.setMode(Library::Filter::Mode::Filename);

	LibraryId lib_id = m->libraryInfo.id();

	auto* db = DB::Connector::instance();
	DB::LibraryDatabase* library_db = db->libraryDatabase(lib_id, 0);

	MetaDataList v_md;
	library_db->getAllTracksBySearchString(filter, v_md);

	for(const MetaData& md : v_md)
	{
		QString d = Util::File::getParentDirectory(md.filepath());
		m->allFiles << md.filepath();
		m->allDirectories << d;
	}
}

QModelIndexList DirectoryModel::searchResults(const QString& substr)
{
	QModelIndexList ret;

	m->foundStrings.clear();
	m->currentIndex = -1;

	createFileList(substr);
	if(m->allFiles.isEmpty()){
		return QModelIndexList();
	}

	QString cvt_search_string = Library::Utils::convertSearchstring(substr, searchMode());

	for(const QString& dir : m->allDirectories)
	{
		QString dir_cvt(dir);
		dir_cvt.remove(m->libraryInfo.path());
		dir_cvt = Library::Utils::convertSearchstring(dir_cvt, searchMode());

		if(dir_cvt.contains(cvt_search_string))
		{
			m->foundStrings << dir;
		}
	}

	if(!m->searchOnlyDirectories)
	{
		for(const QString& file : m->allFiles)
		{
			QString file_cvt(file);
			file_cvt.remove(m->libraryInfo.path());
			file_cvt = Library::Utils::convertSearchstring(file_cvt, searchMode());

			if(file_cvt.contains(cvt_search_string))
			{
				QString d = Util::File::getParentDirectory(file);
				m->foundStrings << d;
			}
		}
	}

	QString str;
	if(m->foundStrings.size() > 0)
	{
		str = m->foundStrings.first();
		m->currentIndex = 0;
	}

	for(const QString& found_str : m->foundStrings)
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
