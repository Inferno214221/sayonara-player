/* ContextMenu.h
 *
 * Copyright (C) 2011-2024 Michael Lugmair
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

#ifndef SOUNDCLOUDCONTEXTMENU_H
#define SOUNDCLOUDCONTEXTMENU_H

#include "Gui/Utils/ContextMenu/LibraryContextMenu.h"

namespace SC
{
	class ContextMenu : public Library::ContextMenu
	{
		Q_OBJECT
		PIMPL(ContextMenu)

		signals:
			void sigAddArtistTriggered();

		public:
			enum SCEntry
			{
				SCEntryAddArtist=Library::ContextMenu::EntryLast
			};

			using Entries=uint64_t;

			explicit ContextMenu(QWidget* parent=nullptr);
			~ContextMenu() override;

			// WidgetTemplateParent interface
		protected:
			void languageChanged() override;

			// ContextMenu interface
		public:
			ContextMenu::Entries entries() const override;
			void showActions(ContextMenu::Entries entries) override;
			void showAction(ContextMenu::Entry entry, bool visible) override;
	};

}

#endif // SOUNDCLOUDCONTEXTMENU_H
