/* Dragable.h */

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

#ifndef DRAGGABLE_H
#define DRAGGABLE_H

#include "Utils/Pimpl.h"

class QPoint;
class QPixmap;
class QMimeData;
class QWidget;
class QDrag;

namespace Gui
{
	/**
	 * @brief The Dragable class
	 * @ingroup Widgets
	 */
	class Dragable
	{
		public:
			explicit Dragable(QWidget* parent);
			virtual ~Dragable();

			enum class ReleaseReason : char
			{
				Dropped,
				Destroyed
			};

		private:
			PIMPL(Dragable)

		protected:
			virtual void	drag_pressed(const QPoint& p) final;
			virtual QDrag*	drag_moving(const QPoint& p) final;
			virtual void	drag_released(ReleaseReason reason);

			virtual QMimeData*	dragable_mimedata() const=0;
			virtual bool		is_valid_drag_position(const QPoint& p) const;
			virtual QPixmap		drag_pixmap() const;
			virtual bool		has_drag_label() const;
			virtual QString		drag_label() const;
	};
}

#endif // DRAGGABLE_H
