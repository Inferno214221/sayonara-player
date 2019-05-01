/* FileListModel.cpp */

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

#include "FileListModel.h"
#include "Components/Directories/DirectoryReader.h"
#include "Components/Library/LibraryManager.h"

#include "Utils/globals.h"
#include "Utils/Utils.h"
#include "Utils/FileUtils.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Library/LibraryInfo.h"

#include "GUI/Utils/CustomMimeData.h"
#include "GUI/Utils/MimeDataUtils.h"
#include "GUI/Utils/Icons.h"

#include <QMap>
#include <QVariant>
#include <QModelIndex>
#include <QMimeData>
#include <QUrl>
#include <QIcon>
#include <QDir>

struct FileListModel::Private
{
	QString		parent_directory;

	LibraryId	library_id;
	QStringList files;

	Private() :
		library_id(-1)
	{}
};




FileListModel::FileListModel(QObject* parent) :
	SearchableListModel(parent)
{
	m = Pimpl::make<Private>();
}

FileListModel::~FileListModel() {}


void FileListModel::set_parent_directory(LibraryId id, const QString& dir)
{
	m->files.clear();
	m->library_id = id;
	m->parent_directory = dir;

	int old_rowcount = rowCount();

	QDir base_dir(dir);
	QStringList extensions;
	extensions = Util::soundfile_extensions();
	extensions << Util::playlist_extensions();
	extensions << "*";

	DirectoryReader reader;
	reader.set_filter(extensions);
	reader.scan_files(base_dir, m->files);

	if(m->files.size() > old_rowcount){
		beginInsertRows(QModelIndex(), old_rowcount, m->files.size());
		endInsertRows();
	}

	else if(m->files.size() < old_rowcount)
	{
		beginRemoveRows(QModelIndex(), m->files.size(), old_rowcount);
		endRemoveRows();
	}

	Util::sort(m->files, [](const QString& f1, const QString& f2){
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

	emit dataChanged(
			index(0,0),
			index(m->files.size() - 1, 0)
	);
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


LibraryId FileListModel::library_id() const
{
	return m->library_id;
}

int FileListModel::rowCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent)

	return m->files.size();
}

QVariant FileListModel::data(const QModelIndex &index, int role) const
{
	int row = index.row();
	if(!between(row, m->files)) {
		return QVariant();
	}

	QString filename = m->files[row];

	using namespace Util;

	switch(role)
	{
		case Qt::DisplayRole:
			return File::get_filename_of_path(filename);
		case Qt::DecorationRole:
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

			return QIcon();

		default:
			return QVariant();
	}
}

bool FileListModel::check_row_for_searchstring(int row, const QString& substr) const
{
	QString converted_string = Library::Utils::convert_search_string(substr, search_mode());

	QString dirname, filename;
	Util::File::split_filename(m->files[row], dirname, filename);

	QString converted_filepath = Library::Utils::convert_search_string(filename, search_mode());
	return converted_filepath.contains(converted_string);
}

QMimeData* FileListModel::mimeData(const QModelIndexList& indexes) const
{
	Q_UNUSED(indexes)

	QList<QUrl> urls;

	for(const QModelIndex& idx : indexes)
	{
		int row = idx.row();
		if(!between(row, m->files)){
			continue;
		}

		QString filename = m->files[row];
		if(Util::File::is_soundfile(filename) || Util::File::is_playlistfile(filename)){
			urls << QUrl::fromLocalFile(filename);
		}
	}

	if(urls.isEmpty()){
		return nullptr;
	}

	CustomMimeData* data = new CustomMimeData(this);
	data->setUrls(urls);

	return data;
}


Qt::ItemFlags FileListModel::flags(const QModelIndex& index) const
{
	QString file = m->files[index.row()];
	if(Util::File::is_soundfile(file) || Util::File::is_playlistfile(file))
	{
		return (Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled);
	}

	return Qt::NoItemFlags;
}
