/* LibraryContextMenu.h */

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

#ifndef LIBRARYCONTEXTMENU_H
#define LIBRARYCONTEXTMENU_H

#include "Gui/Utils/Widgets/WidgetTemplate.h"
#include "Gui/Utils/Shortcuts/ShortcutIdentifier.h"
#include "Utils/Pimpl.h"

#include <QMenu>

namespace Gui
{
	class ExtensionSet;
	class PreferenceAction;
}

namespace Library
{
	/**
	 * @brief Context menu used for Library and playlist windows
	 * @ingroup Gui
	 * @ingroup Library
	 */
	class ContextMenu :
			public Gui::WidgetTemplate<QMenu>
	{
		Q_OBJECT
		PIMPL(ContextMenu)

		signals:
			void sigInfoClicked();
			void sigEditClicked();
			void sigLyricsClicked();
			void sigRemoveClicked();
			void sigDeleteClicked();
			void sigPlayClicked();
			void sigPlayNewTabClicked();
			void sigPlayNextClicked();
			void sigAppendClicked();
			void sigRefreshClicked();
			void sigClearClicked();
			void sigFilterTriggered(const QString& extension, bool b);
			void sigReloadClicked();


		public:
			/**
			 * @brief This enum indicates which entries should be visible
			 */
			enum Entry
			{
				EntryNone			= 0,
				EntryInfo			= (1<<0),
				EntryEdit			= (1<<1),
				EntryLyrics			= (1<<2),
				EntryRemove			= (1<<3),
				EntryDelete			= (1<<4),
				EntryPlayNext		= (1<<5),
				EntryAppend			= (1<<6),
				EntryRefresh		= (1<<7),
				EntryClear			= (1<<8),
				EntryStandardView	= (1<<9),
				EntryCoverView		= (1<<10),
				EntryDirectoryView	= (1<<11),
				EntryPlay			= (1<<12),
				EntryPlayNewTab		= (1<<13),
				EntryFilterExtension= (1<<14),
				EntryReload			= (1<<15),
				EntryLast			= (1<<16)
			};

			using Entries=uint64_t;


		public:
			explicit ContextMenu(QWidget* parent=nullptr);
			virtual ~ContextMenu() override;

			/**
			 * @brief get all visible entries
			 * @return all visible entries
			 */
			virtual ContextMenu::Entries entries() const;

			/**
			 * @brief show a specific amount of Entries
			 * @param entries bitwise combination of Entry
			 */
			virtual void showActions(ContextMenu::Entries entries);

			/**
			 * @brief show/hide a specific Entry
			 * @param The entry of interest
			 * @param visible
			 */
			virtual void showAction(ContextMenu::Entry entry, bool visible);

			/**
			 * @brief show all possible entries
			 */
			virtual void showAll();

			QAction* action(ContextMenu::Entry entry) const;
			QAction* actionAfter(ContextMenu::Entry entry) const;

			QAction* addPreferenceAction(Gui::PreferenceAction* action);
			QAction* beforePreferenceAction() const;

			void setActionShortcut(ContextMenu::Entry entry, const QString& shortcut);

			void setExtensions(const Gui::ExtensionSet& extensions);
			void setSelectionCount(int selectionSount);

			QKeySequence shortcut(ContextMenu::Entry entry) const;


		private slots:
			void showFilterExtensionBarChanged();
			void showFilterExtensionBarTriggered(bool b);
			void libraryViewTypeChanged();
			void libraryViewTypeTriggered(bool b);
			void shortcutChanged(ShortcutIdentifier identifier);
			void skinTimerTimeout();


		protected:
			void skinChanged() override;
			void languageChanged() override;
	};
}

#endif // LIBRARYCONTEXTMENU_H
