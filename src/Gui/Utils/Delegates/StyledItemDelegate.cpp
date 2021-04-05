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
#include <QBrush>
#include <QVariant>

namespace
{
	QPixmap extractPixmapFromIndex(const QModelIndex& index, const QSize& size, double scaleFactor)
	{
		const auto variant = index.data(Qt::DecorationRole);
		if(variant.canConvert<QPixmap>())
		{
			return variant.value<QPixmap>().scaled(size * scaleFactor, Qt::KeepAspectRatio, Qt::SmoothTransformation);
		}

		else if(variant.canConvert<QIcon>())
		{
			const auto icon = variant.value<QIcon>();
			return icon.pixmap(size * scaleFactor);
		}

		return QPixmap();
	}

	QRect getPixmapBoundingRectangle(const QPixmap& pixmap, const QRect& rect)
	{
		auto result = rect;

		result.translate((rect.width() - pixmap.width()) / 2, (rect.height() - pixmap.height()) / 2);
		result.setWidth(pixmap.width());
		result.setHeight(pixmap.height());

		return result;
	}
}

struct Gui::StyledItemDelegate::Private
{
	int decorationColumn;

	Private() :
		decorationColumn(-1) {}
};

Gui::StyledItemDelegate::StyledItemDelegate(QObject* parent) :
	QStyledItemDelegate(parent)
{
	m = Pimpl::make<Private>();
}

Gui::StyledItemDelegate::StyledItemDelegate(int decorationColumn, QObject* parent) :
	Gui::StyledItemDelegate(parent)
{
	m->decorationColumn = decorationColumn;
}

Gui::StyledItemDelegate::~StyledItemDelegate() = default;

QSize Gui::StyledItemDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	const auto width = index.data(Qt::SizeHintRole).toSize().width();
	const auto height = (Gui::Util::viewRowHeight() < 0)
	                    ? option.fontMetrics.height() + 4
	                    : Gui::Util::viewRowHeight();

	return QSize(width, height);
}

void
Gui::StyledItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	if(index.column() != m->decorationColumn)
	{
		QStyledItemDelegate::paint(painter, option, index);
		return;
	}

	painter->save();

	const auto isSelected = (option.state & QStyle::State_Selected);
	if(isSelected)
	{
		painter->fillRect(option.rect, option.palette.highlight());
	}

	const auto pixmap = extractPixmapFromIndex(index, option.rect.size(), 0.8);
	if(!pixmap.isNull())
	{
		const auto rect = getPixmapBoundingRectangle(pixmap, option.rect);
		painter->drawPixmap(rect, pixmap);
	}

	painter->restore();
}