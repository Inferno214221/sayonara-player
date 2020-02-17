/* Widget.h */

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

#ifndef SAYONARAWIDGET_H
#define SAYONARAWIDGET_H

#include "WidgetTemplate.h"
#include <QWidget>
#include <QMainWindow>

namespace Gui
{
	class Dialog;

	/**
	 * @brief Widget with Settings connection. Also contains triggers for language_changed() and skin_changed()
	 * \nThe widget's content can be exported to a Dialog via the box_into_dialog() method.
	 * @ingroup Widgets
	 */
	class Widget :
			public Gui::WidgetTemplate<QWidget>
	{
		Q_OBJECT

	signals:
		void sigShown();
		void sigClosed();

	public:
		explicit Widget(QWidget* parent=nullptr);
		virtual ~Widget() override;

		Dialog* boxIntoDialog();

	protected:
		Dialog* mBoxedDialog=nullptr;

		virtual void showEvent(QShowEvent* e) override;
		virtual void closeEvent(QCloseEvent* e) override;
	};

	/**
	 * @brief The SayonaraMainWindow class
	 * @ingroup Widgets
	 */
	class MainWindow :
			public Gui::WidgetTemplate<QMainWindow>
	{
		Q_OBJECT

	signals:
		void sigShown();

	public:
		explicit MainWindow(QWidget* parent=nullptr);
		virtual ~MainWindow() override;

		void raise();

	protected:
		virtual void showEvent(QShowEvent *e) override;
	};
}

#endif // SAYONARAWIDGET_H
