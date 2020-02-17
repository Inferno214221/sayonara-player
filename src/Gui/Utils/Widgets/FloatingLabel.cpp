/* FloatingLabel.cpp */

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

#include "FloatingLabel.h"
#include "Gui/Utils/GuiUtils.h"

#include <QPaintEvent>
#include <QPainter>
#include <QTimer>

using Gui::FloatingLabel;

const static int timerInterval=20;

struct FloatingLabel::Private
{
	double		offset;
	double		offsetDirection;
	int			charsPerSecond;
	QTimer*		timer=nullptr;
	QString		text;

	bool		floating;

	Private() :
		offset(0),
		offsetDirection(-1.0),
		charsPerSecond(3),
		floating(false)
	{}
};

FloatingLabel::FloatingLabel(QWidget* parent) :
	Gui::WidgetTemplate<QLabel>(parent)
{
	m = Pimpl::make<Private>();

	m->timer = new QTimer(this);
	connect(m->timer, &QTimer::timeout, this, &FloatingLabel::updateOffset);
}

FloatingLabel::~FloatingLabel() = default;

void FloatingLabel::paintEvent(QPaintEvent* event)
{
	QPainter painter(this);
	event->ignore();

	if(!m->floating)
	{
		painter.drawText
		(
			QRectF(0, 0, this->width(), this->height()),
			int(this->alignment()),
			m->text
		);

		return;
	}

	QFontMetrics fm = this->fontMetrics();

	painter.drawText
	(
		QRectF(int(m->offset), 0, Gui::Util::textWidget(fm, m->text), fm.height()),
		m->text
	);
}

void FloatingLabel::resizeEvent(QResizeEvent* event)
{
	QLabel::resizeEvent(event);

	QFontMetrics fm = this->fontMetrics();
	int difference = Gui::Util::textWidget(fm, m->text) - this->width();

	if(difference <= 0)
	{
		m->timer->stop();

		m->offset = 0;
		m->offsetDirection = -1.0;
		m->floating = false;

		this->update();
	}

	else if(!m->timer->isActive()){
		m->timer->start(timerInterval);
	}
}

void FloatingLabel::setFloatingText(const QString& text)
{
	if(text == m->text){
		return;
	}

	m->text = text;
	m->offset = 0;
	m->offsetDirection = -1.0;
	m->floating = false;

	updateOffset();
	repaint();
}

void FloatingLabel::setCharsPerSecond(int chars_per_second)
{
	m->charsPerSecond = chars_per_second;
}

void FloatingLabel::updateOffset()
{
	static const int tolerance = 10;

	QFontMetrics fm = this->fontMetrics();
	int difference = Gui::Util::textWidget(fm, m->text) - this->width();

	if(difference <= 0)
	{
		m->floating = false;
		m->offset = 0;
		return;
	}

	m->floating = true;

	int minOffset = -(Gui::Util::textWidget(fm, m->text) - this->width() + tolerance);
	int maxOffset = tolerance;

	int charWidth = Gui::Util::textWidget(fm, "O");
	int charsWidth = m->charsPerSecond * charWidth;

	if(difference < charsWidth)
	{
		charsWidth = difference / 2;
	}

	double pixelsPerMSecond = (charsWidth) / 1000.0;
	double pixelsPerIntervalStep = pixelsPerMSecond * timerInterval;

	m->offset += ( m->offsetDirection * pixelsPerIntervalStep );
	if(m->offset < minOffset || m->offset > maxOffset)
	{
		m->offsetDirection = -m->offsetDirection;

		if(m->offset < minOffset)
		{
			m->offset = minOffset;
		}

		else {
			m->offset = maxOffset;
		}
	}

	this->update();
}
