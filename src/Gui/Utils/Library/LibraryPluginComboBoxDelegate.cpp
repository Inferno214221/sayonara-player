/* LibraryPluginComboBoxDelegate.cpp */

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

#include "LibraryPluginComboBoxDelegate.h"
#include <QPainter>
#include <QPalette>
#include <QColor>
#include <QFrame>

using Library::PluginComboBoxDelegate;

PluginComboBoxDelegate::PluginComboBoxDelegate(QWidget* parent) :
	Gui::ComboBoxDelegate(parent),
	mParent(parent)
{}

PluginComboBoxDelegate::~PluginComboBoxDelegate() = default;


void PluginComboBoxDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	if(index.data(Qt::AccessibleDescriptionRole).toString() == QLatin1String("separator") && index.row() == 1)
	{
		QColor color = painter->brush().color().darker();
		color.setAlpha(80);
		QPen pen(color);
		painter->setPen(pen);

		painter->drawLine(option.rect.left() + 2,
						  option.rect.center().y(),
						  option.rect.right() - 2,
						  option.rect.center().y());
	}

	else
	{
		Gui::ComboBoxDelegate::paint(painter, option, index);
	}
}

QSize PluginComboBoxDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	QString type = index.data(Qt::AccessibleDescriptionRole).toString();

	if(type == QLatin1String("separator"))
	{
		return QSize(0, 5);
	}

	return Gui::ComboBoxDelegate::sizeHint( option, index );
}
