/* EventFilter.h */

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

#ifndef EVENTFILTER_H
#define EVENTFILTER_H

#include <QObject>
#include <QEvent>
#include <QList>

class QAction;
class QMouseEvent;

namespace Gui
{
	/**
	 * @brief The GenericFilter class
	 * @ingroup EventFilter
	 */
	class GenericFilter :
			public QObject
	{
		Q_OBJECT

		signals:
			void sig_event(QEvent::Type);

		private:
			QList<QEvent::Type> m_types;

		public:
			explicit GenericFilter(const QEvent::Type& type, QObject* parent=nullptr);
			explicit GenericFilter(const QList<QEvent::Type>& types, QObject* parent=nullptr);

		protected:
			bool eventFilter(QObject* o , QEvent* e);
	};

	/**
	 * @brief The KeyPressFilter class
	 * @ingroup EventFilter
	 */
	class KeyPressFilter :
			public QObject
	{
		Q_OBJECT

		public:
			explicit KeyPressFilter(QObject* parent=nullptr);

		signals:
			void sig_key_pressed(int key);

		protected:
			bool eventFilter(QObject* o , QEvent* e);
	};

	/**
	 * @brief The ContextMenuFilter class
	 * @ingroup EventFilter
	 */
	class ContextMenuFilter :
			public QObject
	{
		Q_OBJECT

		public:
			explicit ContextMenuFilter(QObject* parent=nullptr);

		signals:
			// directly connect this signal to QMenu::popup
			void sig_context_menu(const QPoint& p, QAction* action);

		protected:
			bool eventFilter(QObject* o , QEvent* e);
	};

	/**
	 * @brief The MouseMoveFilter class
	 * @ingroup EventFilter
	 */
	class MouseMoveFilter :
			public QObject
	{
		Q_OBJECT

		public:
			explicit MouseMoveFilter(QObject* parent=nullptr);

		signals:
			void sig_mouse_moved(QMouseEvent* e);

		protected:
			bool eventFilter(QObject* o , QEvent* e);
	};

	/**
	 * @brief The MouseMoveFilter class
	 * @ingroup EventFilter
	 */
	class MousePressedFilter :
			public QObject
	{
		Q_OBJECT

		public:
			explicit MousePressedFilter(QObject* parent=nullptr);

		signals:
			void sig_mouse_pressed(QMouseEvent* e);

		protected:
			bool eventFilter(QObject* o , QEvent* e);
	};

	/**
	 * @brief The MouseEnterFilter class
	 * @ingroup EventFilter
	 */
	class MouseEnterFilter :
			public QObject
	{
		Q_OBJECT

		public:
			explicit MouseEnterFilter(QObject* parent=nullptr);

		signals:
			void sig_mouse_entered();

		protected:
			bool eventFilter(QObject* o, QEvent* e);
	};


	/**
	 * @brief The MouseLeaveFilter class
	 * @ingroup EventFilter
	 */
	class MouseLeaveFilter :
			public QObject
	{
		Q_OBJECT

		public:
			explicit MouseLeaveFilter(QObject* parent=nullptr);

		signals:
			void sig_mouse_left();

		protected:
			bool eventFilter(QObject* o, QEvent* e);
	};


	/**
	 * @brief The HideFilter class
	 * @ingroup
	 */
	class HideFilter :
			public QObject
	{
		Q_OBJECT

		public:
			explicit HideFilter(QObject* parent=nullptr);

		signals:
			void sig_hidden();

		protected:
			bool eventFilter(QObject* o, QEvent* e);
	};


	/**
	 * @brief The ShowFilter class
	 * @ingroup EventFilter
	 */
	class ShowFilter :
			public QObject
	{
		Q_OBJECT

		public:
			explicit ShowFilter(QObject* parent=nullptr);

		signals:
			void sig_shown();

		protected:
			bool eventFilter(QObject* o, QEvent* e);
	};


	/**
	 * @brief The PaintFilter class
	 * @ingroup EventFilter
	 */
	class PaintFilter :
			public QObject
	{
		Q_OBJECT

		public:
			explicit PaintFilter(QObject* parent=nullptr);

		signals:
			void sig_painted();

		protected:
			bool eventFilter(QObject* o, QEvent* e);
	};
}

#endif // EVENTFILTER_H
