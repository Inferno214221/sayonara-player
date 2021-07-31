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

namespace
{
	QRect getBasicDimensions(QSlider* slider, bool isHorizontal, int letterSize)
	{
		constexpr const auto Padding = 2;
		const auto rectThickness = letterSize / 4;

		return isHorizontal
		       ? QRect(Padding, (slider->height() - rectThickness) / 2, slider->width() - Padding * 2, rectThickness)
		       : QRect((slider->width() - rectThickness) / 2, Padding, rectThickness, slider->height() - Padding * 2);
	}

	QRect getPercentDimensions(const QRect& basicDimensions, bool isHorizontal, double percent)
	{
		auto result = basicDimensions;
		if(isHorizontal)
		{
			const auto newWidth = static_cast<int>(basicDimensions.width() * percent);
			result.setWidth(newWidth);
		}

		else
		{
			const auto newHeight = static_cast<int>(percent * basicDimensions.height());
			const auto heightDifference = basicDimensions.height() - newHeight;
			result.setTop(basicDimensions.top() + heightDifference);
			result.setHeight(newHeight);
		}

		return result;
	}

	QRect calculateRectangle(QSlider* slider, int value, bool isHorizontal, int letterSize)
	{
		const auto minimum = slider->minimum();
		const auto maximum = slider->maximum();

		const auto basicDimensions = getBasicDimensions(slider, isHorizontal, letterSize);
		const auto percent = ((value - minimum) * 1.0) / (maximum - minimum);

		return getPercentDimensions(basicDimensions, isHorizontal, percent);
	}

	double percentFromPosition(int position, int maxPosition, bool isHorizontal)
	{
		return isHorizontal
		       ? (position * 1.0) / maxPosition
		       : 1.0 - ((position * 1.0) / maxPosition);
	}
}

struct Slider::Private
{
	bool hovered {false};
	int letterSize {16};
};

Slider::Slider(QWidget* parent) :
	QSlider(parent)
{
	m = Pimpl::make<Private>();

	setTracking(true);
	setMouseTracking(true);
	setSingleStep(1);
	setPageStep(1);
}

Slider::~Slider() = default;

int Slider::valueFromPosition(const QPoint& pos) const
{
	const auto percent = (orientation() == Qt::Horizontal)
	                     ? percentFromPosition(pos.x(), geometry().width(), true)
	                     : percentFromPosition(pos.y(), geometry().height(), false);

	const auto range = (maximum() - minimum());

	return static_cast<int>((range * percent) + minimum());
}

void Slider::paintEvent(QPaintEvent* e)
{
	if(!Style::isDark())
	{
		QSlider::paintEvent(e);
		return;
	}

	static const auto Dark = QColor(42, 42, 42);
	static const auto Orange = QColor(243, 132, 26);
	static const auto LightGrey = QColor(72, 72, 72);

	const auto isHorizontal = (orientation() == Qt::Horizontal);

	using RectColorPair = QPair<QRect, QColor>;
	QList<RectColorPair> rects;

	const auto rectDark = calculateRectangle(this, this->maximum(), isHorizontal, m->letterSize);
	rects << RectColorPair(rectDark, Dark);

	const auto rectOrange = calculateRectangle(this, this->value(), isHorizontal, m->letterSize);
	rects << RectColorPair(rectOrange, Orange);

	QPainter painter(this);

	if(m->hovered)
	{
		painter.setPen(LightGrey);

		QPainterPath path;
		path.addRoundedRect(this->rect(), 3, 3);
		painter.fillPath(path, LightGrey);
		painter.drawPath(path);
	}

	for(const auto&[rect, color] : rects)
	{
		painter.setPen(color);
		painter.drawRect(rect);
		painter.fillRect(rect, color);
	}
}

void Slider::mouseMoveEvent(QMouseEvent* e)
{
	const auto newValue = valueFromPosition(e->pos());
	if(this->isSliderDown())
	{
		setValue(newValue);
	}

	else
	{
		emit sigSliderHovered(newValue);
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

	const auto newValue = valueFromPosition(e->pos());
	setValue(newValue);
}

void Slider::mouseReleaseEvent(QMouseEvent* e)
{
	const auto newValue = valueFromPosition(e->pos());
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
			if(!hasFocus())
			{
				emit sigSliderLostFocus();
			}

			break;

		case QEvent::FontChange:
			m->letterSize = this->fontMetrics().width('m');
			break;

		default:
			break;
	}

	return QSlider::event(e);
}
