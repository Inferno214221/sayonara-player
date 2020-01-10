/* FileListModel.cpp */

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

#include "FileListModel.h"

#include "Components/Directories/DirectoryReader.h"
#include "Components/LibraryManagement/LibraryManager.h"

#include "Database/Connector.h"
#include "Database/LibraryDatabase.h"

#include "Utils/Utils.h"
#include "Utils/Algorithm.h"
#include "Utils/Language/Language.h"
#include "Utils/FileUtils.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Library/LibraryInfo.h"
#include "Utils/MetaData/MetaDataList.h"

#include "Gui/Utils/CustomMimeData.h"
#include "Gui/Utils/MimeDataUtils.h"
#include "Gui/Utils/Icons.h"

#include <QVariant>
#include <QModelIndex>
#include <QMimeData>
#include <QUrl>
#include <QIcon>
#include <QDir>
#include <QMap>

enum ColumnName
{
	ColumnDecoration=0,
	ColumnName,
	ColumnInLibrary,
	ColumnCount
};

namespace Algorithm=Util::Algorithm;

struct FileListModel::Private
{
	QString		parent_directory;
	QStringList files;
	QMap<QString, bool> files_in_library;
	LibraryId	library_id;

	Private() :
		library_id(-1)
	{}
};


FileListModel::FileListModel(QObject* parent) :
	SearchableTableModel(parent)
{
	m = Pimpl::make<Private>();
}

FileListModel::~FileListModel() = default;

void FileListModel::set_parent_directory(LibraryId library_id, const QString& dir)
{
	int old_rowcount = rowCount();

	auto* db = DB::Connector::instance();
	DB::LibraryDatabase* lib_db = db->library_db(library_id, 0);

	MetaDataList tracks;
	lib_db->getAllTracksByPaths({dir}, tracks);

	m->files.clear();
	m->parent_directory = dir;
	m->library_id = library_id;

	QStringList extensions;
	extensions << Util::soundfile_extensions();
	extensions << Util::playlist_extensions();
	extensions << "*";

	DirectoryReader reader;
	reader.set_filter(extensions);
	reader.scan_files(QDir(dir), m->files);

	for(const QString& file : m->files)
	{
		m->files_in_library[file] = Util::Algorithm::contains(tracks, [&file](const MetaData& md){
			return Util::File::is_same_path(file, md.filepath());
		});
	}

	if(m->files.size() > old_rowcount)
	{
		beginInsertRows(QModelIndex(), old_rowcount, m->files.size());
		endInsertRows();
	}

	else if(m->files.size() < old_rowcount)
	{
		beginRemoveRows(QModelIndex(), m->files.size(), old_rowcount);
		endRemoveRows();
	}

	Algorithm::sort(m->files, [](const QString& f1, const QString& f2)
	{
		bool is_soundfile1 = Util::File::is_soundfile(f1);
		bool is_soundfile2 = Util::File::is_soundfile(f2);

		bool is_playlistfile1 = Util::File::is_playlistfile(f1);
		bool is_playlistfile2 = Util::File::is_playlistfile(f2);

		bool is_imagefile1 = Util::File::is_imagefile(f1);
		bool is_imagefile2 = Util::File::is_imagefile(f2);

		if(is_soundfile1 && is_soundfile2){
			return (f1.toLower() < f2.toLower());
		}

		if(is_soundfile1 && !is_soundfile2){
			return true;
		}

		if(!is_soundfile1 && is_soundfile2){
			return false;
		}

		if(is_playlistfile1 && is_playlistfile2){
			return (f1.toLower() < f2.toLower());
		}

		if(is_playlistfile1 && !is_playlistfile2){
			return true;
		}

		if(!is_playlistfile1 && is_playlistfile2){
			return false;
		}

		if(is_imagefile1 && is_imagefile2){
			return (f1.toLower() < f2.toLower());
		}

		if(is_imagefile1 && !is_imagefile2){
			return true;
		}

		if(!is_imagefile1 && is_imagefile2){
			return false;
		}

		return (f1.toLower() < f2.toLower());

	});

	emit dataChanged
	(
		index(0,0),
		index(m->files.size() - 1, this->columnCount(QModelIndex()))
	);
}

