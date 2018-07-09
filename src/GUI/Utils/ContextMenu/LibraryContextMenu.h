/* LibraryContextMenu.h */

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

#ifndef LIBRARYCONTEXTMENU_H
#define LIBRARYCONTEXTMENU_H

#include <QMenu>

#include "GUI/Utils/Widgets/WidgetTemplate.h"
#include "Utils/Pimpl.h"

using LibraryContexMenuEntries=int;

class PreferenceAction;
class QAction;
/**
 * @brief Context menu used for Library and playlist windows
 * @ingroup GUIHelper
 */
class LibraryContextMenu :
		public Gui::WidgetTemplate<QMenu>
{
	Q_OBJECT
	PIMPL(LibraryContextMenu)

public:
	explicit LibraryContextMenu(QWidget *parent=nullptr);
	virtual ~LibraryContextMenu();


	/**
	 * @brief This enum indicates which entries should be visible
	 */
	enum Entry
	{
		EntryNone=0,
		EntryInfo=(1<<0),
		EntryEdit=(1<<1),
		EntryLyrics=(1<<2),
		EntryRemove=(1<<3),
		EntryDelete=(1<<4),
		EntryPlayNext=(1<<5),
		EntryAppend=(1<<6),
		EntryRefresh=(1<<7),
		EntryClear=(1<<8),
		EntryRating=(1<<9),
		EntryClearSelection=(1<<10),
		EntryCoverView=(1<<11),
		EntryPlay=(1<<12),
		EntryPlayNewTab=(1<<13),
		EntryLast=(1<<14)
	};

	/**
	 * @brief get all visible entries
	 * @return all visible entries
	 */
	virtual LibraryContexMenuEntries get_entries() const;

	/**
	 * @brief show a specific amount of Entries
	 * @param entries bitwise combination of Entry
	 */
	virtual void show_actions(LibraryContexMenuEntries entries);

	/**
	 * @brief show/hide a specific Entry
	 * @param The entry of interest
	 * @param visible
	 */
	virtual void show_action(Entry entry, bool visible);

	/**
	 * @brief show all possible entries
	 */
	virtual void show_all();

	/**
	 * @brief set rating for the rating entry
	 * @param rating from 0 to 5
	 */
	void set_rating(Rating rating);

	QAction* get_action(Entry entry) const;

	QAction* add_preference_action(PreferenceAction* action);


signals:
	void sig_info_clicked();
	void sig_edit_clicked();
	void sig_lyrics_clicked();
	void sig_remove_clicked();
	void sig_delete_clicked();
	void sig_play_clicked();
	void sig_play_new_tab_clicked();
	void sig_play_next_clicked();
	void sig_append_clicked();
	void sig_refresh_clicked();
	void sig_clear_clicked();
	void sig_rating_changed(Rating rating);
	void sig_clear_selection_clicked();


private slots:
	void show_cover_view_changed();
	void show_cover_triggered(bool b);


protected:
	QAction* init_rating_action(Rating rating, QObject* parent);

	void skin_changed() override;
	void language_changed() override;
};

#endif // LIBRARYCONTEXTMENU_H
