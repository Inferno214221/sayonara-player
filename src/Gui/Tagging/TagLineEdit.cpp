/* TagLineEdit.cpp */

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

#include "TagLineEdit.h"
#include "TextSelection.h"

#include <QFocusEvent>

struct TagLineEdit::Private
{
	TextSelection text_selection;
};

TagLineEdit::TagLineEdit(QWidget* parent) :
	QLineEdit(parent)
{
	m = Pimpl::make<Private>();
	this->setReadOnly(true);
}

TagLineEdit::~TagLineEdit() = default;

void TagLineEdit::focusInEvent(QFocusEvent* e)
{
	m->text_selection.reset();
	QLineEdit::focusInEvent(e);
}

void TagLineEdit::focusOutEvent(QFocusEvent* e)
{
	QString selected_text = this->selectedText();
	if(selected_text.isEmpty())
	{
		m->text_selection.reset();
	}

	else
	{
		m->text_selection.selection_start = this->selectionStart();
		m->text_selection.selection_size = selected_text.size();
	}

	QLineEdit::focusOutEvent(e);
}


TextSelection TagLineEdit::text_selection() const
{
	QString selected_text = this->selectedText();
	if(selected_text.isEmpty())
	{
		m->text_selection.reset();
	}

	else
	{
		m->text_selection.selection_start = this->selectionStart();
		m->text_selection.selection_size = selected_text.size();
	}

	return m->text_selection;
}

