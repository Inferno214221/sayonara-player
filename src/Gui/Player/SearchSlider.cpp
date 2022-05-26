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

#include <QEvent>
#include <QWheelEvent>

#include <algorithm>

using Gui::SearchSlider;

namespace
{
	int getDeltaFromModifier(const QFlags<Qt::KeyboardModifier>& modifiers)
	{
		constexpr const auto SmallStep = 5;
		constexpr const auto StandardStep = 10;
		constexpr const auto BigStep = 50;

		if(modifiers & Qt::ShiftModifier)
		{
			return StandardStep;
		}

		if(modifiers & Qt::AltModifier)
		{
			return BigStep;
		}

		return SmallStep;
	}
}

[[maybe_unused]] SearchSlider::SearchSlider(QWidget* parent) :
	Gui::Slider(parent)
{
	setMouseTracking(true);
}

SearchSlider::~SearchSlider() = default;

void SearchSlider::emitNewValue(int value)
{
	value = std::max(value, 0);
	value = std::min(value, maximum());

	emit sigSliderMoved(value);
}

bool SearchSlider::isBusy() const
{
	return this->isSliderDown();
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

bool SearchSlider::event(QEvent* e)
{
	if(isEnabled() && (e->type() == QEvent::Wheel))
	{
		auto* wheelEvent = dynamic_cast<QWheelEvent*>(e);

		const auto deltaVal = getDeltaFromModifier(wheelEvent->modifiers());
		const auto isScrollingDown = (wheelEvent->angleDelta().y() > 0);
		const auto newValue = isScrollingDown
		                      ? value() + deltaVal
		                      : value() - deltaVal;

		setValue(newValue);

		emitNewValue(value());
	}

	return Gui::Slider::event(e);
}
