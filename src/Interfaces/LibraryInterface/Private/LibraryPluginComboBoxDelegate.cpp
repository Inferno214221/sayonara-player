/* LibraryPluginComboBoxDelegate.cpp */

/* Copyright (C) 2011-2019 Lucio Carreras
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



#include "LibraryPluginComboBoxDelegate.h"
#include <QPainter>
#include <QPalette>
#include <QColor>

LibraryPluginComboBoxDelegate::LibraryPluginComboBoxDelegate(QObject* parent) : Gui::ComboBoxDelegate(parent) {}

LibraryPluginComboBoxDelegate::~LibraryPluginComboBoxDelegate() {}

void LibraryPluginComboBoxDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	if(index.data(Qt::AccessibleDescriptionRole).toString() == QLatin1String("separator"))
	{
		QColor color = option.palette.color(QPalette::Disabled, QPalette::Foreground);
		color.setAlpha(196);
		painter->setPen(color);

		//painter->translate(4, 0);
		painter->setBrush(option.palette.color(QPalette::Active, QPalette::Background));
		painter->fillRect(option.rect, painter->brush());
		painter->drawLine(option.rect.left() + 4, option.rect.center().y(), option.rect.right() - 4, option.rect.center().y());
	}

	else {
		Gui::ComboBoxDelegate::paint(painter, option, index);
	}
}

QSize LibraryPluginComboBoxDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	QString type = index.data(Qt::AccessibleDescriptionRole).toString();

	if(type == QLatin1String("separator"))
	{
		return QSize(0, 1);
	}

	return Gui::ComboBoxDelegate::sizeHint( option, index );
}
