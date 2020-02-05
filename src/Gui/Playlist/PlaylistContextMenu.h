/* PlaylistContextMenu.h */

/* Copyright (C) 2011-2020  Lucio Carreras
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

class MetaData;

namespace Playlist
{
	/**
	 * @brief The PlaylistContextMenu class
	 * @ingroup GuiPlaylists
	 */
	class ContextMenu :
			public Library::ContextMenu
	{
		Q_OBJECT
		PIMPL(ContextMenu)

	signals:
		void sig_rating_changed(Rating rating);
		void sig_bookmark_pressed(Seconds timestamp);
		void sig_jump_to_current_track();
		void sig_find_track_triggered();
		void sig_reverse_triggered();

	public:
		enum Entry
		{
			EntryRating			= (Library::ContextMenu::EntryLast << 1),
			EntryBookmarks		= (Library::ContextMenu::EntryLast << 2),
			EntryCurrentTrack	= (Library::ContextMenu::EntryLast << 3),
			EntryFindInLibrary	= (Library::ContextMenu::EntryLast << 4),
			EntryReverse		= (Library::ContextMenu::EntryLast << 5)
		};

		explicit ContextMenu(QWidget* parent);
		~ContextMenu() override;

		ContextMenu::Entries get_entries() const override;
		void show_actions(ContextMenu::Entries entries) override;

		/**
		 * @brief set rating for the rating entry
		 * @param rating from 0 to 5
		 */
		void set_rating(Rating rating);
		void set_metadata(const MetaData& md);

	private:
		QAction* init_rating_action(Rating rating, QObject* parent);
		void language_changed() override;
		void skin_changed() override;

	private slots:
		void bookmark_pressed(Seconds timestamp);
	};
}


#endif // PLAYLISTCONTEXTMENU_H
