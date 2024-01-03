/* SomaFMStationDelegate.cpp
 *
 * Copyright (C) 2011-2024 Michael Lugmair
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

#include "SomaFMStationDelegate.h"

#include "Utils/Logger/Logger.h"
#include <QPainter>
#include <QPixmap>
#include <QIcon>

SomaFMStationDelegate::SomaFMStationDelegate(QObject* parent) :
	Gui::StyledItemDelegate(parent)
{}

SomaFMStationDelegate::~SomaFMStationDelegate() = default;
/*
void SomaFMStationDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	if(index.column() != 0)
	{
		Gui::StyledItemDelegate::paint(painter, option, index);
		return;
	}
	painter->save();

	const auto icon = index.data(Qt::DecorationRole).value<QIcon>();
	const auto pixmap = icon.pixmap(option.rect.size());

	if(pixmap.isNull())
	{
		spLog(Log::Info, this) << "Pixmap is NULL!";
	}

	painter->drawPixmap(option.rect, icon.pixmap(option.rect.size()));
	painter->restore();
}
*/