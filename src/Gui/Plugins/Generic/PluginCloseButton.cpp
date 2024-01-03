/* PluginCloseButton.cpp */

/* Copyright (C) 2011-2024 Michael Lugmair (Lucio Carreras)
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
#include "Utils/Language/Language.h"

#include <QEvent>

PluginCloseButton::PluginCloseButton(QWidget* parent) :
	Gui::WidgetTemplate<QPushButton>(parent)
{
	this->setFlat(true);
	this->setIconSize(QSize(14, 14));

	this->setStyleSheet("margin-left: 2px; margin-right: 2px; padding-left: 0px; padding-right: 0px;");
	this->setToolTip(Lang::get(Lang::Close));
}

PluginCloseButton::~PluginCloseButton() = default;

void PluginCloseButton::enterEvent(QEvent* e)
{
	using namespace Gui;

	QPushButton::enterEvent(e);

	QIcon icon;

	if(Style::isDark())
	{
		icon = Util::icon("tool_grey", Util::NoTheme);
	}

	else
	{
		icon = Icons::icon(Icons::Close);
		if(icon.isNull())
		{
			icon = Icons::icon(Icons::Exit);
		}
	}

	if(this->isEnabled())
	{
		this->setIcon(icon);
		e->accept();
	}
}

void PluginCloseButton::leaveEvent(QEvent* e)
{
	QPushButton::leaveEvent(e);

	setStandardIcon();
}

void PluginCloseButton::setStandardIcon()
{
	using namespace Gui;

	QIcon icon;
	QPixmap pixmap;
	QPixmap pixmapDisabled;

	if(Style::isDark())
	{
		pixmap = Util::pixmap("tool_dark_grey", Util::NoTheme);
		pixmapDisabled = Util::pixmap("tool_disabled", Util::NoTheme);
		icon.addPixmap(pixmap, QIcon::Normal, QIcon::On);
		icon.addPixmap(pixmap, QIcon::Normal, QIcon::Off);
		icon.addPixmap(pixmapDisabled, QIcon::Disabled, QIcon::On);
		icon.addPixmap(pixmapDisabled, QIcon::Disabled, QIcon::Off);
		icon.addPixmap(pixmap, QIcon::Active, QIcon::On);
		icon.addPixmap(pixmap, QIcon::Active, QIcon::Off);
		icon.addPixmap(pixmap, QIcon::Selected, QIcon::On);
		icon.addPixmap(pixmap, QIcon::Selected, QIcon::Off);
	}

	else
	{
		icon = icon.isNull() ? Icons::icon(Icons::Exit) : Icons::icon(Icons::Close);

	}

	this->setIcon(icon);
	this->update();
}

void PluginCloseButton::skinChanged()
{
	setStandardIcon();
}

