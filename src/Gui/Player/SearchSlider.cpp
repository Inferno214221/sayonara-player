/* SearchSlider.cpp

 * Copyright (C) 2011-2020 Michael Lugmair (Lucio Carreras)  
 *
 * This file is part of sayonara-player
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * created by Michael Lugmair (Lucio Carreras), 
 * Sep 14, 2012 
 *
 */

#include "SearchSlider.h"

#include <QColor>
#include <QEvent>
#include <QWheelEvent>

#include <algorithm>

using Gui::SearchSlider;

struct SearchSlider::Private
{
	int buffer_progress;

	Private() :
		buffer_progress(-1) {}
};

SearchSlider::SearchSlider(QWidget* parent) :
	Gui::Slider(parent)
{
	m = Pimpl::make<Private>();
	this->setMouseTracking(true);
}

SearchSlider::~SearchSlider() = default;

bool SearchSlider::event(QEvent* e)
{
	if(e->type() == QEvent::Wheel && this->isEnabled())
	{
		auto* we = dynamic_cast<QWheelEvent*>(e);

		int deltaVal = 5;
		if(we->modifiers() & Qt::ShiftModifier)
		{
			deltaVal = 10;
		}

		else if(we->modifiers() & Qt::AltModifier)
		{
			deltaVal = 50;
		}

		if(we->angleDelta().y() > 0)
		{
			setValue(value() + deltaVal);
		}
		else
		{
			setValue(value() - deltaVal);
		}

		emitNewValue(value());
	}

	return Gui::Slider::event(e);
}

bool SearchSlider::hasAdditionalValue() const
{
	return (m->buffer_progress >= 0);
}

int SearchSlider::additionalValue() const
{
	return (m->buffer_progress * (this->maximum() - this->minimum())) / 100;
}

QColor SearchSlider::additionalValueColor() const
{
	return QColor(66, 78, 114);
}

void SearchSlider::set_buffering(int progress)
{
	m->buffer_progress = progress;
	this->repaint();
}

void SearchSlider::mousePressEvent(QMouseEvent* e)
{
	Gui::Slider::mousePressEvent(e);
	emitNewValue(this->value());
}

void SearchSlider::mouseReleaseEvent(QMouseEvent* e)
{
	Gui::Slider::mouseReleaseEvent(e);
	emitNewValue(this->value());
}

void SearchSlider::mouseMoveEvent(QMouseEvent* e)
{
	Gui::Slider::mouseMoveEvent(e);
	if(this->isSliderDown())
	{
		emitNewValue(this->value());
	}
}

void SearchSlider::emitNewValue(int value)
{
	value = std::max(value, 0);
	value = std::min(value, maximum());

	emit sig_slider_moved(value);
}

bool SearchSlider::is_busy() const
{
	return this->isSliderDown();
}

