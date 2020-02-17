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

#include "Utils/Singleton.h"
#include "Utils/Pimpl.h"

#include <QObject>

class LocalLibrary;

namespace Library
{
	class Info;
	class Manager :
			public QObject
	{
		Q_OBJECT
		PIMPL(Manager)
		SINGLETON(Manager)

		friend class LocalLibrary;

	signals:
		void sigPathChanged(LibraryId id);
		void sigAdded(LibraryId id);
		void sigRenamed(LibraryId id);
		void sigMoved(LibraryId id, int from, int to);
		void sigRemoved(LibraryId id);

	private:
		void reset();

	public:
		LibraryId addLibrary(const QString& name, const QString& path);
		bool renameLibrary(LibraryId id, const QString& name);
		bool removeLibrary(LibraryId id);
		bool moveLibrary(int old_row, int new_row);
		bool changeLibraryPath(LibraryId id, const QString& path);

		QList<Info> allLibraries() const;

		Info libraryInfo(LibraryId id) const;
		Info libraryInfoByPath(const QString& path) const;

		int count() const;

		LocalLibrary* libraryInstance(LibraryId id);

		static QString requestLibraryName(const QString& path);
	};
}


#endif // LIBRARYMANAGER_H
