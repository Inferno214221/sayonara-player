/* Slider.cpp */

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

#include "Slider.h"
#include "Gui/Utils/Style.h"
#include "Gui/Utils/GuiUtils.h"

#include <QPainter>
#include <QMouseEvent>

using Gui::Slider;

struct Slider::Private
{
	bool hovered;

	Private() :
		hovered(false)
	{}
};

Slider::Slider(QWidget* parent) :
	QSlider(parent)
{
	m = Pimpl::make<Private>();

	this->setTracking(true);
	this->setMouseTracking(true);
	this->setSingleStep(1);
	this->setPageStep(1);
}

Slider::~Slider() = default;

bool Slider::event(QEvent* e){
	/** We need this for activate an item as soon it is hovered.
	Otherwise, the curve functionality with the mouse wheel event does not work **/
	switch(e->type())
	{
		case QEvent::HoverEnter:
			m->hovered = true;
			emit sigSliderGotFocus();
			break;

		case QEvent::HoverLeave:
			m->hovered = false;
			if(!this->hasFocus()){
				emit sigSliderLostFocus();
			}

			break;

		default: 
			break;
	}

	return QSlider::event(e);
}


void Slider::focusInEvent(QFocusEvent* e){
	QSlider::focusInEvent(e);
	emit sigSliderGotFocus();
}

void Slider::focusOutEvent(QFocusEvent* e){
	QSlider::focusOutEvent(e);
	emit sigSliderLostFocus();
}

void Slider::mousePressEvent(QMouseEvent* e)
{
	this->setSliderDown(true);

	int new_val = valueFromPosition(e->pos());
	setValue(new_val);
}

void Slider::mouseReleaseEvent(QMouseEvent* e)
{
	int new_val = valueFromPosition(e->pos());
	setValue(new_val);

	this->setSliderDown(false);
}

bool Slider::hasAdditionalValue() const
{
	return false;
}

int Slider::additionalValue() const
{
	return -1;
}

QColor Slider::additionalValueColor() const
{
	return QColor(0, 0, 0);
}

void Slider::mouseMoveEvent(QMouseEvent* e)
{
	int new_val = valueFromPosition(e->pos());

	if(this->isSliderDown())
	{
		setValue(new_val);
	}

	else
	{
		emit sigSliderHovered(new_val);
	}
}

void Slider::sliderChange(SliderChange change){
	QSlider::sliderChange(change);
}

int Slider::valueFromPosition(const QPoint& pos) const
{
	int percent;
	if(this->orientation() == Qt::Vertical){
		percent = 100 - (pos.y() * 100) / geometry().height();
	}

	else{
		percent = (pos.x() * 100) / geometry().width();
	}

	int range = this->maximum() - this->minimum();
	return  ( range * percent) / 100 + this->minimum();
}

static QRect calc_rect(QSlider* slider, int value, bool is_horizontal)
{
	int long_side = slider->width();
	int short_side = slider->height();	
	int rect_thickness = Gui::Util::textWidth(slider->fontMetrics(), "m") / 4;

	if(!is_horizontal){
		long_side = slider->height();
		short_side = slider->width();
	}

	int h = rect_thickness;
	int w = long_side - 4;
	int x = 2;
	int y = (short_side - h) / 2;

	int h_rect = h;
	int percent = ((value - slider->minimum()) * 10000) / (slider->maximum() - slider->minimum());
	int w_rect = (w * percent) / 10000;
	int x_rect = x;
	int y_rect = y + (h - h_rect) / 2;

	QRect ret(x_rect, y_rect, w_rect, h_rect);
	if(!is_horizontal)
	{
		ret = QRect(y_rect, long_side - w_rect, h_rect, w_rect);
	}

	return ret;
}

#include "Utils/Logger/Logger.h"
void Slider::paintEvent(QPaintEvent* e)
{
	if(!Style::isDark())
	{
		QSlider::paintEvent(e);
		return;
	}

	bool is_horizontal = (this->orientation() == Qt::Horizontal);

	using RectColorPair=QPair<QRect, QColor>;
	QList<RectColorPair> rects;

	QRect rect_dark = calc_rect(this, this->maximum(), is_horizontal);
	rects << RectColorPair(rect_dark, QColor(42, 42, 42));

	if(this->hasAdditionalValue())
	{
		int other_value = this->additionalValue();

		spLog(Log::Info, this) << "value: " << this->value() << " buffer: " << this->additionalValue();
		QRect rect = calc_rect(this, other_value, is_horizontal);
		rects << RectColorPair(rect, this->additionalValueColor());
	}

	QRect rect_orange = calc_rect(this, this->value(), is_horizontal);
	rects << RectColorPair(rect_orange, QColor(243, 132, 26));
	//rects << RectColorPair(rect_orange, QColor(66, 78, 114));

	QPainter painter(this);

	if(m->hovered)
	{
		QColor color_light(72,72,72);
		painter.setPen(color_light);

		QPainterPath path;
		path.addRoundedRect(this->rect(), 3, 3);
		painter.fillPath(path, color_light);
		painter.drawPath(path);
	}

	for(const RectColorPair& rcp : rects)
	{
		QRect rect = rcp.first;
		QColor color = rcp.second;
		painter.setPen(color);
		painter.drawRect(rect);
		painter.fillRect(rect, color);
	}
}

