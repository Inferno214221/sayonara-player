/* Widget.cpp */

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

#include "Widget.h"
#include "Dialog.h"
#include <QFormLayout>

using Gui::MainWindow;
using Gui::Dialog;
using Gui::Widget;
using Gui::WidgetTemplate;

Widget::Widget(QWidget* parent) :
	WidgetTemplate<QWidget>(parent)
{}

Widget::~Widget() {}

Dialog* Widget::box_into_dialog()
{
	if(!_boxed_dialog){
		_boxed_dialog = new Dialog(this->parentWidget());
		QFormLayout* layout = new QFormLayout(_boxed_dialog);
		layout->addWidget(this);
		this->setParent(_boxed_dialog);
	}

	return _boxed_dialog;
}

void Widget::showEvent(QShowEvent* e)
{
	WidgetTemplate<QWidget>::showEvent(e);
	emit sig_shown();
}

void Widget::closeEvent(QCloseEvent* e)
{
	WidgetTemplate<QWidget>::closeEvent(e);
	emit sig_closed();
}

MainWindow::MainWindow(QWidget* parent) :
	WidgetTemplate<QMainWindow>(parent)
{}

MainWindow::~MainWindow() {}

void MainWindow::raise()
{
	QMainWindow::raise();

	if(this->isHidden())
	{
		this->setHidden(false);
	}

	if(!this->isVisible()){
		this->setVisible(true);
	}

	this->setWindowFlags((Qt::WindowFlags) (windowFlags() & ~Qt::WindowMinimized));
	this->activateWindow();
	this->showNormal();

	/*QDBusMessage msg = QDBusMessage::createMethodCall(
		"com.canonical.AppMenu.Registrar",
		"/com/canonical/AppMenu/Registrar",
		"com.canonical.AppMenu.Registrar",
		"RegisterWindow");

	QList<QVariant> args;
	QDBusObjectPath obj_path("/Menubar/1");
	args << QVariant::fromValue(static_cast<uint32_t>(winId()))
		 << QVariant::fromValue(obj_path);

	msg.setArguments(args);
	QDBusConnection::sessionBus().send(msg);*/
}

void MainWindow::showEvent(QShowEvent* e)
{
	WidgetTemplate<QMainWindow>::showEvent(e);
	emit sig_shown();
}
