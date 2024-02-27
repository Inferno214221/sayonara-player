/* TableView.h */

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

#ifndef LIBRARYTABLEVIEW_H
#define LIBRARYTABLEVIEW_H

#include "Gui/Library/ItemView.h"
#include "Gui/Library/Header/ColumnHeader.h"
#include "Gui/Utils/SearchableWidget/SelectionView.h"

#include "Utils/Pimpl.h"
#include "Utils/Library/Sortorder.h"

namespace Library
{
	class TableView :
		public ItemView
	{
		Q_OBJECT
		PIMPL(TableView)

		private:

		public:
			explicit TableView(QWidget* parent = nullptr);
			~TableView() override;
			TableView(const TableView& other) = delete;
			TableView& operator=(const TableView& other) = delete;

			void init(AbstractLibrary* library);

		protected:
			void setupColumnNames();
			virtual void initView(AbstractLibrary* library) = 0;

			[[nodiscard]] virtual ColumnHeaderList columnHeaders() const = 0;
			[[nodiscard]] virtual QByteArray columnHeaderState() const = 0;
			virtual void saveColumnHeaderState(const QByteArray& state) = 0;

			[[nodiscard]] virtual VariableSortorder sortorder() const = 0;
			virtual void applySortorder(VariableSortorder s) = 0;

			[[nodiscard]] virtual bool autoResizeState() const = 0;
			virtual void saveAutoResizeState(bool b) = 0;

			void languageChanged() override;

			[[nodiscard]] int mapModelIndexToIndex(const QModelIndex& idx) const override;
			[[nodiscard]] ModelIndexRange mapIndexToModelIndexes(int idx) const override;

		protected slots:
			void headerColumnsChanged(int oldCount, int newCount);
			void sortorderChanged(int index, Qt::SortOrder sortorder);
			void sectionResized(int logicalIndex, int oldSize, int newSize);
			void sectionMoved(int logicalIndex, int oldVisualIndex, int newVisualIndex);
			void autoResizeTriggered(bool b);
	};
}

#endif // LIBRARYTABLEVIEW_H
