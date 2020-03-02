/* MenuToolButton.h */

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

#ifndef MENUTOOL_H
#define MENUTOOL_H

#include "Utils/Pimpl.h"

#include "Gui/Utils/Widgets/WidgetTemplate.h"
#include "Gui/Utils/ContextMenu/ContextMenu.h"

#include <QPushButton>

namespace Gui
{
	class PreferenceAction;

	/**
	 * @brief This is the little button you often see near comboboxes\n
	 * It opens up a menu when clicked. The actions in the menu a configurable
	 * @ingroup Gui
	 */
	class MenuToolButton :
			public WidgetTemplate<QPushButton>
	{
		Q_OBJECT
		PIMPL(MenuToolButton)

		signals:
			void sigOpen();
			void sigNew();
			void sigUndo();
			void sigSave();
			void sigSaveAs();
			void sigRename();
			void sigDelete();
			void sigEdit();
			void sigDefault();

		public:
			explicit MenuToolButton(QWidget* parent);
			explicit MenuToolButton(QMenu* menu, QWidget* parent);
			virtual ~MenuToolButton() override;

			/**
			 * @brief Use this to add custom actions
			 * @param action a custom action
			 */
			void registerAction(QAction* action);

			/**
			 * @brief Use this to add a preference Action
			 * @param PreferenceAction for accessing preference dialog
			 */
			void registerPreferenceAction(Gui::PreferenceAction* action);

			/**
			 * @brief get current visible entries in menu\n
			 * calls ContextMenu::get_entries()
			 * @return a mask indicating which entries are shown. See ContextMenu::Entry
			 */
			Gui::ContextMenuEntries entries() const;

			void setOverrideText(bool b);


		public slots:
			/**
			 * @brief show/hide an action
			 * calls ContextMenu::show_action(ContextMenu::Entry entry, bool visible)
			 * @param entry the entry of interes
			 * @param visible show/hide the action
			 */
			void showAction(ContextMenu::Entry entry, bool visible);

			/**
			 * @brief shows all actions specified in options. Hide every other action\n
			 * calls ContextMenu::show_actions(ContextMenuEntries options)
			 * @param options option mask
			 */
			void showActions(ContextMenuEntries options);

			/**
			 * @brief show all actions
			 */
			void showAll();

		private slots:
			void mouseReleaseEvent(QMouseEvent* e) override;

		private:
			/**
			 * @brief show the menu when triggered
			 * @param pos mouse cursor position
			 */
			/**
			 * @brief check if the menu button should be enabled or disabled
			 * @return true if there are any entries in menu, false else
			 */
			bool proveEnabled();

		protected:
			void languageChanged() override;
	};
}

#endif // MENUTOOL_H
