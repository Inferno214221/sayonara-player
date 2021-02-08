
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

#ifndef SEARCHABLEFILETREEMODEL_H
#define SEARCHABLEFILETREEMODEL_H

#include "Gui/Utils/SearchableWidget/SearchableModel.h"
#include "Utils/Pimpl.h"

#include <QTreeView>
#include <QFileSystemModel>
#include <QSortFilterProxyModel>

namespace Library
{
	class Info;
	class Manager;
}

namespace Directory
{
	/**
	 * @brief The SearchableFileTreeModel class
	 * @ingroup GuiDirectories
	 */
	class Model :
		public QSortFilterProxyModel
	{
		Q_OBJECT
		PIMPL(Model)

		signals:
			void sigBusy(bool b);

		public:
			explicit Model(Library::Manager* libraryManager, QObject* parent=nullptr);
			~Model() override;

			QModelIndex setDataSource(LibraryId libraryId);
			QModelIndex setDataSource(const QString& path);
			LibraryId libraryDataSource() const;

			QString filePath(const QModelIndex& index) const;
			QModelIndex indexOfPath(const QString& path) const;

			void setFilter(const QString& filter);

			int columnCount(const QModelIndex& parent) const override;
			QMimeData* mimeData(const QModelIndexList& indexes) const override;

		private slots:
			void filterTimerTimeout();

		protected:
			using QSortFilterProxyModel::setFilterRegExp;
			using QSortFilterProxyModel::setFilterWildcard;
			using QSortFilterProxyModel::setFilterFixedString;

			bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;
	};
}

#endif // SEARCHABLEFileTreeView_H
