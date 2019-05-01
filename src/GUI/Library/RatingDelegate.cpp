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
#include "GUI/Utils/RatingLabel.h"

#include <QPainter>
#include <QStyle>

using namespace Library;

struct RatingDelegate::Private
{
	int	rating_column;
	bool enabled;

	Private(bool enabled, int rating_column) :
		rating_column(rating_column),
		enabled(enabled)
	{}
};

RatingDelegate::RatingDelegate(QObject* parent, int rating_column, bool enabled) :
	StyledItemDelegate(parent)
{
	m = Pimpl::make<Private>(enabled, rating_column);
}

RatingDelegate::~RatingDelegate() {}

void RatingDelegate::paint(QPainter *painter, const QStyleOptionViewItem & option, const QModelIndex & index) const
{
	if(!index.isValid()) return;

	QStyledItemDelegate::paint(painter, option, index);

	if(index.column() == m->rating_column)
	{
		RatingLabel label(nullptr, true);
		label.set_rating(index.data(Qt::EditRole).toInt());
		label.setGeometry(option.rect);

		painter->save();
		painter->translate(option.rect.left(), option.rect.top() );

		label.render(painter);

		painter->restore();
	}
}

QWidget* RatingDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem & option, const QModelIndex & index) const
{
	Q_UNUSED(option)

	RatingLabel *label = new RatingLabel(parent, m->enabled);

	connect(label, &RatingLabel::sig_finished, this, &RatingDelegate::destroy_editor);

	label->set_rating(index.data(Qt::EditRole).toInt());

	return label;
}


void RatingDelegate::destroy_editor(bool save)
{
	Q_UNUSED(save)

	RatingLabel *label = qobject_cast<RatingLabel *>(sender());
	if(!label) return;

	disconnect(label, &RatingLabel::sig_finished, this, &RatingDelegate::destroy_editor);

	emit commitData(label);
	emit closeEditor(label);
}


void RatingDelegate::setEditorData(QWidget *editor, const QModelIndex & index) const
{
	Rating rating = index.data(Qt::EditRole).toInt();

	RatingLabel* label = qobject_cast<RatingLabel *>(editor);
	if(!label) return;

	label->set_rating(rating);
}


void RatingDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex & index) const
{
	RatingLabel* label = qobject_cast<RatingLabel *>(editor);
	if(!label) return;
	model->setData(index, label->get_rating());
}
