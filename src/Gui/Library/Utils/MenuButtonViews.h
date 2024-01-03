/* MenuButtonViews.h
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

#ifndef MENUBUTTONVIEWS_H
#define MENUBUTTONVIEWS_H

#include "Gui/Utils/MenuTool/MenuToolButton.h"
#include "Gui/Utils/Shortcuts/ShortcutIdentifier.h"

namespace Library
{
	class MenuButtonViews : public Gui::MenuToolButton
	{
		Q_OBJECT
		PIMPL(MenuButtonViews)

		public:
			MenuButtonViews(QWidget* parent = nullptr);
			~MenuButtonViews() override;

		private slots:
			void actionTriggered(bool b);
			void viewTypeChanged();
			void shortcutChanged(ShortcutIdentifier identifier);

		protected:
			void languageChanged() override;
			void skinChanged() override;
	};
}

#endif // MENUBUTTONVIEWS_H
