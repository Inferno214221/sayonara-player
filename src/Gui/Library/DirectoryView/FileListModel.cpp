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
#include "Components/Library/LocalLibrary.h"

#include "Database/Connector.h"
#include "Database/LibraryDatabase.h"

#include "Utils/Set.h"
#include "Utils/Utils.h"
#include "Utils/Algorithm.h"
#include "Utils/DirectoryReader.h"
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
#include <QPixmap>
#include <QThread>
#include <QPixmapCache>

using Directory::IconWorkerThread;
using Directory::FileListModel;

enum ColumnName
{
	ColumnDecoration = 0,
	ColumnName,
	ColumnCount
};

namespace Algorithm = Util::Algorithm;

struct IconWorkerThread::Private
{
	QString filename;
	QPixmap pixmap;
	QSize targetSize;

	Private(const QSize& targetSize, const QString& filename) :
		filename(filename),
		targetSize(targetSize) {}
};

struct FileListModel::Private
{
	QString parentDirectory;
	QStringList files;
	Util::Set<QString> filesInLibrary;

	LocalLibrary* localLibrary;

	int iconSize;

	Private(LocalLibrary* localLibrary) :
		localLibrary(localLibrary),
		iconSize(24) {}

	void calcFilesInLibrary(const QString& dir)
	{
		filesInLibrary.clear();

		MetaDataList tracks;
		auto* db = DB::Connector::instance();
		auto* libraryDatabase = db->libraryDatabase(this->localLibrary->info().id(), 0);

		libraryDatabase->getAllTracksByPaths({dir}, tracks);
		for(const auto& track : tracks)
		{
			filesInLibrary.insert(track.filepath());
		}
	}
};

FileListModel::FileListModel(LocalLibrary* localLibrary, QObject* parent) :
	SearchableTableModel(parent)
{
	m = Pimpl::make<Private>(localLibrary);
}

FileListModel::~FileListModel() = default;

void FileListModel::setParentDirectory(const QString& dir)
{
	const auto oldRowcount = rowCount();

	m->files.clear();
	m->parentDirectory = dir;

	m->calcFilesInLibrary(dir);

	QStringList extensions;
	extensions << Util::soundfileExtensions();
	extensions << Util::playlistExtensions();
	extensions << "*";

	m->files = DirectoryReader::scanFilesInDirectory(QDir(dir), extensions);
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

	Algorithm::sort(m->files, [](const QString& f1, const QString& f2) {
		const auto isSoundfile1 = Util::File::isSoundFile(f1);
		const auto isSoundfile2 = Util::File::isSoundFile(f2);
		if(isSoundfile1 && isSoundfile2)
		{
			return (f1.toLower() < f2.toLower());
		}

		if(isSoundfile1 && !isSoundfile2)
		{
			return true;
		}

		if(!isSoundfile1 && isSoundfile2)
		{
			return false;
		}

		const auto isPlaylistfile1 = Util::File::isPlaylistFile(f1);
		const auto isPlaylistfile2 = Util::File::isPlaylistFile(f2);
		if(isPlaylistfile1 && isPlaylistfile2)
		{
			return (f1.toLower() < f2.toLower());
		}

		if(isPlaylistfile1 && !isPlaylistfile2)
		{
			return true;
		}

		if(!isPlaylistfile1 && isPlaylistfile2)
		{
			return false;
		}

		const auto isImagefile1 = Util::File::isImageFile(f1);
		const auto isImagefile2 = Util::File::isImageFile(f2);
		if(isImagefile1 && isImagefile2)
		{
			return (f1.toLower() < f2.toLower());
		}

		if(isImagefile1 && !isImagefile2)
		{
			return true;
		}

		if(!isImagefile1 && isImagefile2)
		{
			return false;
		}

		return (f1.toLower() < f2.toLower());
	});

	emit dataChanged
		(
			index(0, 0),
			index(m->files.size() - 1, this->columnCount(QModelIndex()))
		);
}

