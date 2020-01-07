/* LocalLibraryWatcher.h */

/* Copyright (C) 2011-2020 Lucio Carreras
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



#ifndef LIBRARYWATCHER_H
#define LIBRARYWATCHER_H

#include "Utils/typedefs.h"
#include "Utils/Pimpl.h"

#include <QObject>
#include <QList>

namespace Library
{
	class Container;
	class LocalLibraryWatcher : public QObject
	{
		Q_OBJECT
		PIMPL(LocalLibraryWatcher)

		public:
			explicit LocalLibraryWatcher(QObject* parent=nullptr);
			~LocalLibraryWatcher();

			QList<Container*> get_local_library_containers() const;

		private slots:
			void library_added(LibraryId id);
			void library_moved(LibraryId id, int from, int to);
			void library_renamed(LibraryId id);
			void library_removed(LibraryId id);
	};
}

#endif // LIBRARYWATCHER_H
