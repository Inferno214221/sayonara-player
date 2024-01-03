/* BookmarksMenu.h */

/* Copyright (C) 2011-2024 Michael Lugmair (Lucio Carreras)
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

#ifndef BOOKMARKS_ACTION_H
#define BOOKMARKS_ACTION_H

#include <QMenu>
#include "Utils/Pimpl.h"

namespace Playlist
{
	/**
	 * @brief The BookmarksMenu class
	 * @ingroup GuiPlaylists
	 */
	class BookmarksMenu :
		public QMenu
	{
		Q_OBJECT
		PIMPL(BookmarksMenu)

		signals:
			void sigBookmarkPressed(Seconds time_sec);

		public:
			explicit BookmarksMenu(QWidget* parent);
			virtual ~BookmarksMenu();

			bool hasBookmarks() const;
			void setTrack(const MetaData& track, bool editAllowed);
			MetaData track() const;

		private slots:
			void actionPressed();
			void bookmarksChanged(bool editAllowed);
	};
}

#endif
