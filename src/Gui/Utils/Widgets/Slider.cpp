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
#include "Utils/Logger/Logger.h"

#include <QPainter>
#include <QPainterPath>
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

void Slider::sliderChange(SliderChange change){
	QSlider::sliderChange(change);
}

int Slider::valueFromPosition(const QPoint& pos) const
{
	int percent;
	if(this->orientation() == Qt::Vertical) {
		percent = 100 - (pos.y() * 100) / geometry().height();
	}

	else {
		percent = (pos.x() * 100) / geometry().width();
	}

	int range = this->maximum() - this->minimum();
	return (range * percent) / 100 + this->minimum();
}

static QRect calculateRectangle(QSlider* slider, int value, bool is_horizontal)
{
	int longSide = slider->width();
	int shortSide = slider->height();
	int rectThickness = Gui::Util::textWidth(slider->fontMetrics(), "m") / 4;

	if(!is_horizontal){
		longSide = slider->height();
		shortSide = slider->width();
	}

	int h = rectThickness;
	int w = longSide - 4;
	int x = 2;
	int y = (shortSide - h) / 2;

	int rectHeight = h;
	int percent = ((value - slider->minimum()) * 10000) / (slider->maximum() - slider->minimum());
	int rectWidth = (w * percent) / 10000;
	int rectX = x;
	int rectY = y + (h - rectHeight) / 2;

	QRect ret(rectX, rectY, rectWidth, rectHeight);
	if(!is_horizontal)
	{
		ret = QRect(rectY, longSide - rectWidth, rectHeight, rectWidth);
	}

	return ret;
}

void Slider::paintEvent(QPaintEvent* e)
{
	if(!Style::isDark())
	{
		QSlider::paintEvent(e);
		return;
	}

	bool isHorizontal = (this->orientation() == Qt::Horizontal);

	using RectColorPair=QPair<QRect, QColor>;
	QList<RectColorPair> rects;

	QRect rectDark = calculateRectangle(this, this->maximum(), isHorizontal);
	rects << RectColorPair(rectDark, QColor(42, 42, 42));

	if(this->hasAdditionalValue())
	{
		int otherValue = this->additionalValue();

		spLog(Log::Develop, this) << "value: " << this->value() << " buffer: " << this->additionalValue();
		QRect rect = calculateRectangle(this, otherValue, isHorizontal);
		rects << RectColorPair(rect, this->additionalValueColor());
	}

	QRect rectOrange = calculateRectangle(this, this->value(), isHorizontal);
	rects << RectColorPair(rectOrange, QColor(243, 132, 26));
	//rects << RectColorPair(rect_orange, QColor(66, 78, 114));

	QPainter painter(this);

	if(m->hovered)
	{
		QColor colorLight(72,72,72);
		painter.setPen(colorLight);

		QPainterPath path;
		path.addRoundedRect(this->rect(), 3, 3);
		painter.fillPath(path, colorLight);
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

void Slider::focusInEvent(QFocusEvent* e)
{
	QSlider::focusInEvent(e);
	emit sigSliderGotFocus();
}

void Slider::focusOutEvent(QFocusEvent* e)
{
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
	int newValue = valueFromPosition(e->pos());
	setValue(newValue);

	this->setSliderDown(false);
}

bool Slider::event(QEvent* e)
{
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
