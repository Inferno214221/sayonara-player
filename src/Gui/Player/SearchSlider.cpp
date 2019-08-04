/* SearchSlider.cpp

 * Copyright (C) 2011-2019 Lucio Carreras  
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
 * created by Lucio Carreras, 
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

	Private() : buffer_progress(-1) {}
};

SearchSlider::SearchSlider(QWidget* parent) :
	Gui::Slider(parent)
{
	m = Pimpl::make<Private>();
	this->setMouseTracking(true);
}

SearchSlider::~SearchSlider() {}


bool SearchSlider::event(QEvent *e)
{
	QWheelEvent* we;
	int delta_val = 5;

	switch(e->type()){
		case QEvent::Wheel:

			if(!this->isEnabled()) {
				break;
			}

			we = (QWheelEvent*) e;
			if(we->modifiers() & Qt::ShiftModifier){
				delta_val = 10;
			}

			else if(we->modifiers() & Qt::AltModifier){
				delta_val = 50;
			}


			if(we->delta() > 0){
				setValue(value() + delta_val);
			}
			else{
				setValue(value() - delta_val);
			}

			emit_new_val(value());
			break;

		default:
			break;
	}

	return Gui::Slider::event(e);
}

bool SearchSlider::has_other_value() const
{
	return (m->buffer_progress >= 0);
}

int SearchSlider::other_value() const
{
	return (m->buffer_progress * (this->maximum() - this->minimum())) / 100;
}

QColor SearchSlider::other_value_color() const
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
	emit_new_val(this->value());
}


void SearchSlider::mouseReleaseEvent(QMouseEvent* e)
{
	Gui::Slider::mouseReleaseEvent(e);
	emit_new_val(this->value());
}


void SearchSlider::mouseMoveEvent(QMouseEvent *e)
{
	Gui::Slider::mouseMoveEvent(e);
	if(this->isSliderDown()){
		emit_new_val(this->value());
	}
}

void SearchSlider::emit_new_val(int value)
{
	value = std::max(value, 0);
	value = std::min(value, maximum());

	emit sig_slider_moved(value);
}

bool SearchSlider::is_busy() const
{
	return this->isSliderDown();
}

