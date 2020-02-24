
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

#ifndef HEADERVIEW_H
#define HEADERVIEW_H

#include "Gui/Library/Header/ColumnHeader.h"
#include "Gui/Utils/Widgets/WidgetTemplate.h"
#include "Utils/Library/Sortorder.h"
#include "Utils/Pimpl.h"

#include <QTableView>
#include <QHeaderView>
#include <QMenu>

namespace Library
{
	/**
	 * @brief The HeaderView class
	 * @ingroup GuiLibrary
	 */
	class HeaderView :
			public Gui::WidgetTemplate<QHeaderView>
	{
		Q_OBJECT
		PIMPL(HeaderView)

		signals:
			void sigColumnsChanged();

		public:
			HeaderView(Qt::Orientation orientation, QWidget* parent=nullptr);
			virtual ~HeaderView() override;

			void init(const ColumnHeaderList& column_headers, const QByteArray& state, Library::SortOrder sorting);

			Library::SortOrder	switchSortorder(int column_index);
			ColumnHeaderPtr		column(int idx);

			QSize sizeHint() const override;

		private:
			QString resizeText() const;
			int calcHeaderWidth() const;

		private slots:
			void actionTriggered(bool b);
			void actionResizeTriggered();

		protected:
			void languageChanged() override;
	};
}

#endif // HEADERVIEW_H
