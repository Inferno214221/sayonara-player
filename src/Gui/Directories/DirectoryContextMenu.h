/* DirectoryContextMenu.h */

/* Copyright (C) 2011-2020 Michael Lugmair (Lucio Carreras)
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

#ifndef DIRECTORYCONTEXTMENU_H
#define DIRECTORYCONTEXTMENU_H

#include "Gui/Utils/ContextMenu/LibraryContextMenu.h"

namespace Directory
{
	/**
	 * @brief The DirectoryContextMenu class
	 * @ingroup GuiDirectories
	 */
	class ContextMenu :
			public Library::ContextMenu
	{
		Q_OBJECT
		PIMPL(ContextMenu)

		signals:
			void sigCreateDirectoryClicked();
			void sigRenameClicked();
			void sigRenameByTagClicked();
			void sigCollapseAllClicked();
			void sigMoveToLibrary(LibraryId id);
			void sigCopyToLibrary(LibraryId id);

		public:
			enum Mode
			{
				Dir=0,
				File
			};

			enum Entry
			{
				EntryCreateDir		= Library::ContextMenu::EntryLast,
				EntryRename			= Library::ContextMenu::EntryLast << 1,
				EntryRenameByTag	= Library::ContextMenu::EntryLast << 2,
				EntryCollapseAll	= Library::ContextMenu::EntryLast << 3,
				EntryMoveToLib		= Library::ContextMenu::EntryLast << 4,
				EntryCopyToLib		= Library::ContextMenu::EntryLast << 5
			};

			ContextMenu(Mode mode, QWidget* parent);
			~ContextMenu() override;

			void refresh(int count=0);

			ContextMenu::Entries entries() const override;
			void showActions(ContextMenu::Entries entries) override;
			void showDirectoryAction(ContextMenu::Entry entry, bool b);

		private slots:
			void libraryMoveActionTriggered();
			void libraryCopyActionTriggered();

		protected:
			void languageChanged() override;
			void skinChanged() override;
	};
}

#endif // DIRECTORYCONTEXTMENU_H
