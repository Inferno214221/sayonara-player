/* FileListModel.cpp */

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

#include "FileListModel.h"

#include "Components/Covers/LocalCoverSearcher.h"
#include "Components/Directories/DirectoryReader.h"
#include "Components/Library/LocalLibrary.h"

#include "Database/Connector.h"
#include "Database/LibraryDatabase.h"

#include "Interfaces/LibraryInfoAccessor.h"

#include "Utils/Utils.h"
#include "Utils/Algorithm.h"
#include "Utils/Language/Language.h"
#include "Utils/FileUtils.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Library/LibraryInfo.h"
#include "Utils/MetaData/MetaDataList.h"

#include "Gui/Utils/MimeData/MimeDataUtils.h"
#include "Gui/Utils/MimeData/CustomMimeData.h"
#include "Gui/Utils/Icons.h"
#include "Gui/Utils/GuiUtils.h"

#include <QVariant>
#include <QModelIndex>
#include <QMimeData>
#include <QUrl>
#include <QIcon>
#include <QDir>
#include <QMap>
#include <QPixmap>
#include <QThread>
#include <QPixmapCache>

using Directory::IconWorkerThread;
using Directory::FileListModel;

enum ColumnName
{
	ColumnDecoration=0,
	ColumnName,
	ColumnCount
};

namespace Algorithm=Util::Algorithm;

struct IconWorkerThread::Private
{
	QString filename;
	QPixmap pixmap;
	QSize targetSize;

	Private(const QSize& targetSize, const QString& filename) :
		filename(filename),
		targetSize(targetSize)
	{}
};

struct FileListModel::Private
{
	QPixmapCache cache;
	QString	parentDirectory;
	QStringList files;
	QMap<QString, bool> filesInLibrary;

	LibraryInfoAccessor* libraryInfoAccessor;

	int	iconSize;
	LibraryId libraryId;

	Private(LibraryInfoAccessor* libraryInfoAccessor) :
		libraryInfoAccessor(libraryInfoAccessor),
		iconSize(24),
		libraryId(-1)
	{}
};

FileListModel::FileListModel(LibraryInfoAccessor* libraryInfoAccessor, QObject* parent) :
	SearchableTableModel(parent)
{
	m = Pimpl::make<Private>(libraryInfoAccessor);
}

FileListModel::~FileListModel() = default;

