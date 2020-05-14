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
	QHash<int, QSize> minimumActualSizeMap;

	Private() :
		decorationColumn(-1)
	{}

	QSize calcPixmapSize(QSize size)
	{
		int minimum = std::min(size.width(), size.height());
		if(this->minimumActualSizeMap.contains(minimum)){
			return this->minimumActualSizeMap[minimum];
		}

		int scaledMinimum = ((minimum * 30) / 40);

		const QList<int> rounds{16, 22, 24, 32, 36, 48};
		auto it = std::min_element(rounds.begin(), rounds.end(), [scaledMinimum](int r1, int r2){
			return (std::abs(scaledMinimum - r1) < std::abs(scaledMinimum - r2));
		});

		QSize ret(*it, *it);
		this->minimumActualSizeMap[minimum] = ret;

		return ret;
	}
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

	const QPixmap pixmap = index.data(Qt::DecorationRole).value<QPixmap>();

	QRect r2 = option.rect;
	QSize actualSize = m->calcPixmapSize(option.rect.size());

	r2.setSize(actualSize);
	r2.translate((option.rect.bottomRight() - r2.bottomRight()) / 2);

	painter->drawPixmap(r2, pixmap);
	painter->restore();
}

void Gui::StyledItemDelegate::setDecorationColumn(int index)
{
	m->decorationColumn = index;
}
