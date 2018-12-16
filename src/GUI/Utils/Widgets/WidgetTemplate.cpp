/* WidgetTemplate.cpp */

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

/* Nothing in here */

#include "WidgetTemplate.h"
#include "Utils/Settings/Settings.h"

Gui::AbstrWidgetTemplate::AbstrWidgetTemplate(QObject* parent, WidgetTemplateParent* wtp) :
	QObject(parent),
	SayonaraClass()
{
	_wtp = wtp;

	Set::listen<Set::Player_Language>(this, &AbstrWidgetTemplate::language_changed);
	Set::listen<Set::Player_Style>(this, &AbstrWidgetTemplate::skin_changed);
	Set::listen<Set::Player_FontName>(this, &AbstrWidgetTemplate::skin_changed, false);
	Set::listen<Set::Player_FontSize>(this, &AbstrWidgetTemplate::skin_changed, false);
	Set::listen<Set::PL_FontSize>(this, &AbstrWidgetTemplate::skin_changed, false);
	Set::listen<Set::Lib_FontSize>(this, &AbstrWidgetTemplate::skin_changed, false);
	Set::listen<Set::Lib_FontBold>(this, &AbstrWidgetTemplate::skin_changed, false);
	Set::listen<Set::Icon_Theme>(this, &AbstrWidgetTemplate::skin_changed, false);
	Set::listen<Set::Icon_ForceInDarkTheme>(this, &AbstrWidgetTemplate::skin_changed, false);
}

Gui::AbstrWidgetTemplate::~AbstrWidgetTemplate() {}

void Gui::AbstrWidgetTemplate::language_changed() { _wtp->language_changed(); }

void Gui::AbstrWidgetTemplate::skin_changed() { _wtp->skin_changed(); }


Gui::WidgetTemplateParent::WidgetTemplateParent() :
	SayonaraClass()
{}

Gui::WidgetTemplateParent::~WidgetTemplateParent() {}
void Gui::WidgetTemplateParent::language_changed() {}
void Gui::WidgetTemplateParent::skin_changed() {}
