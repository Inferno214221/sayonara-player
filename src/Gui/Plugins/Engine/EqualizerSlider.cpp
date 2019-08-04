/* EqualizerSlider.cpp */

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

#include "EqualizerSlider.h"

#include <QLabel>
#include <QShortcut>

using Gui::EqualizerSlider;

struct EqualizerSlider::Private
{
	QLabel* label=nullptr;
	int		idx;

	Private() :
		idx(-1)
	{}
};

EqualizerSlider::EqualizerSlider(QWidget *parent) :
	Gui::Slider(parent)
{
	m = Pimpl::make<Private>();

	this->setMaximum(24);
	this->setMinimum(-24);
	this->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);

	new QShortcut(QKeySequence(Qt::Key_0), this, SLOT(set_zero()), nullptr, Qt::WidgetShortcut);
}

EqualizerSlider::~EqualizerSlider() {}

void EqualizerSlider::set_label(int idx, QLabel* label)
{
	m->label = label;
	m->idx = idx;
}

QLabel* EqualizerSlider::label() const
{
	return m->label;
}

int EqualizerSlider::index() const
{
	return m->idx;
}

void EqualizerSlider::sliderChange(SliderChange change)
{
	Gui::Slider::sliderChange(change);

	if(change == QAbstractSlider::SliderValueChange)
	{
		emit sig_value_changed(m->idx, eq_value());
	}
}


double EqualizerSlider::eq_value() const
{
	int val = this->value();
	if( val > 0 ){
		return (val) / 1.0;
	}

	else {
		return (val / 2.0);
	}
}

void EqualizerSlider::set_zero()
{
	this->setValue(0);
	emit sig_value_changed(m->idx, eq_value());
}

QSize EqualizerSlider::minimumSizeHint() const
{
	return QSize(10, 50);
}

