/* LibraryContextMenu.h */

/* Copyright (C) 2011-2019  Lucio Carreras
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

#include "Gui/Utils/Widgets/WidgetTemplate.h"
#include "Gui/Utils/Shortcuts/ShortcutIdentifier.h"
#include "Utils/Pimpl.h"

#include <QMenu>

class ExtensionSet;
class PreferenceAction;
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
		EntryCoverView=(1<<9),
		EntryPlay=(1<<10),
		EntryPlayNewTab=(1<<11),
		EntryFilterExtension=(1<<12),
		EntryReload=(1<<13),
		EntryLast=(1<<14)
	};

	using Entries=uint64_t;

public:
	explicit LibraryContextMenu(QWidget *parent=nullptr);
	virtual ~LibraryContextMenu();


	/**
	 * @brief get all visible entries
	 * @return all visible entries
	 */
	virtual LibraryContextMenu::Entries get_entries() const;

	/**
	 * @brief show a specific amount of Entries
	 * @param entries bitwise combination of Entry
	 */
	virtual void show_actions(LibraryContextMenu::Entries entries);

	/**
	 * @brief show/hide a specific Entry
	 * @param The entry of interest
	 * @param visible
	 */
	virtual void show_action(LibraryContextMenu::Entry entry, bool visible);

	/**
	 * @brief show all possible entries
	 */
	virtual void show_all();

	QAction* get_action(LibraryContextMenu::Entry entry) const;
	QAction* get_action_after(LibraryContextMenu::Entry entry) const;

	QAction* add_preference_action(PreferenceAction* action);
	QAction* before_preference_action() const;

	void set_action_shortcut(LibraryContextMenu::Entry entry, const QString& shortcut);

	void set_extensions(const ExtensionSet& extensions);


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
	void sig_filter_triggered(const QString& extension, bool b);
	void sig_reload_clicked();

private slots:
	void show_cover_view_changed();
	void show_filter_ext_bar_changed();
	void show_cover_triggered(bool b);
	void shortcut_changed(ShortcutIdentifier identifier);
	void show_filter_extension_bar_triggered(bool b);


protected:
	void skin_changed() override;
	void language_changed() override;
};

#endif // LIBRARYCONTEXTMENU_H
