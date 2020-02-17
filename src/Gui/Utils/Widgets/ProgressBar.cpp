/* ProgressBar.cpp */

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

#include "ProgressBar.h"
#include "Gui/Utils/EventFilter.h"
#include <QAbstractScrollArea>

using Gui::ProgressBar;

struct Gui::ProgressBar::Private
{
	int			fixedHeight;
	Position	position;

	Private() :
		fixedHeight(5),
		position(ProgressBar::Position::Bottom)
	{}
};


ProgressBar::ProgressBar(QWidget* parent) :
	Gui::WidgetTemplate<QProgressBar>(parent)
{
	m = Pimpl::make<Private>();

	this->setEnabled(false);
	this->setObjectName("loading_bar");

	this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	this->setMaximumHeight(m->fixedHeight);
	this->setMinimum(0);
	this->setMaximum(0);

	auto* filter = new ResizeFilter(parent);
	parent->installEventFilter(filter);
	connect(filter, &Gui::ResizeFilter::sigResized, this, &ProgressBar::parentResized);

	skinChanged();
}

ProgressBar::~ProgressBar() = default;

void ProgressBar::setPosition(ProgressBar::Position o)
{
	m->position = o;
}

void ProgressBar::refresh()
{
	QWidget* woi = parentWidget();

	auto* scrollArea = dynamic_cast<QAbstractScrollArea*>(parentWidget());
	if(scrollArea)
	{
		woi = scrollArea->viewport();
	}

	int y;
	switch(m->position)
	{
		case ProgressBar::Position::Top:
			 y = 2;
			break;
		case ProgressBar::Position::Middle:
			y = (woi->height() - m->fixedHeight) / 2;
			break;
		case ProgressBar::Position::Bottom:
		default:
			y = woi->height() - m->fixedHeight - 2;
			break;
	}

	this->setGeometry
	(
		2,
		y,
		woi->width() - 4,
		m->fixedHeight
	);
}

void ProgressBar::parentResized(const QSize&)
{
	refresh();
}

void ProgressBar::showEvent(QShowEvent* e)
{
	QProgressBar::showEvent(e);

	refresh();
}

void ProgressBar::skinChanged()
{
	m->fixedHeight = std::max((fontMetrics().height()) * 10 / 30, 8);
	refresh();
}
