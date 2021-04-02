/* RatingLabel.cpp */

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

#include "Utils/globals.h"

#include "RatingLabel.h"
#include "Gui/Utils/GuiUtils.h"

#include <QMouseEvent>
#include <QPainter>
#include <QIcon>

#include <algorithm>

using Gui::RatingLabel;

namespace
{
	struct IconCache
	{
		const QIcon active = Gui::Util::icon(QStringLiteral("star.png"), Gui::Util::NoTheme);
		const QIcon inactive = Gui::Util::icon(QStringLiteral("star_disabled.png"), Gui::Util::NoTheme);
	};

	QIcon iconProvider(bool active)
	{
		static IconCache iconCache;
		return (active)
			? iconCache.active
			: iconCache.inactive;
	}

	int iconSize(const QLabel* label, int verticalOffset)
	{
		return std::min(label->height() - verticalOffset - 4, label->width() / 6);
	}
}

struct RatingLabel::Private
{
	int offsetX;
	int offsetY;

	Rating rating;
	bool enabled;

	Private(bool enabled) :
		offsetX(3),
		offsetY(0),
		rating(Rating::Zero),
		enabled(enabled) {}

	Private() :
		Private(true) {}
};

RatingLabel::RatingLabel(QWidget* parent, bool enabled) :
	QLabel(parent)
{
	m = Pimpl::make<Private>(enabled);
}

RatingLabel::~RatingLabel() = default;

Rating RatingLabel::ratingAt(QPoint pos) const
{
	const auto drating = ((pos.x() * 1.0) / (iconSize(this, m->offsetY) + 2.0)) + 0.5;
	const auto iRating = static_cast<int>(drating);

	auto rating = static_cast<Rating>(iRating);
	rating = std::min(rating, Rating::Five);
	rating = std::max(rating, Rating::Zero);

	return rating;
}

QSize RatingLabel::sizeHint() const
{
	const auto height = iconSize(this, m->offsetY);
	const auto width = iconSize(this, m->offsetY) * 5;

	return QSize(width, height);
}

QSize RatingLabel::minimumSizeHint() const
{
	return sizeHint();
}

void RatingLabel::setRating(Rating rating)
{
	m->rating = rating;
}

Rating RatingLabel::rating() const
{
	return m->rating;
}

void RatingLabel::setVerticalOffset(int offset)
{
	m->offsetY = offset;
}

void RatingLabel::paint(QPainter* painter, const QRect& rect)
{
	this->setGeometry(rect);

	const auto sz = iconSize(this, m->offsetY);

	painter->save();
	const auto offsetY =
		(m->offsetY == 0)
		? (this->height() - sz) / 2
		: m->offsetY;

	painter->translate(rect.x() + m->offsetX, rect.y() + offsetY);

	for(uchar i = 0; i < uchar(Rating::Five); i++)
	{
		const auto rating = static_cast<Rating>(i);
		const auto icon = (rating < m->rating)
		                  ? iconProvider(true)
		                  : iconProvider(false);

		painter->drawPixmap(0, 0, sz, sz, icon.pixmap(sz, sz));

		painter->translate(sz + 2, 0);
	}

	painter->restore();
}

struct Gui::RatingEditor::Private
{
	RatingLabel* label = nullptr;
	bool mouseTrackable;

	// this rating is the rating we want to set
	// the rating shown in RatingLabel is the visible
	// rating. actual_rating is updated on mouse click
	// This is the _ONLY_ way to update it
	Rating actualRating;

	Private(Rating rating) :
		label {new RatingLabel(nullptr, true)},
		mouseTrackable(true),
		actualRating(rating)
	{
		label->setRating(actualRating);
	}
};

Gui::RatingEditor::RatingEditor(QWidget* parent) :
	Gui::RatingEditor(Rating::Zero, parent) {}

Gui::RatingEditor::RatingEditor(Rating rating, QWidget* parent) :
	QWidget(parent)
{
	m = Pimpl::make<Private>(rating);

	this->setEnabled(rating != Rating::Last);
	this->setStyleSheet("background: transparent;");
	this->setFocusPolicy(Qt::StrongFocus);
}

Gui::RatingEditor::~RatingEditor() = default;

void Gui::RatingEditor::setRating(Rating rating)
{
	m->actualRating = rating;
	m->label->setRating(rating);

	this->setEnabled(rating != Rating::Last);
	this->repaint();
}

Rating Gui::RatingEditor::rating() const
{
	return m->actualRating;
}

void Gui::RatingEditor::setVerticalOffset(int offset)
{
	m->label->setVerticalOffset(offset);
}

void Gui::RatingEditor::setMouseTrackable(bool b)
{
	m->mouseTrackable = b;
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
	m->label->paint(&painter, e->rect());
}

void Gui::RatingEditor::focusInEvent(QFocusEvent* e)
{
	this->setMouseTracking(m->mouseTrackable);
	QWidget::focusInEvent(e);
}

void Gui::RatingEditor::focusOutEvent(QFocusEvent* e)
{
	this->setMouseTracking(false);
	m->label->setRating(m->actualRating);

	emit sigFinished(false);

	QWidget::focusOutEvent(e);
}

void Gui::RatingEditor::mousePressEvent(QMouseEvent* e)
{
	const auto rating = m->label->ratingAt(e->pos());
	m->label->setRating(rating);

	repaint();

	QWidget::mousePressEvent(e);
}

void Gui::RatingEditor::mouseMoveEvent(QMouseEvent* e)
{
	const auto rating = m->label->ratingAt(e->pos());
	m->label->setRating(rating);

	repaint();

	QWidget::mouseMoveEvent(e);
}

void Gui::RatingEditor::mouseReleaseEvent(QMouseEvent* e)
{
	/* Important: Do not call QWidget::mouseReleaseEvent here.
	 * this causes the edit trigger QAbstractItemView::SelectedClicked
	 * to fire again and open a new Editor */
	e->accept();

	Rating rating = m->label->ratingAt(e->pos());

	m->actualRating = rating;
	m->label->setRating(rating);

	repaint();

	emit sigFinished(true);
}
