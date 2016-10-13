/* SayonaraLoadingBar.cpp */

/* Copyright (C) 2011-2016  Lucio Carreras
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



/* SayonaraLoadingBar.cpp */

#include "SayonaraLoadingBar.h"

SayonaraLoadingBar::SayonaraLoadingBar(QWidget* parent) :
	QProgressBar(parent)
{
	_parent = parent;

	_fixed_height = 5;

	this->setEnabled(false);
	this->setObjectName("loading_bar");

	this->setMinimum(0);
	this->setMaximum(0);
	this->setVisible(false);
}

SayonaraLoadingBar::~SayonaraLoadingBar() {}

void SayonaraLoadingBar::showEvent(QShowEvent* e)
{
	this->setGeometry(0,
					  _parent->height() - _fixed_height,
					  _parent->width(),
					  _fixed_height);

	QProgressBar::showEvent(e);
}
