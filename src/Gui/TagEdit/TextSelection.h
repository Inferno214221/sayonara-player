/* TextSelection.h */

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



#ifndef TEXTSELECTION_H
#define TEXTSELECTION_H

/**
 * @brief Holds start and size of a selection
 * @ingroup Tagging
 */
struct TextSelection
{
	int selection_start;
	int selection_size;

	TextSelection();

	/**
	 * @brief set selection_start to -1 and selection_size to 0
	 */
	void reset();
};

#endif // TEXTSELECTION_H
