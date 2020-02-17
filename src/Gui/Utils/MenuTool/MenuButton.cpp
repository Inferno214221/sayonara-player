/* MenuButton.cpp */

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

#include "MenuButton.h"
#include "Gui/Utils/Icons.h"
#include "Utils/Language/Language.h"

#include <QMouseEvent>

using namespace Gui;

MenuButton::MenuButton(QWidget* parent) :
	WidgetTemplate<QPushButton>(parent)
{
	this->setText(QString::fromUtf8("≡"));

	this->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	this->setIconSize(QSize(10, 10));
	this->setToolTip(Lang::get(Lang::Menu));
	this->setMaximumWidth(28);
}

MenuButton::~MenuButton() {}

void MenuButton::showMenu(QPoint pos)
{
	Q_UNUSED(pos)
	this->setAttribute( Qt::WA_Hover, false);
	this->setAttribute( Qt::WA_UnderMouse, false);
}

bool MenuButton::proveEnabled()
{
	return true;
}


void MenuButton::mousePressEvent(QMouseEvent* e)
{
	QPushButton::mousePressEvent(e);

	QPoint globalPoint = this->mapToGlobal(this->pos()) - this->pos();

	emit sigTriggered(globalPoint);

	showMenu(globalPoint);
}

void MenuButton::skinChanged() {}

void MenuButton::languageChanged()
{
	this->setToolTip(Lang::get(Lang::Menu));
	this->setText(QString::fromUtf8("≡"));
}