LibraryId FileListModel::library_id() const
{
	return m->library_id;
}

QString FileListModel::parent_directory() const
{
	return m->parent_directory;
}

QStringList FileListModel::files() const
{
	return m->files;
}

QModelIndexList FileListModel::search_results(const QString& substr)
{
	QModelIndexList ret;

	for(int i=0; i<m->files.size(); i++)
	{
		if(check_row_for_searchstring(i, substr)){
			ret << index(i, 0);
		}
	}

	return ret;
}

int FileListModel::rowCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent)

	return m->files.size();
}

int FileListModel::columnCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent)
	return ColumnCount;
}

QVariant FileListModel::data(const QModelIndex &index, int role) const
{
	int row = index.row();
	int col = index.column();

	if(!Util::between(row, m->files)) {
		return QVariant();
	}

	QString filename = m->files[row];
	bool in_library = m->files_in_library.contains(filename) && (m->files_in_library[filename] == true);

	using namespace Util;

	switch(role)
	{
		case Qt::DisplayRole:
			switch(col)
			{
				case ColumnName:
					return File::get_filename_of_path(filename);
				case ColumnInLibrary:
					return (in_library == true) ? Lang::get(Lang::Yes) : Lang::get(Lang::No);
				default: return QVariant();
			}

		case Qt::DecorationRole:
			if(col == ColumnDecoration)
			{
				if(File::is_soundfile(filename))
				{
					return Gui::Icons::icon(Gui::Icons::AudioFile);
				}

				if(File::is_playlistfile(filename)){
					return Gui::Icons::icon(Gui::Icons::PlaylistFile);
				}

				if(File::is_imagefile(filename))
				{
					return Gui::Icons::icon(Gui::Icons::ImageFile);
				}
			}

			return QIcon();

		case Qt::TextAlignmentRole:
			if(col == ColumnInLibrary){
				return int(Qt::AlignCenter | Qt::AlignVCenter);
			}

			return QVariant();

		case Qt::UserRole:
			return filename;

		default:
			return QVariant();
	}
}

QVariant FileListModel::headerData(int column, Qt::Orientation orientation, int role) const
{
	if((role == Qt::DisplayRole) && (orientation == Qt::Orientation::Horizontal))
	{
		if(column == ColumnDecoration) {
			return QString();
		}

		else if(column == ColumnName){
			return Lang::get(Lang::Filename);
		}

		else if(column == ColumnInLibrary){
			return Lang::get(Lang::Library);
		}
	}

	return SearchableTableModel::headerData(column, orientation, role);
}

bool FileListModel::check_row_for_searchstring(int row, const QString& substr) const
{
	QString converted_string = Library::Utils::convert_search_string(substr, search_mode());
	QString filename = Util::File::get_filename_of_path(m->files[row]);

	QString converted_filepath = Library::Utils::convert_search_string(filename, search_mode());
	return converted_filepath.contains(converted_string);
}

QMimeData* FileListModel::mimeData(const QModelIndexList& indexes) const
{
	QList<QUrl> urls;

	for(const QModelIndex& idx : indexes)
	{
		int row = idx.row();
		if(idx.column() != ColumnName){
			continue;
		}

		if(Util::between(row, m->files)) {
			urls << QUrl::fromLocalFile(m->files[row]);
		}
	}

	if(urls.isEmpty()){
		return nullptr;
	}

	auto* data = new Gui::CustomMimeData(this);
	data->setUrls(urls);

	return data;
}


Qt::ItemFlags FileListModel::flags(const QModelIndex& index) const
{
	if(index.isValid())
	{
		return Qt::ItemFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled);
	}

	return Qt::NoItemFlags;
}
