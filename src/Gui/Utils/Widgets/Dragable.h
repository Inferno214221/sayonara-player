/* Dragable.h */

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

#ifndef DRAGGABLE_H
#define DRAGGABLE_H

#include "Utils/Pimpl.h"
#include <QObject>

class QPoint;
class QPixmap;
class QMimeData;
class QWidget;
class QDrag;
class QMouseEvent;
class QAbstractItemView;

namespace Gui
{
	class Dragable;
	class DragableConnector : public QObject
	{
		friend class Dragable;

		Q_OBJECT
		PIMPL(DragableConnector)

		private:
			DragableConnector(QAbstractItemView* widget, Dragable* dragable);
			~DragableConnector() override;

		private slots:
			void mousePressed(QMouseEvent* e);
			void mouseMoved(QMouseEvent* e);

			void dragDestroyed();
	};

	/**
	 * @brief The Dragable class
	 * @ingroup Widgets
	 */
	class Dragable
	{
		friend class DragableConnector;

		public:
			explicit Dragable(QAbstractItemView* parent);
			virtual ~Dragable();

			enum class ReleaseReason : char
			{
				Dropped,
				Destroyed
			};

		private:
			PIMPL(Dragable)

			void	startDrag(const QPoint& p);
			QDrag*	moveDrag(const QPoint& p);
			void	releaseDrag(ReleaseReason reason);

		protected:
			virtual QMimeData*	dragableMimedata() const=0;
			virtual bool		isValidDragPosition(const QPoint& p) const;
			virtual QPixmap		dragPixmap() const;
			virtual bool		hasDragLabel() const;
			virtual QString		dragLabel() const;
	};
}

#endif // DRAGGABLE_H
