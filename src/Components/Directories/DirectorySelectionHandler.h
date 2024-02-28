/* DirectorySelectionHandler.h
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

#ifndef DIRECTORYSELECTIONHANDLER_H
#define DIRECTORYSELECTIONHANDLER_H

#include "Utils/Pimpl.h"
#include <QObject>

namespace Library
{
	class Info;
	class Manager;
}

class FileOperations;
class LocalLibrary;
class QStringList;

class DirectorySelectionHandler :
	public QObject
{
	Q_OBJECT
	PIMPL(DirectorySelectionHandler)

	signals:
		void sigLibrariesChanged();
		void sigImportDialogRequested(const QString& targetPath);
		void sigFileOperationStarted();
		void sigFileOperationFinished();

	private:
		FileOperations* createFileOperation();

	public:
		explicit DirectorySelectionHandler(Library::Manager* libraryManager, QObject* parent = nullptr);
		~DirectorySelectionHandler() override;

		void playNext(const QStringList& paths) const;
		void createPlaylist(const QStringList& paths, bool createNewPlaylist) const;
		void appendTracks(const QStringList& paths) const;
		void prepareTracksForPlaylist(const QStringList& paths, bool createNewPlaylist) const;

		void requestImport(LibraryId libId, const QStringList& paths, const QString& targetDirectory) const;

		void setLibraryId(LibraryId libId);
		[[nodiscard]] LibraryId libraryId() const;

		[[nodiscard]] Library::Info libraryInfo() const;
		[[nodiscard]] LocalLibrary* libraryInstance() const;

		void copyPaths(const QStringList& paths, const QString& target);
		void movePaths(const QStringList& paths, const QString& target);
		void renamePath(const QString& path, const QString& newName);
		void renameByExpression(const QString& path, const QString& expression);
		void deletePaths(const QStringList& paths);

	private slots:
		void librariesChanged();
};

#endif // DIRECTORYSELECTIONHANDLER_H
