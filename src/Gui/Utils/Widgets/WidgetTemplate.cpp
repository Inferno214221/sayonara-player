/* WidgetTemplate.cpp */

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

/* Nothing in here */

#include "WidgetTemplate.h"
#include "Utils/Settings/Settings.h"

Gui::AbstrWidgetTemplate::AbstrWidgetTemplate(QObject* parent, WidgetTemplateParent* wtp) :
	QObject(parent)
{
	_wtp = wtp;

	ListenSetting(Set::Player_Language, AbstrWidgetTemplate::language_changed);
	ListenSetting(Set::Player_Style, AbstrWidgetTemplate::skin_changed);
	ListenSettingNoCall(Set::Player_FontName, AbstrWidgetTemplate::skin_changed);
	ListenSettingNoCall(Set::Player_FontSize, AbstrWidgetTemplate::skin_changed);
	ListenSettingNoCall(Set::PL_FontSize, AbstrWidgetTemplate::skin_changed);
	ListenSettingNoCall(Set::Lib_FontSize, AbstrWidgetTemplate::skin_changed);
	ListenSettingNoCall(Set::Lib_FontBold, AbstrWidgetTemplate::skin_changed);
	ListenSettingNoCall(Set::Icon_Theme, AbstrWidgetTemplate::skin_changed);
	ListenSettingNoCall(Set::Icon_ForceInDarkTheme, AbstrWidgetTemplate::skin_changed);
}

Gui::AbstrWidgetTemplate::~AbstrWidgetTemplate() {}

void Gui::AbstrWidgetTemplate::language_changed() { _wtp->language_changed(); }

void Gui::AbstrWidgetTemplate::skin_changed() { _wtp->skin_changed(); }


Gui::WidgetTemplateParent::WidgetTemplateParent() {}
Gui::WidgetTemplateParent::~WidgetTemplateParent() {}
void Gui::WidgetTemplateParent::language_changed() {}
void Gui::WidgetTemplateParent::skin_changed() {}
