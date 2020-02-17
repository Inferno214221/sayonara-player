/* TagTextInput.cpp */

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

#include "Utils/Utils.h"
#include "TagTextInput.h"

#include <QMenu>
#include <QAction>
#include <QContextMenuEvent>
#include <QCursor>

struct TagTextInput::Private
{
	QMenu*			contextMenu=nullptr;
	QAction*		actionConvertToFirstUpper=nullptr;
	QAction*		actionConvertToVeryFirstUpper=nullptr;
};

TagTextInput::TagTextInput(QWidget* parent) :
    WidgetTemplate<QLineEdit>(parent)
{
	m = Pimpl::make<Private>();
	initContextMenu();
}

TagTextInput::~TagTextInput() = default;

void TagTextInput::initContextMenu() 
{
    m->contextMenu = createStandardContextMenu();

//	_context_menu = new QMenu(this);
	m->actionConvertToVeryFirstUpper = new QAction("Convert to very first upper", m->contextMenu);
	m->actionConvertToFirstUpper = new QAction("Convert to first upper", m->contextMenu);

	m->contextMenu->addSeparator();
	m->contextMenu->addAction(m->actionConvertToVeryFirstUpper);
	m->contextMenu->addAction(m->actionConvertToFirstUpper);

	connect(m->actionConvertToFirstUpper, &QAction::triggered, this, &TagTextInput::convertToFirstUpper);
	connect(m->actionConvertToVeryFirstUpper, &QAction::triggered, this, &TagTextInput::convertToVeryFirstUpper);
}

void TagTextInput::contextMenuEvent(QContextMenuEvent* event){
	QPoint pos = event->pos();

	pos.setX(QCursor::pos().x());	
	pos.setY(QCursor::pos().y());	
	m->contextMenu->exec(pos);
}

void TagTextInput::keyPressEvent(QKeyEvent* event)
{
	WidgetTemplate<QLineEdit>::keyPressEvent(event);

	if(event->key() == Qt::Key_Up)
	{
		if(this->text() == Util::stringToFirstUpper(this->text()))
		{
			this->setText(text().toUpper());
		}

		else if(this->text() == Util::stringToVeryFirstUpper(this->text()))
		{
			convertToFirstUpper();
		}

		else
		{
			convertToVeryFirstUpper();
		}
	}

	else if(event->key() == Qt::Key_Down)
	{
		if(this->text() == Util::stringToVeryFirstUpper(this->text()))
		{
			this->setText(this->text().toLower());
		}

		else if(this->text() == Util::stringToFirstUpper(this->text()))
		{
			convertToVeryFirstUpper();
		}

		else
		{
			convertToFirstUpper();
		}
	}
}

void TagTextInput::convertToFirstUpper()
{
	QString text = this->text();
	text = Util::stringToFirstUpper(text);
	this->setText(text);
}

void TagTextInput::convertToVeryFirstUpper()
{
	QString text = this->text();
	text = Util::stringToVeryFirstUpper(text);
	this->setText(text);
}

void TagTextInput::languageChanged()
{
	m->actionConvertToVeryFirstUpper->setText(tr("Very first letter to upper case"));
	m->actionConvertToFirstUpper->setText(tr("First letters to upper case"));
}
