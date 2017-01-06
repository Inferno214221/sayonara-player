/* Dragable.h */

/* Copyright (C) 2011-2017  Lucio Carreras
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

#include "Helper/Pimpl.h"

class QPoint;
class QPixmap;
class QMimeData;
class QWidget;
class QDrag;

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

	virtual QMimeData*	get_mimedata() const=0;
	virtual QPixmap		get_pixmap() const;
};

#endif // DRAGGABLE_H
