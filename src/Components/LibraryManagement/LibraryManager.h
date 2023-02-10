/* LibraryManager.h */

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

#ifndef LIBRARYMANAGER_H
#define LIBRARYMANAGER_H

#include "Utils/typedefs.h"

#include <QObject>

class LocalLibrary;
class LibraryPlaylistInteractor;

namespace Library
{
	class Info;

	class InfoAccessor
	{
		public:
			virtual ~InfoAccessor() = default;

			[[nodiscard]] virtual QList<Info> allLibraries() const = 0;
			[[nodiscard]] virtual Info libraryInfo(LibraryId id) const = 0;
			[[nodiscard]] virtual Info libraryInfoByPath(const QString& path) const = 0;
			[[nodiscard]] virtual int count() const = 0;
			[[nodiscard]] virtual LocalLibrary* libraryInstance(LibraryId id) = 0;
	};

	class Manager :
		public QObject,
		public InfoAccessor
	{
		Q_OBJECT

		signals:
			void sigPathChanged(LibraryId id);
			void sigAdded(LibraryId id);
			void sigRenamed(LibraryId id);
			void sigMoved(LibraryId id, int from, int to);
			void sigRemoved(LibraryId id);

		public:
			~Manager() override = default;

			virtual LibraryId addLibrary(const QString& name, const QString& path) = 0;
			virtual bool renameLibrary(LibraryId id, const QString& newName) = 0;
			virtual bool removeLibrary(LibraryId id) = 0;
			virtual bool moveLibrary(int oldRow, int newRow) = 0;
			virtual bool changeLibraryPath(LibraryId id, const QString& newPath) = 0;

			static QString requestLibraryName(const QString& path);
			static Manager* create(LibraryPlaylistInteractor* playlistInteractor);
	};
}

#endif // LIBRARYMANAGER_H
