/* LibraryRatingDelegate.cpp */

/* Copyright (C) 2011-2017  Lucio Carreras
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

#include "LibraryRatingDelegate.h"
#include "GUI/Helper/RatingLabel/RatingLabel.h"
//#include "GUI/Helper/Style/Style.h"

#include <QFontMetrics>
#include <QPainter>

LibraryRatingDelegate::LibraryRatingDelegate(QObject* parent, int rating_column, bool enabled) :
	QItemDelegate(parent)
{
	_enabled = enabled;
	_rating_column = rating_column;
}

LibraryRatingDelegate::~LibraryRatingDelegate() {}

void LibraryRatingDelegate::paint(QPainter *painter, const QStyleOptionViewItem & option, const QModelIndex & index) const
{
	if(!index.isValid()) return;

	if(index.column() == _rating_column)
	{
		RatingLabel label(nullptr, true);
		label.set_rating(index.data().toInt());
		label.setGeometry(option.rect);

		painter->save();
		painter->translate(option.rect.left(), option.rect.top() );
		label.render(painter);
		painter->restore();
		return;
	}

	QItemDelegate::paint(painter, option, index);
}

QWidget* LibraryRatingDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem & option, const QModelIndex & index) const
{
	Q_UNUSED(option)

	RatingLabel *label = new RatingLabel(parent, _enabled);

	connect(label, &RatingLabel::sig_finished, this, &LibraryRatingDelegate::destroy_editor);

    label->set_rating(index.data().toInt());

    return label;
}


void LibraryRatingDelegate::destroy_editor(bool save) {
	Q_UNUSED(save)

    RatingLabel *label = qobject_cast<RatingLabel *>(sender());
    if(!label) return;

	disconnect(label, &RatingLabel::sig_finished, this, &LibraryRatingDelegate::destroy_editor);

	emit commitData(label);
	emit sig_rating_changed(label->get_rating());
    emit closeEditor(label);
}


void LibraryRatingDelegate::setEditorData(QWidget *editor, const QModelIndex & index) const
{
	int rating = index.data().toInt();

	RatingLabel* label = qobject_cast<RatingLabel *>(editor);
	if(!label) return;

	label->set_rating(rating);
}


void LibraryRatingDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex & index) const
{
    RatingLabel* label = qobject_cast<RatingLabel *>(editor);
    if(!label) return;
    model->setData(index, label->get_rating());
}

