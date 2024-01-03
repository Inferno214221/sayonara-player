/* LibraryContextMenu.h */

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
	class ContextMenu :
		public Gui::WidgetTemplate<QMenu>
	{
		Q_OBJECT
		PIMPL(ContextMenu)

		signals:
			void sigFilterTriggered(const QString& extension, bool b);

		public:
			enum Entry
			{
				EntryNone = 0,
				EntryInfo = (1 << 0),
				EntryEdit = (1 << 1),
				EntryLyrics = (1 << 2),
				EntryRemove = (1 << 3),
				EntryDelete = (1 << 4),
				EntryPlayNext = (1 << 5),
				EntryAppend = (1 << 6),
				EntryRefresh = (1 << 7),
				EntryClear = (1 << 8),
				EntryPlay = (1 << 9),
				EntryPlayNewTab = (1 << 10),
				EntryFilterExtension = (1 << 11),
				EntryReload = (1 << 12),
				EntryViewType = (1 << 13),
				EntryLast = (1 << 14)
			};

			using Entries = uint64_t;

		public: // NOLINT(readability-redundant-access-specifiers)
			explicit ContextMenu(QWidget* parent = nullptr);
			~ContextMenu() override;

			[[nodiscard]] virtual ContextMenu::Entries entries() const;
			virtual void showActions(ContextMenu::Entries entries);
			virtual void showAction(ContextMenu::Entry entry, bool visible);
			virtual void showAll();

			[[nodiscard]] QAction* action(ContextMenu::Entry entry) const;
			[[nodiscard]] QAction* actionAfter(ContextMenu::Entry entry) const;

			QAction* addPreferenceAction(Gui::PreferenceAction* action);
			[[nodiscard]] QAction* beforePreferenceAction() const;

			void setExtensions(const Gui::ExtensionSet& extensions);
			void setSelectionCount(int selectionSount);

			[[nodiscard]] QKeySequence shortcut(ContextMenu::Entry entry) const;

		private slots:
			void showFilterExtensionBarChanged();
			void showFilterExtensionBarTriggered(bool b);
			void libraryViewTypeChanged();
			void libraryViewTypeTriggered(bool b);
			void shortcutChanged(ShortcutIdentifier identifier);

		protected:
			void skinChanged() override;
			void languageChanged() override;
	};
}

#endif // LIBRARYCONTEXTMENU_H
