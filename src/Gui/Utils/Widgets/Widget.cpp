/* Widget.cpp */

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

Dialog* Widget::boxIntoDialog()
{
	if(!mBoxedDialog){
		mBoxedDialog = new Dialog(this->parentWidget());
		QFormLayout* layout = new QFormLayout(mBoxedDialog);
		layout->addWidget(this);
		this->setParent(mBoxedDialog);
	}

	return mBoxedDialog;
}

void Widget::showEvent(QShowEvent* e)
{
	WidgetTemplate<QWidget>::showEvent(e);
	emit sigShown();
}

void Widget::closeEvent(QCloseEvent* e)
{
	WidgetTemplate<QWidget>::closeEvent(e);
	emit sigClosed();
}

MainWindow::MainWindow(QWidget* parent) :
	WidgetTemplate<QMainWindow>(parent)
{}

MainWindow::~MainWindow() = default;

void MainWindow::raise()
{
	QMainWindow::raise();

	if(!this->isActiveWindow())
	{
		this->activateWindow();
	}

	if(this->isHidden()) {
		this->setHidden(false);
	}

	if(!this->isVisible()){
		this->setVisible(true);
	}

	Qt::WindowStates state = this->windowState();
	if(state & Qt::WindowMinimized)
	{
		this->setWindowState(state & ~Qt::WindowMinimized);
	}

	this->showNormal();
}

void MainWindow::showEvent(QShowEvent* e)
{
	WidgetTemplate<QMainWindow>::showEvent(e);
	emit sigShown();
}

void MainWindow::closeEvent(QCloseEvent* e)
{
	WidgetTemplate<QMainWindow>::closeEvent(e);
	emit sigClosed();
}
