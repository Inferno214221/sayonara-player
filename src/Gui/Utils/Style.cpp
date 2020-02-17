/* Style.cpp */

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


/*
 * Style.cpp
 *
 *  Created on: Oct 27, 2011
 *      Author: Michael Lugmair (Lucio Carreras)
 */

#include "Style.h"

#include "Utils/Utils.h"
#include "Utils/Settings/Settings.h"
#include "Utils/FileUtils.h"

#include <QApplication>
#include <QColor>
#include <QFont>
#include <QFontMetrics>
#include <QPalette>
#include <QStyle>
#include <QToolTip>

#define NEWLINE "\n";

QString Style::style(bool dark)
{
	QFont std_font =		QApplication::font();
	QString font_family =	GetSetting(Set::Player_FontName);

	int font_size =			GetSetting(Set::Player_FontSize);
	int font_size_lib =		GetSetting(Set::Lib_FontSize);
	int font_size_pl =		GetSetting(Set::PL_FontSize);
	bool lib_bold =			GetSetting(Set::Lib_FontBold);

	if(font_family.isEmpty()){
		font_family = std_font.family();
	}

	if(font_size <= 0){
		font_size = std_font.pointSize();
	}

	if(font_size_lib <= 0){
		font_size_lib = font_size;
	}

	if(font_size_pl <= 0){
		font_size_pl = font_size;
	}

	QString style, additional_style;
	QString path, additional_path;

	if(!dark)
	{
		path = ":/Style/standard.css";
		additional_path = Util::sayonaraPath("standard.css");
	}

	else
	{
		path = ":/Style/dark.css";
		additional_path = Util::sayonaraPath("dark.css");
	}

	Util::File::readFileIntoString(path, style );
	if(Util::File::exists(additional_path))
	{
		QString additional_style;
		Util::File::readFileIntoString(additional_path, additional_style);
		style += "\n" + additional_style.trimmed();
	}

	style.replace("<<FONT_FAMILY>>", font_family);
	style.replace("<<FONT_SIZE>>", QString::number(font_size));
	style.replace("<<FONT_SIZE_LIB>>", QString::number(font_size_lib));
	style.replace("<<FONT_SIZE_PL>>", QString::number(font_size_pl));
	style.replace("<<FONT_WEIGHT_LIB>>", lib_bold ? "600" : "normal");

	return style;
}

QFont Style::currentFont()
{
	QFont std_font = QApplication::font();

	QString font_family =	GetSetting(Set::Player_FontName);
	int font_size =			GetSetting(Set::Player_FontSize);
	int font_size_lib =		GetSetting(Set::Lib_FontSize);
	int font_size_pl =		GetSetting(Set::PL_FontSize);
	bool lib_bold =			GetSetting(Set::Lib_FontBold);

	Q_UNUSED(lib_bold);

	if(font_family.isEmpty()){
		font_family = std_font.family();
	}

	if(font_size <= 0){
		font_size = std_font.pointSize();
	}

	if(font_size_lib <= 0){
		font_size_lib = font_size;
	}

	if(font_size_pl <= 0){
		font_size_pl = font_size;
	}

	return QFont(font_family, font_size);
}

QString Style::currentStyle()
{
	return style( isDark() );
}

int Style::recommendedHeight()
{
	QFontMetrics fm(currentFont());
	int h = fm.height();
	return h;
}

bool Style::isDark()
{
	return (GetSetting(Set::Player_Style) == 1);
}

void Style::setDark(bool dark)
{
	SetSetting(Set::Player_Style, dark ? 1 : 0);
	Set::shout<SetNoDB::Player_MetaStyle>();
}

void Style::applyCurrentStyle(QApplication* app)
{
	app->setStyleSheet(currentStyle());

	QPalette palette;

	if(Style::isDark())
	{
		palette = QToolTip::palette();
		palette.setBrush(QPalette::ColorGroup::Inactive, QPalette::ColorRole::ToolTipBase, QColor(66, 78, 114));
		palette.setColor(QPalette::ColorGroup::Inactive, QPalette::ColorRole::ToolTipText, QColor(0, 0, 0));
	}

	QToolTip::setPalette(palette);
}
