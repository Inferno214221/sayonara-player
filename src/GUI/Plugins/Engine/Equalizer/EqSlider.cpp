/* EqSlider.cpp */

/* Copyright (C) 2011-2017  Lucio Carreras
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

#include "EqSlider.h"

#include <QKeyEvent>
#include <QSize>

EqSlider::EqSlider(QWidget *parent) :
	Gui::Slider(parent)
{
	_idx = -1;

	this->setMaximum(24);
	this->setMinimum(-24);
	this->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
}

void  EqSlider::setData(int idx, QLabel* label){
	_label = label;
	_idx = idx;
}

QLabel* EqSlider::getLabel() const
{
	return _label;
}

int EqSlider::getIndex() const
{
	return _idx;
}


void EqSlider::sliderChange(SliderChange change)
{
	Gui::Slider::sliderChange(change);

	if(change == QAbstractSlider::SliderValueChange){
		emit sig_value_changed(_idx, this->get_eq_value());
	}
}


double EqSlider::get_eq_value() const
{
	int val = this->value();
	if( val > 0 ){
		return (val) / 1.0;
	}

	else {
		return (val / 2.0);
	}
}

QSize EqSlider::minimumSizeHint() const
{
	return QSize(10, 50);
}


void EqSlider::keyPressEvent(QKeyEvent* e)
{
	if(e->key() == Qt::Key_0){
		this->setValue(0);
		emit sig_value_changed(_idx, this->get_eq_value());
	}

	else{
		Gui::Slider::keyPressEvent(e);
	}

}
