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
}

RatingLabel::~RatingLabel() = default;

Rating RatingLabel::rating_at(QPoint pos) const
{
	double drating = ((pos.x() * 1.0) / (m->icon_size + 2.0)) + 0.5;
	Rating rating = scast(Rating, drating);

	rating=std::min(rating, Rating::Five);
	rating=std::max(rating, Rating::Zero);

	return rating;
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

void RatingLabel::set_rating(Rating rating)
{
	m->rating = rating;
}

Rating RatingLabel::rating() const
{
	return m->rating;
}

void RatingLabel::set_vertical_offset(int offset)
{
	m->offset_y = offset;
}

void RatingLabel::paint(QPainter* painter, const QRect& rect)
{
	this->setGeometry(rect);

	painter->save();
	int offset_y = m->offset_y;
	if(m->offset_y == 0) {
		offset_y = (this->height() - m->icon_size) / 2;
	}

	painter->translate(rect.x() + m->offset_x, rect.y() + offset_y );

	for(uchar i=0; i<uchar(Rating::Five); i++)
	{
		Rating rating = Rating(i);
		if(rating < m->rating)
		{
			painter->drawPixmap(0, 0, m->icon_size, m->icon_size, m->pm_active);
		}

		else {
			painter->drawPixmap(0, 0, m->icon_size, m->icon_size, m->pm_inactive);
		}

		painter->translate(m->icon_size + 2, 0);
	}

	painter->restore();
}

struct Gui::RatingEditor::Private
{
	RatingLabel*	label=nullptr;
	bool			mouse_trackable;

	Private(Rating rating) :
		mouse_trackable(true)
	{
		label = new RatingLabel(nullptr, true);
		label->set_rating(rating);
	}
};

Gui::RatingEditor::RatingEditor(QWidget* parent) :
	Gui::RatingEditor(Rating::Zero, parent)
{}

Gui::RatingEditor::RatingEditor(Rating rating, QWidget* parent) :
	QWidget(parent)
{
	m = Pimpl::make<Private>(rating);

	this->setEnabled(rating != Rating::Last);
	this->setStyleSheet("background: transparent;");
	this->setFocusPolicy(Qt::StrongFocus);
}

Gui::RatingEditor::~RatingEditor() = default;

void Gui::RatingEditor::set_rating(Rating rating)
{
	m->label->set_rating(rating);
	this->setEnabled(rating != Rating::Last);
}

Rating Gui::RatingEditor::rating() const
{
	return m->label->rating();
}

void Gui::RatingEditor::set_vertical_offset(int offset)
{
	m->label->set_vertical_offset(offset);
}

void Gui::RatingEditor::set_mousetrackable(bool b)
{
	m->mouse_trackable = b;
	if(!b)
	{
		this->setMouseTracking(b);
	}
}

QSize Gui::RatingEditor::sizeHint() const
{
	return m->label->sizeHint();
}

QSize Gui::RatingEditor::minimumSizeHint() const
{
	return m->label->sizeHint();
}

void Gui::RatingEditor::paintEvent(QPaintEvent* e)
{
	e->accept();

	QPainter painter(this);
	m->label->paint(&painter, rect());
}

void Gui::RatingEditor::focusInEvent(QFocusEvent* e)
{
	this->setMouseTracking(m->mouse_trackable);

	QWidget::focusInEvent(e);
}

void Gui::RatingEditor::focusOutEvent(QFocusEvent* e)
{
	this->setMouseTracking(false);

	emit sig_finished(false);

	QWidget::focusOutEvent(e);
}

void Gui::RatingEditor::mousePressEvent(QMouseEvent *e)
{
	Rating rating = m->label->rating_at(e->pos());
	m->label->set_rating(rating);

	repaint();

	QWidget::mousePressEvent(e);
}

void Gui::RatingEditor::mouseMoveEvent(QMouseEvent* e)
{
	Rating rating = m->label->rating_at(e->pos());
	m->label->set_rating(rating);

	repaint();

	QWidget::mouseMoveEvent(e);
}

void Gui::RatingEditor::mouseReleaseEvent(QMouseEvent* e)
{
	/* Important: Do not call QWidget::mouseReleaseEvent here.
	 * this causes the edit trigger QAbstractItemView::SelectedClicked
	 * to fire again and open a new Editor */
	e->accept();

	Rating rating = m->label->rating_at(e->pos());
	m->label->set_rating(rating);

	repaint();

	emit sig_finished(true);
}