LibraryId FileListModel::libraryId() const
{
	return m->localLibrary->info().id();
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

	for(int i = 0; i < m->files.size(); i++)
	{
		if(checkRowForSearchstring(i, substr))
		{
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

	const auto row = index.row();
	const auto col = index.column();

	if(!Util::between(row, m->files))
	{
		return QVariant();
	}

	const auto& filename = m->files[row];

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

		if(File::isPlaylistFile(filename))
		{
			return Gui::Icons::icon(Gui::Icons::PlaylistFile);
		}

		if(File::isImageFile(filename))
		{
			QPixmap pixmap;
			if(!QPixmapCache::find(filename, &pixmap) || pixmap.isNull())
			{
				const auto size = QSize(32, 32);

				QPixmapCache::insert(filename, Gui::Icons::pixmap(Gui::Icons::ImageFile, size));

				auto* worker = new IconWorkerThread(size, filename);
				auto* t = new QThread();
				worker->moveToThread(t);

				connect(worker, &IconWorkerThread::sigFinished, this, &FileListModel::pixmapFetched);
				connect(worker, &IconWorkerThread::sigFinished, this, [t](const QString&) {
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
				icon.addPixmap(pixmap);
				return icon;
			}
		}
	}

	else if((role == Qt::ForegroundRole) && Util::File::isSoundFile(filename))
	{
		if(!m->filesInLibrary.contains(filename))
		{
			return Gui::Util::color(QPalette::ColorGroup::Disabled, QPalette::WindowText);
		}
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

	const auto pixmap = worker->pixmap();
	if(!pixmap.isNull())
	{
		QPixmapCache::insert(path, pixmap);
		emit dataChanged(index(0, 0), index(rowCount() - 1, columnCount() - 1));
	}

	worker->deleteLater();
}

QVariant FileListModel::headerData(int column, Qt::Orientation orientation, int role) const
{
	if((role == Qt::DisplayRole) && (orientation == Qt::Orientation::Horizontal))
	{
		if(column == ColumnDecoration)
		{
			return QString();
		}

		else if(column == ColumnName)
		{
			return Lang::get(Lang::Filename);
		}
	}

	return SearchableTableModel::headerData(column, orientation, role);
}

bool FileListModel::checkRowForSearchstring(int row, const QString& substr) const
{
	const auto convertedString = Library::Utils::convertSearchstring(substr, searchMode());
	const auto filename = Util::File::getFilenameOfPath(m->files[row]);

	const auto convertedFilepath = Library::Utils::convertSearchstring(filename, searchMode());

	return convertedFilepath.contains(convertedString);
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

	if(urls.isEmpty())
	{
		return nullptr;
	}

	auto* mimeData = new Gui::CustomMimeData(this);
	mimeData->setUrls(urls);

	const auto tracks = m->localLibrary->currentTracks();
	mimeData->setMetadata(tracks);

	const auto coverPaths = Cover::LocalSearcher::coverPathsFromPathHint(paths.first());
	if(!coverPaths.isEmpty())
	{
		Gui::MimeData::setCoverUrl(mimeData, coverPaths.first());
	}

	return mimeData;
}

Qt::ItemFlags FileListModel::flags(const QModelIndex& index) const
{
	return (index.isValid())
	       ? Qt::ItemFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled)
	       : Qt::NoItemFlags;
}

IconWorkerThread::IconWorkerThread(const QSize& targetSize, const QString& filename)
{
	m = Pimpl::make<Private>(targetSize, filename);
}

IconWorkerThread::~IconWorkerThread() = default;

void IconWorkerThread::start()
{
	const auto pixmap = QPixmap(m->filename);
	if(!pixmap.isNull())
	{
		m->pixmap = pixmap.scaled(m->targetSize);
	}

	emit sigFinished(m->filename);
}

QPixmap IconWorkerThread::pixmap() const
{
	return m->pixmap;
}
