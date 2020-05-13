/* StyledItemDelegate.cpp */

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

#include "StyledItemDelegate.h"
#include "Gui/Utils/GuiUtils.h"
#include <QSize>
#include <QPainter>

struct Gui::StyledItemDelegate::Private
{
	int decorationColumn;

	Private() :
		decorationColumn(-1)
	{}
};

Gui::StyledItemDelegate::StyledItemDelegate(QObject* parent) :
	QStyledItemDelegate(parent)
{
	m = Pimpl::make<Private>();
}

Gui::StyledItemDelegate::StyledItemDelegate(int columnIndex, QObject* parent) :
	Gui::StyledItemDelegate(parent)
{
	m->decorationColumn = columnIndex;
}

Gui::StyledItemDelegate::~StyledItemDelegate() = default;

QSize Gui::StyledItemDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	int width = index.data(Qt::SizeHintRole).toSize().width();
	int height = Gui::Util::viewRowHeight();

	if(height < 0) {
		height = option.fontMetrics.height() + 4;
	}

	return QSize(width, height);
}


void Gui::StyledItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	if(index.column() != m->decorationColumn)
	{
		QStyledItemDelegate::paint(painter, option, index);
		return;
	}

	painter->save();

	QPixmap pixmap = index.data(Qt::DecorationRole).value<QPixmap>();

	const QList<int> rounds{22, 24, 32, 36, 48};

	QRect r2 = option.rect;
	r2.setWidth((option.rect.width() * 30) / 40);
	r2.setHeight((option.rect.height() * 30) / 40);
	int minimum = std::min(r2.width(), r2.height());

	auto it = std::min_element(rounds.begin(), rounds.end(), [minimum](int r1, int r2){
		return (std::abs(minimum - r1) < std::abs(minimum - r2));
	});

	r2.setWidth(*it);
	r2.setHeight(*it);
	r2.translate((option.rect.bottomRight() - r2.bottomRight()) / 2);

	painter->drawPixmap(r2, pixmap);
	painter->restore();
}


void Gui::StyledItemDelegate::setDecorationColumn(int index)
{
	m->decorationColumn = index;
}
