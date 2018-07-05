/* AlbumCoverDelegate.cpp */

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

#include "CoverDelegate.h"
#include <QPainter>

Library::CoverDelegate::CoverDelegate(QObject* parent) :
	QItemDelegate(parent)
{}

Library::CoverDelegate::~CoverDelegate() {}

void Library::CoverDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	const int offset = option.rect.height() / 20;

	painter->save();
	painter->translate(0, offset);

	QStyleOptionViewItem option_new(option);
	option_new.rect.setHeight(option.rect.height() - offset);

	QItemDelegate::paint(painter, option_new, index);
	painter->restore();
}
