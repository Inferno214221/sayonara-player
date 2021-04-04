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

#include "Utils/Settings/Settings.h"
#include "Utils/FileUtils.h"
#include "Utils/StandardPaths.h"

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

QString Style::currentStyle()
{
	return style(isDark());
}

bool Style::isDark()
{
	return (GetSetting(Set::Player_Style) == 1);
}

void Style::setDark(bool dark)
{
	SetSetting(Set::Player_Style, dark ? 1 : 0);
}