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
#include "Utils/StandardPaths.h"

#include <QApplication>
#include <QColor>
#include <QFont>
#include <QFontMetrics>
#include <QMainWindow>
#include <QPalette>
#include <QStyle>
#include <QToolTip>

namespace
{
	int getFontSize(QApplication* app)
	{
		const auto fontSize = app->font().pointSize();
		static auto defaultFontSize = (fontSize > 0) ? fontSize : 12;

		const auto scalingFactor = GetSetting(Set::Player_ScalingFactor);

		return static_cast<int>(scalingFactor * defaultFontSize);
	}

	QString getFontStyleSheet(QApplication* app)
	{
		return QString(R"(
			QWidget
			{
				font-size: %1px;
			}

			Library--View,
			Library--ItemView,
			Library--GenreView
			{
				font-weight: %2;
			}
		)")
			.arg(getFontSize(app))
			.arg(GetSetting(Set::Lib_FontBold) ? "600" : "normal");
	}
}

QString Style::style(bool dark)
{
	QString style;
	if(dark)
	{
		Util::File::readFileIntoString(":/Style/dark.css", style);
	}

	const auto additionalPath = (dark)
	                            ? Util::xdgConfigPath("dark.css")
	                            : Util::xdgConfigPath("standard.css");

	if(Util::File::exists(additionalPath))
	{
		QString additionalStyle;
		Util::File::readFileIntoString(additionalPath, additionalStyle);
		style += "\n" + additionalStyle.trimmed();
	}

	return style;
}

QFont Style::currentFont()
{
	return QApplication::font();
}

QString Style::currentStyle()
{
	return style(isDark());
}

int Style::recommendedHeight()
{
	return QFontMetrics(currentFont()).height();
}

bool Style::isDark()
{
	return (GetSetting(Set::Player_Style) == 1);
}

void Style::setDark(bool dark)
{
	SetSetting(Set::Player_Style, dark ? 1 : 0);
}

void Style::applyCurrentStyle(QApplication* app, QMainWindow* player)
{
	const auto style = Style::currentStyle();
	const auto fontStyle = getFontStyleSheet(app);

    player->setStyleSheet(fontStyle + '\n' + style);

	if(Style::isDark())
	{
		auto palette = QToolTip::palette();
		palette.setBrush(QPalette::ColorGroup::Inactive, QPalette::ColorRole::ToolTipBase, QColor(66, 78, 114));
		palette.setColor(QPalette::ColorGroup::Inactive, QPalette::ColorRole::ToolTipText, QColor(0, 0, 0));
		QToolTip::setPalette(palette);
	}

	else
	{
		QToolTip::setPalette(app->palette());
	}
}
