/* ProgressBar.cpp */

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

#include "ProgressBar.h"

using Gui::ProgressBar;

struct Gui::ProgressBar::Private
{
	QWidget*	parent=nullptr;
	int			fixed_height;
	Position	position;

	Private(QWidget* parent) :
		parent(parent),
		fixed_height(5),
		position(ProgressBar::Position::Bottom)
	{}
};


ProgressBar::ProgressBar(QWidget* parent) :
	Gui::WidgetTemplate<QProgressBar>(parent)
{
	m = Pimpl::make<Private>(parent);

	this->setEnabled(false);
	this->setObjectName("loading_bar");

	this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	this->setMaximumHeight(m->fixed_height);
	this->setMinimum(0);
	this->setMaximum(0);

	skin_changed();
}

ProgressBar::~ProgressBar() = default;

void ProgressBar::set_position(ProgressBar::Position o)
{
	m->position = o;
}

void ProgressBar::refresh()
{
	int y;
	switch(m->position)
	{
		case ProgressBar::Position::Top:
			 y = 2;
			break;
		case ProgressBar::Position::Middle:
			y = (parentWidget()->height() - m->fixed_height) / 2;
			break;
		case ProgressBar::Position::Bottom:
		default:
			 y = parentWidget()->height() - m->fixed_height - 2;
			break;
	}

	this->setGeometry
	(
		2,
		y,
		parentWidget()->width() - 4,
		m->fixed_height
	);
}

void ProgressBar::showEvent(QShowEvent* e)
{
	QProgressBar::showEvent(e);

	refresh();
}

void ProgressBar::skin_changed()
{
	m->fixed_height = std::max((fontMetrics().height()) * 10 / 30, 8);
}
