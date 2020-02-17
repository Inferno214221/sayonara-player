/* EventFilter.cpp */

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

#include "EventFilter.h"

#include <QAction>
#include <QContextMenuEvent>
#include <QKeyEvent>

using namespace Gui;

KeyPressFilter::KeyPressFilter(QObject* parent) :
	QObject(parent)
{}


bool KeyPressFilter::eventFilter(QObject *o, QEvent *e)
{
	if(e->type() == QEvent::KeyPress)
	{
		auto* ke = static_cast<QKeyEvent*>(e);
		ke->accept();
		emit setKeyPressed(ke->key());
	}

	return QObject::eventFilter(o, e);
}



ContextMenuFilter::ContextMenuFilter(QObject* parent) :
	QObject(parent)
{}

bool ContextMenuFilter::eventFilter(QObject *o, QEvent *e)
{
	if(e->type() == QEvent::ContextMenu)
	{
		e->accept();

		auto* cme = static_cast<QContextMenuEvent*>(e);
		emit sigContextMenu(cme->globalPos(), nullptr);

		return true;
	}

	return QObject::eventFilter(o, e);
}


MouseMoveFilter::MouseMoveFilter(QObject* parent) :
	QObject(parent)
{}

bool MouseMoveFilter::eventFilter(QObject *o, QEvent *e)
{
	if(e->type() == QEvent::MouseMove)
	{
		e->accept();

		emit sigMouseMoved(static_cast<QMouseEvent*>(e));
		return true;
	}

	return QObject::eventFilter(o, e);
}


MousePressedFilter::MousePressedFilter(QObject* parent) :
	QObject(parent)
{}

bool MousePressedFilter::eventFilter(QObject* o, QEvent* e)
{
	if(e->type() == QEvent::MouseButtonPress)
	{
		e->accept();
		emit sigMousePressed(static_cast<QMouseEvent*>(e));
	}

	return QObject::eventFilter(o, e);
}


MouseEnterFilter::MouseEnterFilter(QObject* parent) :
	QObject(parent)
{}

bool MouseEnterFilter::eventFilter(QObject *o, QEvent *e)
{
	if(e->type() == QEvent::Enter)
	{
		e->accept();

		emit sigMouseEntered();
	}

	return QObject::eventFilter(o, e);
}


MouseLeaveFilter::MouseLeaveFilter(QObject* parent) :
	QObject(parent)
{}

bool MouseLeaveFilter::eventFilter(QObject *o, QEvent *e)
{
	if(e->type() == QEvent::Leave)
	{
		e->accept();
		emit sigMouseLeft();
	}

	return QObject::eventFilter(o, e);
}


HideFilter::HideFilter(QObject* parent) : QObject(parent) {}

bool HideFilter::eventFilter(QObject* o, QEvent* e)
{
	bool success = QObject::eventFilter(o, e);

	if(e->type() == QEvent::Hide)
	{
		emit sigHidden();
	}

	return success;
}

ShowFilter::ShowFilter(QObject* parent) : QObject(parent) {}

bool ShowFilter::eventFilter(QObject* o, QEvent* e)
{
	bool success = QObject::eventFilter(o, e);

	if(e->type() == QEvent::Show)
	{
		emit sigShown();
	}

	return success;
}


ResizeFilter::ResizeFilter(QObject* parent) : QObject(parent) {}

bool ResizeFilter::eventFilter(QObject* o, QEvent* e)
{
	bool success = QObject::eventFilter(o, e);

	if(e->type() == QEvent::Resize)
	{
		auto* re = dynamic_cast<QResizeEvent*>(e);
		emit sigResized(re->size());
	}

	return success;
}


PaintFilter::PaintFilter(QObject* parent) : QObject(parent) {}

bool PaintFilter::eventFilter(QObject* o, QEvent* e)
{
	bool success = QObject::eventFilter(o, e);

	if(e->type() == QEvent::Paint)
	{
		emit sigPainted();
	}

	return success;
}

GenericFilter::GenericFilter(const QEvent::Type& type, QObject* parent) :
	QObject(parent)
{
	m_types << type;
}

GenericFilter::GenericFilter(const QList<QEvent::Type>& types, QObject* parent) :
	QObject(parent)
{
	m_types = types;
}

bool GenericFilter::eventFilter(QObject* o, QEvent* e)
{
	bool success = QObject::eventFilter(o, e);

	if(m_types.contains(e->type()))
	{
		emit sigEvent(e->type());
	}

	return success;
}
