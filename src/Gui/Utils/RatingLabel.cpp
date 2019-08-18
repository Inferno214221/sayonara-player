/* RatingLabel.cpp */

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

#include "Utils/globals.h"

#include "RatingLabel.h"
#include "Gui/Utils/GuiUtils.h"

#include <QMouseEvent>
#include <QPainter>
#include <QPixmap>

#include <algorithm>

using Gui::RatingLabel;

struct RatingLabel::Private
{
	QWidget*	parent=nullptr;

	QPixmap 	pm_active;
	QPixmap 	pm_inactive;

	int			offset_x;
	int			offset_y;

	Rating		rating;
	uint8_t     icon_size;
	bool		enabled;

	Private() :
		parent(nullptr),
		offset_x(3),
		offset_y(0),
		rating(Rating::Zero),
		icon_size(14),
		enabled(true)
	{
		pm_active = Gui::Util::pixmap("star.png", Gui::Util::NoTheme, QSize(icon_size, icon_size), true);
		pm_inactive = Gui::Util::pixmap("star_disabled.png", Gui::Util::NoTheme, QSize(icon_size, icon_size), true);
	}

	Private(QWidget* parent, bool enabled) :
		Private()
	{
		this->parent = parent;
		this->enabled = enabled;
	}
};

RatingLabel::RatingLabel(QWidget* parent, bool enabled) :
	QLabel(parent)
{
	m = Pimpl::make<Private>(parent, enabled);

	QSizePolicy p(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

	this->setSizePolicy(p);
	this->setMouseTracking(true);
	this->setStyleSheet("background: transparent;");
}

RatingLabel::~RatingLabel() = default;

Rating RatingLabel::calc_rating(QPoint pos) const
{
	double drating = ((pos.x() * 1.0) / (m->icon_size + 2.0)) + 0.5;
	Rating rating = scast(Rating, drating);

	rating=std::min(rating, Rating(5));
	rating=std::max(rating, Rating(0));

	return rating;
}

void RatingLabel::paintEvent(QPaintEvent* e)
{
	QLabel::paintEvent(e);

	QPainter painter(this);

	int offset_y = m->offset_y;
	if(m->offset_y == 0) {
		offset_y = (this->height() - m->icon_size) / 2;
	}

	painter.translate(rect().x() + m->offset_x, rect().y() + offset_y );

	for(uchar i=0; i<uchar(Rating::Five); i++)
	{
		Rating rating = Rating(i);
		if(rating < m->rating)
		{
			painter.drawPixmap(0, 0, m->icon_size, m->icon_size, m->pm_active);
		}

		else {
			painter.drawPixmap(0, 0, m->icon_size, m->icon_size, m->pm_inactive);
		}

		painter.translate(m->icon_size + 2, 0);
	}
}

QSize RatingLabel::sizeHint() const
{
	int	h = m->icon_size + 2;
	int w = (m->icon_size + 2) * 5;

	return QSize(w, h);
}

QSize RatingLabel::minimumSizeHint() const
{
	return sizeHint();
}


void RatingLabel::mouseMoveEvent(QMouseEvent *e)
{
	if(!m->enabled) {
		return;
	}

	if(!hasFocus()){
		return;
	}

	Rating rating = calc_rating(e->pos());
	this->update_rating(rating);
}



void RatingLabel::mousePressEvent(QMouseEvent *e)
{
	if(!m->enabled) {
		return;
	}

	Rating rating = calc_rating(e->pos());
	update_rating(rating);
}


void RatingLabel::mouseReleaseEvent(QMouseEvent *e)
{
	Q_UNUSED(e);

	if(!m->enabled) {
		return;
	}

	emit sig_finished(true);
}


void RatingLabel::focusInEvent(QFocusEvent* e)
{
	Q_UNUSED(e);
}

void RatingLabel::focusOutEvent(QFocusEvent* e)
{
	Q_UNUSED(e);

	if(!m->enabled) {
		return;
	}

	emit sig_finished(false);
}

void RatingLabel::update_rating(Rating rating)
{
	m->rating = rating;
	update();
}

void RatingLabel::set_rating(Rating rating)
{
	m->rating = rating;
	update();
}

Rating RatingLabel::get_rating() const
{
	return m->rating;
}

void RatingLabel::set_offset_y(int offset)
{
	m->offset_y = offset;
}


