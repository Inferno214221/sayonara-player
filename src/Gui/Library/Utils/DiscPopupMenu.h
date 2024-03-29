/* DiscPopupMenu.h */

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

#ifndef DISC_POPUP_MENU_H_
#define DISC_POPUP_MENU_H_

#include <QMenu>
#include <QAction>
#include <QList>
#include "Utils/typedefs.h"

namespace Library
{
	/**
	 * @brief A single action representing one or all discs
	 * @ingroup GuiLibrary
	 */
	class DiscAction :
		public QAction
	{
		Q_OBJECT

		signals:
			void sigDiscPressed(int);

		public:
			DiscAction(QWidget* parent, Disc d);
			~DiscAction() override;
	};

	/**
	 * @brief A menu containing various DiscAction objects
	 * @ingroup GuiLibrary
	 */
	class DiscPopupMenu :
		public QMenu
	{
		Q_OBJECT

		signals:
			void sigDiscPressed(Disc disc);

		public:
			DiscPopupMenu(QWidget* parent, QList<Disc> discs);
			~DiscPopupMenu() override;
	};
}

#endif
