/* Style.h */

/* Copyright (C) 2011-2019 Lucio Carreras
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
 *      Author: Lucio Carreras
 */

#ifndef STYLE_H_
#define STYLE_H_

class QString;
class QFont;

/**
 * @define the Style namespace
 * @ingroup GUIHelper
 */
namespace Style
{
	/**
	 * @brief fetch the stylesheet file formatted as string
	 * @return string formatted stylesheet file
	 */
	QString style(bool dark);

	QString current_style();

	QFont current_font();

	int recommended_height();

	bool is_dark();
	void set_dark(bool b);
}

#endif /* STYLE_H_ */
