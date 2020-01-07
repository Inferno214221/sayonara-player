/* TagTextInput.cpp */

/* Copyright (C) 2011-2020  Lucio Carreras
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

#include "Utils/Utils.h"
#include "TagTextInput.h"

#include <QMenu>
#include <QAction>
#include <QContextMenuEvent>
#include <QCursor>

struct TagTextInput::Private
{
	QMenu*			context_menu=nullptr;
	QAction*		action_cvt_to_first_upper=nullptr;
	QAction*		action_cvt_to_very_first_upper=nullptr;
};

TagTextInput::TagTextInput(QWidget* parent) :
    WidgetTemplate<QLineEdit>(parent)
{
	m = Pimpl::make<Private>();
	init_context_menu();
}

TagTextInput::~TagTextInput() = default;

void TagTextInput::init_context_menu() 
{
    m->context_menu = createStandardContextMenu();

//	_context_menu = new QMenu(this);
	m->action_cvt_to_very_first_upper = new QAction("Convert to very first upper", m->context_menu);
	m->action_cvt_to_first_upper = new QAction("Convert to first upper", m->context_menu);

	m->context_menu->addSeparator();
	m->context_menu->addAction(m->action_cvt_to_very_first_upper);
	m->context_menu->addAction(m->action_cvt_to_first_upper);

	connect(m->action_cvt_to_first_upper, &QAction::triggered, this, &TagTextInput::cvt_to_first_upper);
	connect(m->action_cvt_to_very_first_upper, &QAction::triggered, this, &TagTextInput::cvt_to_very_first_upper);
}

void TagTextInput::contextMenuEvent(QContextMenuEvent* event){
	QPoint pos = event->pos();

	pos.setX(QCursor::pos().x());	
	pos.setY(QCursor::pos().y());	
	m->context_menu->exec(pos);
}

void TagTextInput::keyPressEvent(QKeyEvent* event)
{
	WidgetTemplate<QLineEdit>::keyPressEvent(event);

	if(event->key() == Qt::Key_Up)
	{
		if(this->text() == Util::cvt_str_to_first_upper(this->text()))
		{
			this->setText(text().toUpper());
		}

		else if(this->text() == Util::cvt_str_to_very_first_upper(this->text()))
		{
			cvt_to_first_upper();
		}

		else
		{
			cvt_to_very_first_upper();
		}
	}

	else if(event->key() == Qt::Key_Down)
	{
		if(this->text() == Util::cvt_str_to_very_first_upper(this->text()))
		{
			this->setText(this->text().toLower());
		}

		else if(this->text() == Util::cvt_str_to_first_upper(this->text()))
		{
			cvt_to_very_first_upper();
		}

		else
		{
			cvt_to_first_upper();
		}
	}
}

void TagTextInput::cvt_to_first_upper()
{
	QString text = this->text();
	text = Util::cvt_str_to_first_upper(text);
	this->setText(text);
}

void TagTextInput::cvt_to_very_first_upper()
{
	QString text = this->text();
	text = Util::cvt_str_to_very_first_upper(text);
	this->setText(text);
}

void TagTextInput::language_changed()
{
	m->action_cvt_to_very_first_upper->setText(tr("Very first letter to upper case"));
	m->action_cvt_to_first_upper->setText(tr("First letters to upper case"));
}
