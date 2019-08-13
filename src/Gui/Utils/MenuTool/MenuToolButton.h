/* MenuToolButton.h */

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

#ifndef MENUTOOL_H
#define MENUTOOL_H

#include "MenuButton.h"
#include "Utils/Pimpl.h"

#include "Gui/Utils/ContextMenu/ContextMenu.h"

namespace Gui
{
	class PreferenceAction;

	/**
	 * @brief This is the little button you often see near comboboxes\n
	 * It opens up a menu when clicked. The actions in the menu a configurable
	 * @ingroup Gui
	 */
	class MenuToolButton :
			public MenuButton
	{
		Q_OBJECT
		PIMPL(MenuToolButton)

		signals:
			void sig_open();
			void sig_new();
			void sig_undo();
			void sig_save();
			void sig_save_as();
			void sig_rename();
			void sig_delete();
			void sig_edit();
			void sig_default();

		public:
			explicit MenuToolButton(QWidget* parent);
			explicit MenuToolButton(QMenu* menu, QWidget *parent);
			virtual ~MenuToolButton() override;

			/**
			 * @brief Use this to add custom actions
			 * @param action a custom action
			 */
			void register_action(QAction* action);

			/**
			 * @brief Use this to add a preference Action
			 * @param PreferenceAction for accessing preference dialog
			 */
			void register_preference_action(Gui::PreferenceAction* action);

			/**
			 * @brief get current visible entries in menu\n
			 * calls ContextMenu::get_entries()
			 * @return a mask indicating which entries are shown. See ContextMenu::Entry
			 */
			Gui::ContextMenuEntries get_entries() const;

		private:
			/**
			 * @brief show the menu when triggered
			 * @param pos mouse cursor position
			 */
			void show_menu(QPoint pos) override;

			/**
			 * @brief check if the menu button should be enabled or disabled
			 * @return true if there are any entries in menu, false else
			 */
			bool prove_enabled() override;


		public slots:
			/**
			 * @brief show/hide an action
			 * calls ContextMenu::show_action(ContextMenu::Entry entry, bool visible)
			 * @param entry the entry of interes
			 * @param visible show/hide the action
			 */
			void show_action(ContextMenu::Entry entry, bool visible);

			/**
			 * @brief shows all actions specified in options. Hide every other action\n
			 * calls ContextMenu::show_actions(ContextMenuEntries options)
			 * @param options option mask
			 */
			void show_actions(ContextMenuEntries options);

			/**
			 * @brief show all actions
			 */
			void show_all();
	};
}

#endif // MENUTOOL_H
