/* TagLineEdit.h */

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

#ifndef TAGLINEEDIT_H
#define TAGLINEEDIT_H

#include "Utils/Pimpl.h"
#include <QLineEdit>

struct TextSelection;

/**
 * @brief The TagLineEdit class
 * @ingroup Tagging
 */
class TagLineEdit :
		public QLineEdit
{
	Q_OBJECT
	PIMPL(TagLineEdit)

public:
	explicit TagLineEdit(QWidget* parent=nullptr);
	~TagLineEdit();

protected:
	/**
	 * @brief resets the TextSelection
	 * @param e
	 */
	void focusInEvent(QFocusEvent* e) override;

	/**
	 * @brief if selectected_text is empty, TextSelection is resetted. Else the new TextSelection is set.
	 * @param e
	 */
	void focusOutEvent(QFocusEvent* e) override;

public:
	/**
	 * @brief Retrieve the current TextSelection
	 * @return The current TextSelection object
	 */
	TextSelection text_selection() const;
};

#endif // TAGLINEEDIT_H
