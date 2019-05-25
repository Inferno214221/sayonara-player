/* FileListModel.h */

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

#ifndef FILE_LIST_MODEL_H
#define FILE_LIST_MODEL_H

#include "Gui/Utils/SearchableWidget/SearchableModel.h"
#include "Utils/Pimpl.h"

#include <QModelIndex>

class QVariant;

class FileListModel :
	public SearchableListModel
{
	Q_OBJECT
	PIMPL(FileListModel)

	private:
		bool check_row_for_searchstring(int row, const QString& substr) const;

	public:
		explicit FileListModel(QObject* parent=nullptr);
		virtual ~FileListModel();

		void set_parent_directory(LibraryId, const QString& dir);

		LibraryId library_id() const;
		QString parent_directory() const;

		QStringList files() const;

		QModelIndexList search_results(const QString& substr) override;

		QVariant data(const QModelIndex &index, int role=Qt::DisplayRole) const override;

		int rowCount(const QModelIndex &parent = QModelIndex()) const override;

		QMimeData* mimeData(const QModelIndexList &indexes) const override;
		Qt::ItemFlags flags(const QModelIndex& index) const override;
};

#endif
