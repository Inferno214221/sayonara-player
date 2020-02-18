
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
}

/**
 * @brief The SearchableFileTreeModel class
 * @ingroup GuiDirectories
 */
class DirectoryModel :
	public QSortFilterProxyModel
{
	Q_OBJECT
	PIMPL(DirectoryModel)

public:
	explicit DirectoryModel(QObject* parent=nullptr);
	~DirectoryModel() override;

	QModelIndex setDataSource(LibraryId libraryId);
	QModelIndex setDataSource(const QString& path);

	LibraryId libraryDataSource() const;

	QString filePath(const QModelIndex& index);
	QModelIndex indexOfPath(const QString& path) const;


	// QSortFilterProxyModel interface
	protected:
	bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;
};


#endif // SEARCHABLEFileTreeView_H
