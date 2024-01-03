/* FileOperationWorkerThread.h
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

#ifndef FLIEOPERATIONWORKERTHREAD_H
#define FLIEOPERATIONWORKERTHREAD_H

#include <QThread>
#include "Utils/Pimpl.h"

namespace Library
{
	class InfoAccessor;
}

class FileOperationThread :
	public QThread
{
	Q_OBJECT
	PIMPL(FileOperationThread)

	signals:
		void sigStarted();
		void sigFinished();

	public:
		virtual ~FileOperationThread();

		QList<LibraryId> sourceIds() const;
		QList<LibraryId> targetIds() const;

	protected:
		FileOperationThread(Library::InfoAccessor* libraryInfoAccessor, const QStringList& sourceFiles,
		                    const QStringList& targetFiles, QObject* parent);
		Library::InfoAccessor* libraryInfoAccessor();
};

class FileMoveThread :
	public FileOperationThread
{
	Q_OBJECT
	PIMPL(FileMoveThread)

	public:
		FileMoveThread(Library::InfoAccessor* libraryInfoAccessor, const QStringList& sourceFiles,
		               const QString& targetDir, QObject* parent);
		~FileMoveThread() override;

	protected:
		void run() override;
};

class FileCopyThread :
	public FileOperationThread
{
	Q_OBJECT
	PIMPL(FileCopyThread)

	public:
		FileCopyThread(Library::InfoAccessor* libraryInfoAccessor, const QStringList& sourceFiles,
		               const QString& targetDir, QObject* parent);
		~FileCopyThread() override;

	protected:
		void run() override;
};

class FileRenameThread :
	public FileOperationThread
{
	Q_OBJECT
	PIMPL(FileRenameThread)

	public:
		FileRenameThread(Library::InfoAccessor* libraryInfoAccessor, const QString& sourceFile,
		                 const QString& targetFile,
		                 QObject* parent);
		~FileRenameThread() override;

	protected:
		void run() override;
};

class FileDeleteThread :
	public FileOperationThread
{
	Q_OBJECT
	PIMPL(FileDeleteThread)

	public:
		FileDeleteThread(Library::InfoAccessor* libraryInfoAccessor, const QStringList& sourcePaths, QObject* parent);
		~FileDeleteThread() override;

	protected:
		void run() override;
};

#endif // FLIEOPERATIONWORKERTHREAD_H
