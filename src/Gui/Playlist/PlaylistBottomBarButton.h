/* BottomBarButton.h */

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

#ifndef BOTTOMBARBUTTON_H
#define BOTTOMBARBUTTON_H

#include "Utils/Pimpl.h"
#include <QPushButton>

class QIcon;

namespace Playlist
{
	/**
	 * @brief The BottomBarButton class
	 * @ingroup GuiPlaylists
	 */
	class BottomBarButton :
		public QPushButton
	{
		Q_OBJECT
		PIMPL(BottomBarButton)

		public:
			BottomBarButton(const QIcon& icon, QWidget* parent);
			~BottomBarButton() override;

			void setIcon(const QIcon& icon);

		private:
			using QPushButton::setIcon;

		protected:
			void paintEvent(QPaintEvent* e) override;
	};
}

#endif // BOTTOMBARBUTTON_H
