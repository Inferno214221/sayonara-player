/* ContextMenu.h */

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

#ifndef CONTEXTMENU_H
#define CONTEXTMENU_H

#include "Gui/Utils/Widgets/WidgetTemplate.h"
#include "Utils/Pimpl.h"

#include <QMenu>


namespace Gui
{
	class PreferenceAction;
	/**
	 * @brief Combination of ContextMenu::Entry values
	 * @ingroup Gui
	 */
	using ContextMenuEntries=uint16_t;

	/**
	 * @brief A context menu with some standard actions
	 * @ingroup Gui
	 */
	class ContextMenu :
			public Gui::WidgetTemplate<QMenu>
	{
		Q_OBJECT
		PIMPL(ContextMenu)

		public:

			/**
			 * @brief The Entry enum
			 */
			enum Entry
			{
				EntryNone	=0,
				EntryNew	=(1<<0),
				EntryEdit	=(1<<1),
				EntryUndo	=(1<<2),
				EntrySave	=(1<<3),
				EntrySaveAs	=(1<<4),
				EntryRename	=(1<<5),
				EntryDelete	=(1<<6),
				EntryOpen	=(1<<7),
				EntryDefault=(1<<8)
			};

		signals:
			void sig_new();
			void sig_edit();
			void sig_undo();
			void sig_save();
			void sig_save_as();
			void sig_rename();
			void sig_delete();
			void sig_open();
			void sig_default();


		private:
			/**
			 * @brief show_action
			 * @param b
			 * @param action
			 */
			void show_action(bool b, QAction* action);


		public:
			explicit ContextMenu(QWidget *parent=nullptr);
			virtual ~ContextMenu();

			/**
			 * @brief register a custom action
			 * @param action the action. You have to set up the connection manually
			 */
			void register_action(QAction* action);

			/**
			 * @brief query, if there are visible actions
			 * @return true, if at least one action is visible. false else
			 */
			bool has_actions();

			/**
			 * @brief get all visible entries
			 * @return ContextMenuEntry mask
			 */
			ContextMenuEntries get_entries() const;


		protected:
			void showEvent(QShowEvent* e) override;
			void language_changed() override;
			void skin_changed() override;


		public slots:
			/**
			 * @brief show actions defined by ContextMenuEntry mask. Hide other actions
			 * @param mask of ContextMenu::Entry
			 */
			void show_actions(ContextMenuEntries entries);

			/**
			 * @brief show/hide specific action
			 * @param entry the entry of interes
			 * @param visible show/hide
			 */
			void show_action(ContextMenu::Entry entry, bool visible);

			/**
			 * @brief show all actions
			 */
			void show_all();

			void add_preference_action(PreferenceAction* action);


		private slots:
			/**
			 * @brief enable actions
			 */
			void timed_out();

	};
}

#endif // CONTEXTMENU_H