void FileListModel::setParentDirectory(LibraryId libraryId, const QString& dir)
{
	int oldRowcount = rowCount();

	auto* db = DB::Connector::instance();
	DB::LibraryDatabase* lib_db = db->libraryDatabase(libraryId, 0);

	MetaDataList tracks;
	lib_db->getAllTracksByPaths({dir}, tracks);

	m->files.clear();
	m->parentDirectory = dir;
	m->libraryId = libraryId;

	QStringList extensions;
	extensions << Util::soundfileExtensions();
	extensions << Util::playlistExtensions();
	extensions << "*";

	DirectoryReader reader;
	reader.setFilter(extensions);
	reader.scanFiles(QDir(dir), m->files);

	for(const QString& file : m->files)
	{
		m->filesInLibrary[file] = Util::Algorithm::contains(tracks, [&file](const MetaData& md){
			return Util::File::isSamePath(file, md.filepath());
		});
	}

	if(m->files.size() > oldRowcount)
	{
		beginInsertRows(QModelIndex(), oldRowcount, m->files.size());
		endInsertRows();
	}

	else if(m->files.size() < oldRowcount)
	{
		beginRemoveRows(QModelIndex(), m->files.size(), oldRowcount);
		endRemoveRows();
	}

	Algorithm::sort(m->files, [](const QString& f1, const QString& f2)
	{
		bool isSoundfile1 = Util::File::isSoundFile(f1);
		bool isSoundfile2 = Util::File::isSoundFile(f2);

		bool isPlaylistfile1 = Util::File::isPlaylistFile(f1);
		bool isPlaylistfile2 = Util::File::isPlaylistFile(f2);

		bool isImagefile1 = Util::File::isImageFile(f1);
		bool isImagefile2 = Util::File::isImageFile(f2);

		if(isSoundfile1 && isSoundfile2){
			return (f1.toLower() < f2.toLower());
		}

		if(isSoundfile1 && !isSoundfile2){
			return true;
		}

		if(!isSoundfile1 && isSoundfile2){
			return false;
		}

		if(isPlaylistfile1 && isPlaylistfile2){
			return (f1.toLower() < f2.toLower());
		}

		if(isPlaylistfile1 && !isPlaylistfile2){
			return true;
		}

		if(!isPlaylistfile1 && isPlaylistfile2){
			return false;
		}

		if(isImagefile1 && isImagefile2){
			return (f1.toLower() < f2.toLower());
		}

		if(isImagefile1 && !isImagefile2){
			return true;
		}

		if(!isImagefile1 && isImagefile2){
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

LibraryId FileListModel::libraryId() const
{
	return m->libraryId;
}

QString FileListModel::parentDirectory() const
{
	return m->parentDirectory;
}

QStringList FileListModel::files() const
{
	return m->files;
}

QModelIndexList FileListModel::searchResults(const QString& substr)
{
	QModelIndexList ret;

	for(int i=0; i<m->files.size(); i++)
	{
		if(checkRowForSearchstring(i, substr)){
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

QVariant FileListModel::data(const QModelIndex& index, int role) const
{
	using namespace Util;

	int row = index.row();
	int col = index.column();

	if(!Util::between(row, m->files)) {
		return QVariant();
	}

	const QString& filename = m->files[row];

	if((role == Qt::DisplayRole) && (col == ColumnName))
	{
		return File::getFilenameOfPath(filename);
	}

	else if((role == Qt::DecorationRole) && (col == ColumnDecoration))
	{
		if(File::isSoundFile(filename))
		{
			return Gui::Icons::icon(Gui::Icons::AudioFile);
		}

		if(File::isPlaylistFile(filename)){
			return Gui::Icons::icon(Gui::Icons::PlaylistFile);
		}

		if(File::isImageFile(filename))
		{
			QPixmap* pm = m->cache.find(filename);
			if(!pm || pm->isNull())
			{
				m->cache.insert(filename, Gui::Icons::pixmap(Gui::Icons::ImageFile));

				auto* worker = new IconWorkerThread(QSize(32,32), filename);
				auto* t = new QThread();
				worker->moveToThread(t);

				connect(worker, &IconWorkerThread::sigFinished, this, &FileListModel::pixmapFetched);
				connect(worker, &IconWorkerThread::sigFinished, this, [t](const QString&){
					t->quit();
				});

				connect(t, &QThread::started, worker, &IconWorkerThread::start);
				connect(t, &QThread::finished, t, &QObject::deleteLater);

				t->start();

				return Gui::Icons::icon(Gui::Icons::ImageFile);
			}

			else
			{
				QIcon icon;
				icon.addPixmap(*pm);
				return icon;
			}
		}
	}

	else if((role == Qt::TextColorRole) && Util::File::isSoundFile(filename))
	{
		bool inLibrary = m->filesInLibrary[filename];
		if(!inLibrary)
		{
			return QColor(214, 68, 45);
		}
	}

	else if(role == Qt::SizeHintRole)
	{
		return QSize(0, Gui::Util::viewRowHeight());
	}

	else if(role == Qt::UserRole)
	{
		return filename;
	}

	return QVariant();
}

void FileListModel::pixmapFetched(const QString& path)
{
	auto* worker = static_cast<IconWorkerThread*>(sender());

	QPixmap pm = worker->pixmap();
	if(!pm.isNull())
	{
		m->cache.insert(path, pm);
		emit dataChanged(index(0,0), index(rowCount() - 1, columnCount() - 1));
	}

	worker->deleteLater();
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
	}

	return SearchableTableModel::headerData(column, orientation, role);
}

bool FileListModel::checkRowForSearchstring(int row, const QString& substr) const
{
	QString converted_string = Library::Utils::convertSearchstring(substr, searchMode());
	QString filename = Util::File::getFilenameOfPath(m->files[row]);

	QString converted_filepath = Library::Utils::convertSearchstring(filename, searchMode());
	return converted_filepath.contains(converted_string);
}

QMimeData* FileListModel::mimeData(const QModelIndexList& indexes) const
{
	QList<QUrl> urls;
	QStringList paths;

	for(const auto& index : indexes)
	{
		if(index.column() == ColumnName)
		{
			const auto row = index.row();
			if(Util::between(row, m->files))
			{
				const auto& path = m->files[row];
				paths << path;
				urls << QUrl::fromLocalFile(path);
			}
		}
	}

	if(urls.isEmpty()) {
		return nullptr;
	}

	auto* mimeData = new Gui::CustomMimeData(this);
	mimeData->setUrls(urls);

	auto* localLibrary = m->libraryInfoAccessor->libraryInstance(m->libraryId);
	const auto tracks = localLibrary->currentTracks();
	mimeData->setMetadata(tracks);

	const auto coverPaths = Cover::LocalSearcher::coverPathsFromPathHint(paths.first());
	if(!coverPaths.isEmpty()) {
		Gui::MimeData::setCoverUrl(mimeData, coverPaths.first());
	}

	return mimeData;
}

Qt::ItemFlags FileListModel::flags(const QModelIndex& index) const
{
	if(index.isValid())
	{
		return Qt::ItemFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled);
	}

	return Qt::NoItemFlags;
}

IconWorkerThread::IconWorkerThread(const QSize& targetSize, const QString& filename)
{
	m = Pimpl::make<Private>(targetSize, filename);
}

IconWorkerThread::~IconWorkerThread() = default;

void IconWorkerThread::start()
{
	QPixmap pm = QPixmap(m->filename);
	if(!pm.isNull()) {
		m->pixmap = pm.scaled(m->targetSize);
	}

	emit sigFinished(m->filename);
}

QPixmap IconWorkerThread::pixmap() const
{
	return m->pixmap;
}
