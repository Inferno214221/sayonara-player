/* ComboBoxDelegate.cpp */

/* Copyright (C) 2011-2024 Michael Lugmair (Lucio Carreras)
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

#include "ComboBoxDelegate.h"
#include "Gui/Utils/Style.h"

#include <QModelIndex>
#include <QSize>

using Gui::ComboBoxDelegate;

ComboBoxDelegate::ComboBoxDelegate(QObject* parent) :
	QStyledItemDelegate(parent) {}

ComboBoxDelegate::~ComboBoxDelegate() = default;

QSize ComboBoxDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	const auto defaultSize = QStyledItemDelegate::sizeHint(option, index);
	const auto height = option.fontMetrics.height();

	return (Style::isDark())
	       ? QSize(defaultSize.width(), std::min(height * 2, height + 12))
	       : defaultSize;
}
