/* Splitter.cpp
 *
 * Copyright (C) 2011-2024 Michael Lugmair
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

#include "Splitter.h"
#include "Gui/Utils/EventFilter.h"
#include <QMouseEvent>

struct Gui::Splitter::Private
{
	bool handleEnabled;

	Private() :
		handleEnabled(true)
	{}
};

Gui::Splitter::Splitter(QWidget* parent) :
	QSplitter(parent)
{
	m = Pimpl::make<Private>();
}

Gui::Splitter::~Splitter() = default;

void Gui::Splitter::setHandleEnabled(bool b)
{
	m->handleEnabled = b;
}

bool Gui::Splitter::isHandleEnabled() const
{
	return m->handleEnabled;
}

QSplitterHandle* Gui::Splitter::createHandle()
{
	return new Gui::SplitterHandle(this->orientation(), this);
}

void Gui::SplitterHandle::mouseMoveEvent(QMouseEvent* e)
{
	auto* splitter = dynamic_cast<Gui::Splitter*>(this->splitter());
	if(splitter && !splitter->isHandleEnabled())
	{
		return;
	}

	else
	{
		QSplitterHandle::mouseMoveEvent(e);
	}
}
