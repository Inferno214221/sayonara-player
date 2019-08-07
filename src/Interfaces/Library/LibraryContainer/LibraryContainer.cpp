/* LibraryContainer.cpp */

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

#include "Interfaces/Library/LibraryContainer/LibraryContainer.h"

#include <QWidget>
#include <QAction>

using Library::Container;

struct Container::Private
{
	QAction*	action=nullptr;
	bool		initialized;

	Private() :
		initialized(false)
	{}
};

Container::Container(QObject* parent) :
	QObject(parent)
{
	m = Pimpl::make<Private>();
}

Container::~Container() {}

void Container::set_name(const QString& name) { Q_UNUSED(name); }

QString Container::display_name() const
{
	return name();
}

QMenu* Container::menu()
{
	return nullptr;
}

void Container::set_menu_action(QAction* action)
{
	m->action = action;
}

QAction* Container::menu_action() const
{
	return m->action;
}

void Container::set_initialized()
{
	m->initialized = true;
}

bool Container::is_initialized() const
{
	return m->initialized;
}

bool Container::is_local() const
{
	return false;
}


void Container::show()
{
	QWidget* own_widget = widget();

	if(own_widget)
	{
		own_widget->setVisible(true);
		QWidget* parent_widget = own_widget->parentWidget();
		if(parent_widget)
		{
			own_widget->resize(parent_widget->size());
		}

		own_widget->update();
	}

	if(menu_action()){
		menu_action()->setText(this->name());
		menu_action()->setVisible(true);
	}
}

void Container::hide()
{
	if(!this->is_initialized()){
		return;
	}

	if(menu_action()){
		menu_action()->setVisible(false);
	}

	if(widget())
	{
		widget()->hide();
	}
}

void Container::retranslate()
{
	if(menu_action())
	{
		menu_action()->setText(display_name());
	}
}



