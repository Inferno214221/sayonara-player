/* FileOperationWorkerThread.cpp
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

#include "FileOperationWorkerThread.h"

#include "Components/Library/LocalLibrary.h"
#include "Components/LibraryManagement/LibraryManager.h"
#include "Database/Connector.h"
#include "Database/LibraryDatabase.h"
#include "Utils/Algorithm.h"
#include "Utils/FileUtils.h"
#include "Utils/Library/LibraryInfo.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Set.h"

#include <QFileInfo>

namespace Algorithm = Util::Algorithm;

struct FileRenameThread::Private
{
	QString sourceFile;
	QString targetFile;

	Private(const QString& sourceFile, const QString& targetFile) :
		sourceFile(sourceFile),
		targetFile(targetFile) {}
};

FileRenameThread::FileRenameThread(Library::InfoAccessor* libraryInfoAccessor, const QString& sourceFile,
                                   const QString& targetFile, QObject* parent) :
	FileOperationThread(libraryInfoAccessor, QStringList {sourceFile}, QStringList {targetFile}, parent)
{
	m = Pimpl::make<Private>(sourceFile, targetFile);
}

FileRenameThread::~FileRenameThread() = default;

void FileRenameThread::run()
{
	bool success = false;

	QFileInfo info(m->sourceFile);
	if(info.isDir())
	{
		success = Util::File::renameDir(m->sourceFile, m->targetFile);
	}

	else if(info.isFile())
	{
		success = Util::File::renameFile(m->sourceFile, m->targetFile);
	}

	if(success)
	{
		auto* db = DB::Connector::instance();
		auto* libraryDatabase = db->libraryDatabase(-1, db->databaseId());
		const auto info = libraryInfoAccessor()->libraryInfoByPath(m->targetFile);

		QMap<QString, QString> resultPair {{m->sourceFile, m->targetFile}};
		libraryDatabase->renameFilepaths(resultPair, info.id());
	}
}

struct FileMoveThread::Private
{
	QStringList sourceFiles;
	QString targetDir;

	Private(const QStringList& sourceFiles, const QString& targetDir) :
		sourceFiles(sourceFiles),
		targetDir(targetDir) {}
};

FileMoveThread::FileMoveThread(Library::InfoAccessor* libraryInfoAccessor, const QStringList& sourceFiles,
                               const QString& targetDir, QObject* parent) :
	FileOperationThread(libraryInfoAccessor, sourceFiles, QStringList {targetDir}, parent)
{
	m = Pimpl::make<Private>(sourceFiles, targetDir);
}

FileMoveThread::~FileMoveThread() = default;

void FileMoveThread::run()
{
	QMap<QString, QString> resultPair;

	for(const QString& sourceFile: m->sourceFiles)
	{
		bool success = false;
		QString newName;

		QFileInfo info(sourceFile);
		if(info.isDir())
		{
			success = Util::File::moveDir(sourceFile, m->targetDir, newName);
		}

		else if(info.isFile())
		{
			success = Util::File::moveFile(sourceFile, m->targetDir, newName);
		}

		if(success)
		{
			resultPair[sourceFile] = newName;
		}
	}

	auto* db = DB::Connector::instance();
	auto* libraryDatabase = db->libraryDatabase(-1, db->databaseId());
	const auto info = libraryInfoAccessor()->libraryInfoByPath(m->targetDir);

	libraryDatabase->renameFilepaths(resultPair, info.id());
}

struct FileCopyThread::Private
{
	QStringList sourceFiles;
	QString targetDir;

	Private(const QStringList& sourceFiles, const QString& targetDir) :
		sourceFiles(sourceFiles),
		targetDir(targetDir) {}
};

FileCopyThread::FileCopyThread(Library::InfoAccessor* libraryInfoAccessor, const QStringList& sourceFiles,
                               const QString& targetDir, QObject* parent) :
	FileOperationThread(libraryInfoAccessor, sourceFiles, QStringList {targetDir}, parent)
{
	m = Pimpl::make<Private>(sourceFiles, targetDir);
}

FileCopyThread::~FileCopyThread() = default;

void FileCopyThread::run()
{
	for(const auto& sourceFile: m->sourceFiles)
	{
		QString newName;

		QFileInfo info(sourceFile);
		if(info.isDir())
		{
			Util::File::copyDir(sourceFile, m->targetDir, newName);
		}

		else if(info.isFile())
		{
			Util::File::copyFile(sourceFile, m->targetDir, newName);
		}
	}

	const auto info = libraryInfoAccessor()->libraryInfoByPath(m->targetDir);
	auto* library = libraryInfoAccessor()->libraryInstance(info.id());
	library->reloadLibrary(false, Library::ReloadQuality::Fast);
}

struct FileDeleteThread::Private
{
	QStringList paths;

	Private(const QStringList& paths) :
		paths(paths) {}
};

FileDeleteThread::FileDeleteThread(Library::InfoAccessor* libraryInfoAccessor, const QStringList& paths,
                                   QObject* parent) :
	FileOperationThread(libraryInfoAccessor, paths, QStringList(), parent)
{
	m = Pimpl::make<Private>(paths);
}

FileDeleteThread::~FileDeleteThread() = default;

void FileDeleteThread::run()
{
	Util::File::deleteFiles(m->paths);

	auto* db = DB::Connector::instance();
	auto* libraryDatabase = db->libraryDatabase(-1, 0);

	MetaDataList tracks;
	libraryDatabase->getAllTracksByPaths(m->paths, tracks);
	libraryDatabase->deleteTracks(Util::trackIds(tracks));
}

struct FileOperationThread::Private
{
	Util::Set<LibraryId> sourceIds;
	Util::Set<LibraryId> targetIds;

	Library::InfoAccessor* libraryInfoAccessor;

	Private(Library::InfoAccessor* libraryInfoAccessor, const QStringList& sourceFiles,
	        const QStringList& targetFiles) :
		libraryInfoAccessor {libraryInfoAccessor}
	{
		for(const auto& sourceFile: sourceFiles)
		{
			const auto info = libraryInfoAccessor->libraryInfoByPath(sourceFile);
			this->sourceIds << info.id();
		}

		for(const auto& targetFile: targetFiles)
		{
			const auto info = libraryInfoAccessor->libraryInfoByPath(targetFile);
			this->targetIds << info.id();
		}

		this->sourceIds.remove(-1);
		this->targetIds.remove(-1);
	}
};

FileOperationThread::FileOperationThread(Library::InfoAccessor* libraryInfoAccessor, const QStringList& sourceFiles,
                                         const QStringList& targetFiles,
                                         QObject* parent) :
	QThread(parent)
{
	m = Pimpl::make<Private>(libraryInfoAccessor, sourceFiles, targetFiles);
}

FileOperationThread::~FileOperationThread() = default;

QList<LibraryId> FileOperationThread::sourceIds() const
{
	return m->sourceIds.toList();
}

QList<LibraryId> FileOperationThread::targetIds() const
{
	return m->targetIds.toList();
}

Library::InfoAccessor* FileOperationThread::libraryInfoAccessor()
{
	return m->libraryInfoAccessor;
}