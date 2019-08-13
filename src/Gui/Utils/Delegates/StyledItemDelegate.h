/* StyledItemDelegate.h */

/* Copyright (C) 2011-2019  Lucio Carreras
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

#ifndef STYLEDITEMDELEGATE_H
#define STYLEDITEMDELEGATE_H

#include <QStyledItemDelegate>

namespace Gui
{
	/**
	 * @brief The StyledItemDelegate class assures a certain
	 * height of rows in a table and tree view
	 * @ingroup Gui
	 */
	class StyledItemDelegate : public QStyledItemDelegate
	{
	public:
		using QStyledItemDelegate::QStyledItemDelegate;

		QSize sizeHint(const QStyleOptionViewItem &option,
					   const QModelIndex &index) const override;
	};
}

#endif // STYLEDITEMDELEGATE_H
