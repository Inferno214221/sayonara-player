/* CoverViewContextMenu.h */

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
			void sigZoomChanged(int zoom);
			void sigSortingChanged(Library::AlbumSortorder sortorder);

		public:
			enum Entry
			{
				EntryShowUtils = Library::ContextMenu::EntryLast << 1,
				EntrySorting = EntryShowUtils << 1,
				EntryZoom = EntrySorting << 1,
				EntryShowArtist = EntryZoom << 1
			};

			explicit CoverViewContextMenu(QWidget* parent);
			~CoverViewContextMenu() override;

			CoverViewContextMenu::Entries entries() const override;
			void showActions(CoverViewContextMenu::Entries entries) override;

		private:
			void init();
			void initSortingActions();
			void initZoomActions();

			void setZoom(int zoom);
			void setSorting(Library::AlbumSortorder sortOrder);

		private slots:
			void actionZoomTriggered(bool b);
			void actionSortingTriggered(bool b);

		protected:
			void languageChanged() override;
			void showEvent(QShowEvent* e) override;
	};
}
#endif // COVERVIEWCONTEXTMENU_H
