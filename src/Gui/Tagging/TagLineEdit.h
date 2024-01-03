/* TagLineEdit.h */

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

#ifndef TAGLINEEDIT_H
#define TAGLINEEDIT_H

#include <QLineEdit>

struct TextSelection;

/**
 * @brief The TagLineEdit class
 * @ingroup GuiTagging
 */
class TagLineEdit :
	public QLineEdit
{
	Q_OBJECT

	public:
		/**
		 * @brief Holds start and size of a selection
		 * @ingroup GuiTagging
		 */
		struct TextSelection
		{
			int selectionStart {-1};
			int selectionSize {0};
		};

		explicit TagLineEdit(QWidget* parent = nullptr);
		~TagLineEdit() override;

	public:
		/**
		 * @brief Retrieve the current TextSelection
		 * @return The current TextSelection object
		 */
		TextSelection textSelection() const;
};

#endif // TAGLINEEDIT_H
