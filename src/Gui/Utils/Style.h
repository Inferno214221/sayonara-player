/* Style.h */

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


/*
 * Style.h
 *
 *  Created on: Oct 27, 2011
 *      Author: Michael Lugmair (Lucio Carreras)
 */

#ifndef STYLE_H_
#define STYLE_H_

#include <QString>

class QFont;
class QApplication;
class QMainWindow;

/**
 * @define the Style namespace
 * @ingroup Style
 */
namespace Style
{
	/**
	 * @brief fetch the stylesheet file formatted as string
	 * @ingroup Style
	 * @return string formatted stylesheet file
	 */
	QString style(bool dark);

	/**
	 * @brief stylesheet with all replace variables
	 * @ingroup Style
	 * @return
	 */
	QString currentStyle();

	/**
	 * @brief is_dark
	 * @ingroup Style
	 * @return
	 */
	bool isDark();

	/**
	 * @brief set_dark
	 * @ingroup Style
	 * @param b
	 */
	void setDark(bool b);

	template<typename T>
	void applyCurrentStyle(T* instance)
	{
		const auto styleSheet = instance->styleSheet();
		if(styleSheet != currentStyle())
		{
			instance->setStyleSheet(currentStyle());
		}
	}
}

#endif /* STYLE_H_ */
