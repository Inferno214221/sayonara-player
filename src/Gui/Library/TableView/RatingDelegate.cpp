/* LibraryRatingDelegate.cpp */

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

#include "RatingDelegate.h"
#include "Gui/Utils/Widgets/RatingLabel.h"

#include <QPainter>
#include <QStyle>
#include <array>

using Gui::RatingLabel;
using Gui::RatingEditor;

using namespace Library;

struct RatingDelegate::Private
{
	int ratingColumn;

	Private(int ratingColumn) :
		ratingColumn(ratingColumn) {}
};

RatingDelegate::RatingDelegate(int ratingColumn, int decorationColumn, QObject* parent) :
	StyledItemDelegate(decorationColumn, parent)
{
	m = Pimpl::make<Private>(ratingColumn);
}

RatingDelegate::~RatingDelegate() = default;

void RatingDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	if(!index.isValid())
	{
		return;
	}

	Gui::StyledItemDelegate::paint(painter, option, index);

	if(index.column() != m->ratingColumn)
	{
		return;
	}

	const auto rating = index.data(Qt::EditRole).value<Rating>();

	auto label = RatingLabel(nullptr, true);
	label.setRating(rating);
	label.paint(painter, option.rect);
}

QWidget*
RatingDelegate::createEditor(QWidget* parent, [[maybe_unused]] const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	const auto rating = index.data(Qt::EditRole).value<Rating>();
	auto* editor = new RatingEditor(rating, parent);

	connect(editor, &RatingEditor::sigFinished, this, &RatingDelegate::deleteEditor);

	editor->setFocus();
	return editor;
}

void RatingDelegate::deleteEditor([[maybe_unused]] bool save)
{
	auto* ratingEditor = qobject_cast<RatingEditor*>(sender());
	if(ratingEditor)
	{
		disconnect(ratingEditor, &RatingEditor::sigFinished, this, &RatingDelegate::deleteEditor);

		emit commitData(ratingEditor);
		emit closeEditor(ratingEditor);
	}
}

void RatingDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
	auto* ratingEditor = qobject_cast<RatingEditor*>(editor);
	if(!ratingEditor)
	{
		return;
	}

	const auto rating = index.data(Qt::EditRole).value<Rating>();
	ratingEditor->setRating(rating);
}

void RatingDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
	auto* ratingEditor = qobject_cast<RatingEditor*>(editor);
	if(ratingEditor)
	{
		const auto rating = ratingEditor->rating();
		model->setData(index, QVariant::fromValue(rating));
	}
}
