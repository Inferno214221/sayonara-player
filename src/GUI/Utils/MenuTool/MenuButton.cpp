/* MenuButton.cpp */

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

#include "MenuButton.h"
#include "GUI/Utils/GuiUtils.h"
#include "GUI/Utils/Icons.h"
#include "Utils/Language.h"

#include <QMouseEvent>

using namespace Gui;

MenuButton::MenuButton(QWidget* parent) :
	WidgetTemplate<QPushButton>(parent)
{
	this->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	this->setIconSize(QSize(10, 10));
	this->setToolTip(Lang::get(Lang::Menu));
	this->setMaximumWidth(28);
}

MenuButton::~MenuButton() {}

void MenuButton::show_menu(QPoint pos)
{
	Q_UNUSED(pos)
	this->setAttribute( Qt::WA_Hover, false);
	this->setAttribute( Qt::WA_UnderMouse, false);
	return;
}

bool MenuButton::prove_enabled()
{
	return true;
}


void MenuButton::mousePressEvent(QMouseEvent* e)
{
	QPushButton::mousePressEvent(e);

	QPoint globalPoint = this->mapToGlobal(this->pos()) - this->pos();

	emit sig_triggered(globalPoint);

	show_menu(globalPoint);
}

void MenuButton::skin_changed() {}

void MenuButton::language_changed()
{
	this->setToolTip(Lang::get(Lang::Menu));

	if(!this->text().isEmpty()){
		this->setText("...");
	}
}
