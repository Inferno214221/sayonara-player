/* PluginCloseButton.cpp */

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

/* PluginCloseButton.cpp */

#include "PluginCloseButton.h"

#include "Gui/Utils/GuiUtils.h"
#include "Gui/Utils/Icons.h"
#include "Gui/Utils/Style.h"
#include "Utils/Language.h"

#include <QEvent>

PluginCloseButton::PluginCloseButton(QWidget *parent) :
	Gui::WidgetTemplate<QPushButton>(parent)
{
	this->setFlat(true);
	this->setIconSize(QSize(14,14));

	this->setStyleSheet("margin-left: 2px; margin-right: 2px; padding-left: 0px; padding-right: 0px; background: transparent;");
	this->setToolTip(Lang::get(Lang::Close));
}

PluginCloseButton::~PluginCloseButton() {}

void PluginCloseButton::mouseReleaseEvent(QMouseEvent *e)
{
	QPushButton::mouseReleaseEvent(e);
}


void PluginCloseButton::enterEvent(QEvent* e)
{
	QPushButton::enterEvent(e);

	QIcon icon;

	if(Style::is_dark()){
		icon = Gui::Util::icon("tool_grey");
	}

	else{
		icon = Gui::Icons::icon(Gui::Icons::Close);
		if(icon.isNull()){
			icon = Gui::Icons::icon(Gui::Icons::Exit);
		}
	}

	if( this->isEnabled() ){
		this->setIcon(icon);
		e->accept();
	}
}

void PluginCloseButton::leaveEvent(QEvent* e){
	QPushButton::leaveEvent(e);

	set_std_icon();
}


void PluginCloseButton::set_std_icon()
{
	QIcon icon;
	QPixmap pixmap;
	QPixmap pixmap_disabled;

	if(Style::is_dark()){
		pixmap = Gui::Util::pixmap("tool_dark_grey");
		pixmap_disabled = Gui::Util::pixmap("tool_disabled");
		icon.addPixmap(pixmap, QIcon::Normal, QIcon::On);
		icon.addPixmap(pixmap, QIcon::Normal, QIcon::Off);
		icon.addPixmap(pixmap_disabled, QIcon::Disabled, QIcon::On);
		icon.addPixmap(pixmap_disabled, QIcon::Disabled, QIcon::Off);
		icon.addPixmap(pixmap, QIcon::Active, QIcon::On);
		icon.addPixmap(pixmap, QIcon::Active, QIcon::Off);
		icon.addPixmap(pixmap, QIcon::Selected, QIcon::On);
		icon.addPixmap(pixmap, QIcon::Selected, QIcon::Off);
	}

	else{
		icon = Gui::Icons::icon(Gui::Icons::Close);
		if(icon.isNull()){
			icon = Gui::Icons::icon(Gui::Icons::Exit);
		}
	}


	this->setIcon(icon);
	this->update();
}


void PluginCloseButton::skin_changed()
{
	set_std_icon();
}

