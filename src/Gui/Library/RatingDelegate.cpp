/* LibraryRatingDelegate.cpp */

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

#include "RatingDelegate.h"
#include "Gui/Utils/RatingLabel.h"

#include <QPainter>
#include <QStyle>
#include <array>

using Gui::RatingLabel;
using namespace Library;

using RatingLabelArray = std::array<Gui::RatingLabel*, 6>;

struct RatingDelegate::Private
{
	int	rating_column;
	bool enabled;

	RatingLabelArray rating_labels;

	Private(bool enabled, int rating_column) :
		rating_column(rating_column),
		enabled(enabled)
	{}
};

RatingDelegate::RatingDelegate(QObject* parent, int rating_column, bool enabled) :
	StyledItemDelegate(parent)
{
	m = Pimpl::make<Private>(enabled, rating_column);

	for(int i=int(Rating::Zero); i < int(Rating::Last); i++)
	{
		auto* label = new RatingLabel(nullptr, true);
		label->set_rating(Rating(i));
		m->rating_labels[i] = label;
	}
}

RatingDelegate::~RatingDelegate() = default;

void RatingDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	if(!index.isValid()) {
		return;
	}

	if(index.column() != m->rating_column)
	{
		Gui::StyledItemDelegate::paint(painter, option, index);
		return;
	}

	QRect rect = option.rect;
	int parent_width = option.widget->width();
	int remainder = parent_width - (option.rect.x() + option.rect.width());
	if(remainder < 0)
	{
		rect.setWidth(option.rect.width() + remainder);
	}

	Rating rating = index.data(Qt::EditRole).value<Rating>();
	RatingLabel* label = m->rating_labels[int(rating)];

	painter->setClipping(true);
	painter->setClipRect(rect);

	painter->save();
	painter->translate(rect.left(), rect.top() );

	label->setGeometry(rect);
	label->render(painter);

	painter->restore();
}

QWidget* RatingDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem & option, const QModelIndex & index) const
{
	Q_UNUSED(option)

	auto* label = new RatingLabel(parent, m->enabled);

	connect(label, &RatingLabel::sig_finished, this, &RatingDelegate::destroy_editor);

	Rating rating = index.data(Qt::EditRole).value<Rating>();
	label->set_rating(rating);

	return label;
}


void RatingDelegate::destroy_editor(bool save)
{
	Q_UNUSED(save)

	auto* label = qobject_cast<RatingLabel*>(sender());
	if(!label) {
		return;
	}

	disconnect(label, &RatingLabel::sig_finished, this, &RatingDelegate::destroy_editor);

	emit commitData(label);
	emit closeEditor(label);
}


void RatingDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
	auto* label = qobject_cast<RatingLabel *>(editor);
	if(!label) {
		return;
	}

	Rating rating = index.data(Qt::EditRole).value<Rating>();
	label->set_rating(rating);
}


void RatingDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
	auto* label = qobject_cast<RatingLabel*>(editor);
	if(label)
	{
		Rating rating = label->get_rating();
		model->setData(index, QVariant::fromValue(rating));
	}
}
