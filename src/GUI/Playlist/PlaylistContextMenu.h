/* PlaylistContextMenu.h */

/* Copyright (C) 2011-2017  Lucio Carreras
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

#include "GUI/Utils/ContextMenu/LibraryContextMenu.h"

class MetaData;
class PlaylistContextMenu :
		public LibraryContextMenu
{
	Q_OBJECT
	PIMPL(PlaylistContextMenu)

signals:
	void sig_rating_changed(Rating rating);
	void sig_bookmark_pressed(Seconds timestamp);
	void sig_jump_to_current_track();

public:
	enum Entry
	{
		EntryRating=(LibraryContextMenu::EntryLast << 1),
		EntryBookmarks=(EntryRating << 1),
		EntryCurrentTrack=(EntryBookmarks << 1)
	};

	using Entries=LibraryContextMenu::Entries;

	explicit PlaylistContextMenu(QWidget* parent);
	~PlaylistContextMenu();

	PlaylistContextMenu::Entries get_entries() const override;
	void show_actions(PlaylistContextMenu::Entries entries) override;

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


#endif // PLAYLISTCONTEXTMENU_H
