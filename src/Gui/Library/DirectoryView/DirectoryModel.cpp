/* DirectoryModel.cpp
 *
 * Copyright (C) 2011-2024 Michael Lugmair
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
#include "DirectoryIconProvider.h"

#include "Components/Covers/LocalCoverSearcher.h"
#include "Components/Library/LocalLibrary.h"
#include "Components/LibraryManagement/LibraryManager.h"
#include "Gui/Utils/MimeData/CustomMimeData.h"
#include "Gui/Utils/MimeData/MimeDataUtils.h"
#include "Utils/Algorithm.h"
#include "Utils/FileUtils.h"
#include "Utils/Library/LibraryInfo.h"
#include "Utils/Logger/Logger.h"
#include "Utils/MetaData/MetaDataList.h"

#include <QFileSystemModel>
#include <QTimer>
#include <QUrl>

using Directory::Model;

struct Model::Private
{
	Library::InfoAccessor* libraryInfoAccessor;
	QFileSystemModel* model = nullptr;
	QTimer* filterTimer = nullptr;
	QString filter;

	LibraryId libraryId;

	Private(Library::InfoAccessor* libraryInfoAccessor, QObject* parent) :
		libraryInfoAccessor {libraryInfoAccessor},
		model {new QFileSystemModel(parent)},
		libraryId(-1)
	{
		model->setFilter(QDir::NoDotAndDotDot | QDir::Dirs);
		model->setIconProvider(new IconProvider());
	}
};

Model::Model(Library::InfoAccessor* libraryInfoAccessor, QObject* parent) :
	QSortFilterProxyModel(parent)
{
	m = Pimpl::make<Private>(libraryInfoAccessor, parent);

	this->setSourceModel(m->model);
}

Model::~Model() = default;

QModelIndex Model::setDataSource(LibraryId libraryId)
{
	const auto info = m->libraryInfoAccessor->libraryInfo(libraryId);
	const auto index = setDataSource(info.path());

	m->libraryId = libraryId;

	return index;
}

QModelIndex Model::setDataSource(const QString& path)
{
	m->libraryId = -1;
	m->model->setRootPath(path);

	const auto sourceIndex = m->model->index(path);

	return Util::File::exists(path)
	       ? this->mapFromSource(sourceIndex)
	       : QModelIndex();
}

LibraryId Model::libraryDataSource() const
{
	return m->libraryId;
}

QString Model::filePath(const QModelIndex& index) const
{
	return m->model->filePath(mapToSource(index));
}

QModelIndex Model::indexOfPath(const QString& path) const
{
	return mapFromSource(m->model->index(path));
}

void Model::setFilter(const QString& filter)
{
	if(!m->filterTimer)
	{
		m->filterTimer = new QTimer();
		m->filterTimer->setInterval(333);
		m->filterTimer->setSingleShot(true);

		connect(m->filterTimer, &QTimer::timeout, this, &Model::filterTimerTimeout);
	}

	m->filter = filter;
	m->filterTimer->start();

	emit sigBusy(true);
}

void Model::filterTimerTimeout()
{
	this->setFilterFixedString(m->filter);
	emit sigBusy(false);
}

bool Model::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
{
	if(this->filterRegExp().pattern().isEmpty())
	{
		return true;
	}

	const auto index = m->model->index(sourceRow, 0, sourceParent);
	const auto path = m->model->filePath(index);
	if(Util::File::isSubdir(m->model->rootPath(), path) ||
	   Util::File::isSamePath(m->model->rootPath(), path))
	{
		return true;
	}

	if(m->libraryId >= 0)
	{
		auto* localLibrary = m->libraryInfoAccessor->libraryInstance(m->libraryId);
		const auto& tracks = localLibrary->tracks();

		const auto contains = Util::Algorithm::contains(tracks, [&](const auto& track) {
			return Util::File::isSubdir(track.filepath(), path);
		});

		return contains;
	}

	return true;
}

int Model::columnCount(const QModelIndex& /*parent*/) const
{
	return 1;
}

QMimeData* Directory::Model::mimeData(const QModelIndexList& indexes) const
{
	QStringList paths;
	QList<QUrl> urls;
	for(const QModelIndex& index: indexes)
	{
		const auto path = this->filePath(index);
		paths << path;
		urls << QUrl::fromLocalFile(path);
	}

	auto* cmd = new Gui::CustomMimeData(this);
	if(!paths.isEmpty())
	{
		const auto coverPaths = Cover::LocalSearcher::coverPathsFromPathHint(paths.first());
		if(!coverPaths.isEmpty())
		{
			cmd->setCoverUrl(coverPaths.first());
		}
	}

	cmd->setUrls(urls);
	return cmd;
}
