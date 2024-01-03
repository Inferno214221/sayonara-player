/* EqualizerSlider.cpp */

/* Copyright (C) 2011-2024 Michael Lugmair (Lucio Carreras)
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

static QString calculateLabel(int val)
{
	return QString("%1").arg(val);
}

struct EqualizerSlider::Private
{
	QLabel* label = nullptr;
	int index;
	bool silent;

	Private() :
		index(-1),
		silent(false) {}
};

EqualizerSlider::EqualizerSlider(QWidget* parent) :
	Gui::Slider(parent)
{
	m = Pimpl::make<Private>();

	this->setMaximum(24);
	this->setMinimum(-24);
	this->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);

	new QShortcut(QKeySequence(Qt::Key_0), this, SLOT(setZero()), nullptr, Qt::WidgetShortcut);
}

EqualizerSlider::~EqualizerSlider() = default;

void EqualizerSlider::setLabel(int idx, QLabel* label)
{
	m->label = label;
	m->index = idx;
}

QLabel* EqualizerSlider::label() const
{
	return m->label;
}

int EqualizerSlider::index() const
{
	return m->index;
}

void EqualizerSlider::sliderChange(SliderChange change)
{
	Gui::Slider::sliderChange(change);

	if(change == QAbstractSlider::SliderValueChange && !m->silent)
	{
		emit sigValueChanged(m->index, equalizerValue());
	}

	if(this->label())
	{
		this->label()->setText(calculateLabel(equalizerValue()));
	}
}

double EqualizerSlider::equalizerValue() const
{
	int val = this->value();
	return (val > 0) ? (val * 1.0) : (val / 2.0);
}

void Gui::EqualizerSlider::setEqualizerValue(double value)
{
	this->setValue
		(
			(value > 0) ? int(value) : int(value * 2)
		);
}

void EqualizerSlider::setZero()
{
	this->setValue(0);

	if(!m->silent)
	{
		emit sigValueChanged(m->index, equalizerValue());
	}
}

QSize EqualizerSlider::minimumSizeHint() const
{
	return QSize(10, 50);
}

void Gui::EqualizerSlider::setSilent(bool b)
{
	m->silent = b;
}


