/* CoverViewContextMenu.h */

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

#ifndef COVERVIEWCONTEXTMENU_H
#define COVERVIEWCONTEXTMENU_H

#include "Gui/Utils/ContextMenu/LibraryContextMenu.h"
#include "Utils/Library/Sortorder.h"
#include <QStringList>

class MetaData;

namespace Library
{
	class ActionPair;

	/**
	 * @brief Context menu with some additional actions compared to
	 * Gui::LibraryContextMenu
	 * @ingroup GuiLibrary
	 */
	class CoverViewContextMenu :
			public Library::ContextMenu
	{
		Q_OBJECT
		PIMPL(CoverViewContextMenu)

	signals:
		void sig_zoom_changed(int zoom);
		void sig_sorting_changed(Library::SortOrder sortorder);

	public:
		enum Entry
		{
			EntryShowUtils=(Library::ContextMenu::EntryLast << 1),
			EntrySorting=(EntryShowUtils << 1),
			EntryZoom=(EntrySorting << 1),
			EntryShowArtist=(EntryZoom << 1)
		};

		explicit CoverViewContextMenu(QWidget* parent);
		~CoverViewContextMenu() override;

		CoverViewContextMenu::Entries get_entries() const override;
		void show_actions(CoverViewContextMenu::Entries entries) override;

	protected:
		void showEvent(QShowEvent* e) override;

	private:
		void language_changed() override;

		void init();
		void init_sorting_actions();
		void init_zoom_actions();

		void set_zoom(int zoom);
		void set_sorting(Library::SortOrder so);

	private slots:
		void action_zoom_triggered(bool b);
		void action_sorting_triggered(bool b);
	};
}
#endif // COVERVIEWCONTEXTMENU_H
