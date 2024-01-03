/* PlaylistContextMenu.h */

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

#ifndef PLAYLISTCONTEXTMENU_H
#define PLAYLISTCONTEXTMENU_H

#include "Gui/Utils/ContextMenu/LibraryContextMenu.h"
#include "Utils/Library/Sortorder.h"

class DynamicPlaybackChecker;
class MetaData;

namespace Playlist
{
	class ContextMenu :
		public Library::ContextMenu
	{
		Q_OBJECT
		PIMPL(ContextMenu)

		signals:
			void sigBookmarkTriggered(Seconds timestamp);
			void sigSortingTriggered(Library::SortOrder sortOrder);

		public:
			enum Entry
			{
				EntryRating = (Library::ContextMenu::EntryLast << 1),
				EntryBookmarks = (Library::ContextMenu::EntryLast << 2),
				EntryCurrentTrack = (Library::ContextMenu::EntryLast << 3),
				EntryFindInLibrary = (Library::ContextMenu::EntryLast << 4),
				EntryJumpToNextAlbum = (Library::ContextMenu::EntryLast << 5),
				EntrySort = (Library::ContextMenu::EntryLast << 6),
				EntryReverse = (Library::ContextMenu::EntryLast << 7),
				EntryRandomize = (Library::ContextMenu::EntryLast << 8)
			};

			ContextMenu(DynamicPlaybackChecker* dynamicPlaybackChecker, QWidget* parent);
			~ContextMenu() override;

			ContextMenu::Entries entries() const override;
			void showActions(ContextMenu::Entries entries) override;

			using Library::ContextMenu::action;
			QAction* action(ContextMenu::Entry entry) const;

			ContextMenu::Entries setTrack(const MetaData& track, bool isCurrentTrack);
			void clearTrack();

		protected:
			void languageChanged() override;
			void skinChanged() override;
	};
}

#endif // PLAYLISTCONTEXTMENU_H
