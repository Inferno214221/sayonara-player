/* DirectorySelectionHandler.cpp
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

#include "DirectorySelectionHandler.h"
#include "FileOperations.h"

#include "Components/Library/LocalLibrary.h"
#include "Components/LibraryManagement/LibraryManager.h"

#include "Utils/Utils.h"
#include "Utils/Algorithm.h"
#include "Utils/Library/LibraryInfo.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Library/Filter.h"

#include <QList>

namespace
{
	Library::Info getInfo(Library::Manager* libraryManager, const int currentId)
	{
		const auto libraries = libraryManager->allLibraries();
		const auto index = Util::Algorithm::indexOf(libraries, [&](const auto& info) {
			return (info.id() == currentId);
		});

		return (index > 0)
		       ? libraries[index]
		       : Library::Info {};
	}
}

struct DirectorySelectionHandler::Private
{
	LocalLibrary* genericLibrary;
	Library::Manager* libraryManager;
	Library::Info libraryInfo;
	AbstractLibrary* library {nullptr};

	explicit Private(Library::Manager* libraryManager) :
		libraryManager {libraryManager} {}
};

DirectorySelectionHandler::DirectorySelectionHandler(Library::Manager* libraryManager, QObject* parent) :
	QObject(parent)
{
	m = Pimpl::make<Private>(libraryManager);

	connect(m->libraryManager, &Library::Manager::sigAdded, this, [&](auto /* id */) {
		librariesChanged();
	});

	connect(m->libraryManager, &Library::Manager::sigRemoved, this, [&](auto /* id */) {
		librariesChanged();
	});

	connect(m->libraryManager, &Library::Manager::sigMoved, this, [&](auto /* id */, auto /* from */, auto /* to */) {
		librariesChanged();
	});

	connect(m->libraryManager, &Library::Manager::sigRenamed, this, [&](auto /* id */) {
		librariesChanged();
	});

	connect(m->libraryManager, &Library::Manager::sigPathChanged, this, [&](auto /* id */) {
		librariesChanged();
	});
}

DirectorySelectionHandler::~DirectorySelectionHandler() = default;

void DirectorySelectionHandler::createPlaylist(const QStringList& paths, bool createNewPlaylist)
{
	this->libraryInstance()->prepareTracksForPlaylist(paths, createNewPlaylist);
}

void DirectorySelectionHandler::playNext([[maybe_unused]] const QStringList& paths)
{
	this->libraryInstance()->playNextFetchedTracks();
}

void DirectorySelectionHandler::appendTracks([[maybe_unused]] const QStringList& paths)
{
	this->libraryInstance()->appendFetchedTracks();
}

void DirectorySelectionHandler::prepareTracksForPlaylist(const QStringList& paths, bool createNewPlaylist) const
{
	createPlaylist(paths, createNewPlaylist);
}

void DirectorySelectionHandler::requestImport(const LibraryId libraryId, const QStringList& paths,
                                              const QString& targetDirectory) const
{
	if((libraryId != m->libraryInfo.id()) || (libraryId < 0))
	{
		return;
	}

	if(auto* library = libraryInstance(); library)
	{
		connect(library, &LocalLibrary::sigImportDialogRequested,
		        this, &DirectorySelectionHandler::sigImportDialogRequested);

		library->importFilesTo(paths, targetDirectory);
	}
}

FileOperations* DirectorySelectionHandler::createFileOperation()
{
	auto* fo = new FileOperations(m->libraryManager, this);

	connect(fo, &FileOperations::sigStarted, this, &DirectorySelectionHandler::sigFileOperationStarted);
	connect(fo, &FileOperations::sigFinished, this, &DirectorySelectionHandler::sigFileOperationFinished);
	connect(fo, &FileOperations::sigFinished, fo, &QObject::deleteLater);

	return fo;
}

void DirectorySelectionHandler::copyPaths(const QStringList& paths, const QString& targetDirectory)
{
	createFileOperation()->copyPaths(paths, targetDirectory);
}

void DirectorySelectionHandler::movePaths(const QStringList& paths, const QString& targetDirectory)
{
	createFileOperation()->movePaths(paths, targetDirectory);
}

void DirectorySelectionHandler::renamePath(const QString& path, const QString& newName)
{
	createFileOperation()->renamePath(path, newName);
}

void DirectorySelectionHandler::renameByExpression(const QString& path, const QString& expression)
{
	createFileOperation()->renameByExpression(path, expression);
}

void DirectorySelectionHandler::deletePaths(const QStringList& paths)
{
	createFileOperation()->deletePaths(paths);
}

void DirectorySelectionHandler::librariesChanged()
{
	m->libraryInfo = getInfo(m->libraryManager, libraryId());

	emit sigLibrariesChanged();
}

void DirectorySelectionHandler::setLibraryId(const LibraryId libraryId)
{
	m->libraryInfo = getInfo(m->libraryManager, libraryId);
}

LibraryId DirectorySelectionHandler::libraryId() const { return m->libraryInfo.id(); }

Library::Info DirectorySelectionHandler::libraryInfo() const { return m->libraryInfo; }

LocalLibrary* DirectorySelectionHandler::libraryInstance() const
{
	return m->libraryManager->libraryInstance(libraryId());
}
