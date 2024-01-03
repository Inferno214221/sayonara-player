/* FileListModel.h */

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

#ifndef FILE_LIST_MODEL_H
#define FILE_LIST_MODEL_H

#include "Gui/Utils/SearchableWidget/SearchableModel.h"
#include "Utils/Pimpl.h"

#include <QModelIndex>

class QPixmap;
class QVariant;

class LocalLibrary;

namespace Directory
{
	class IconWorkerThread :
		public QObject
	{
		Q_OBJECT
		PIMPL(IconWorkerThread)

		signals:
			void sigFinished(const QString& path);

		public:
			IconWorkerThread(const QSize& targetSize, const QString& filename);
			~IconWorkerThread();

			QPixmap pixmap() const;

		public slots:
			void start();
	};

	/**
	 * @brief The FileListModel class
	 * @ingroup GuiDirectories
	 */
	class FileListModel :
		public SearchableTableModel
	{
		Q_OBJECT
		PIMPL(FileListModel)

		public:
			explicit FileListModel(LocalLibrary* localLibrary, QObject* parent = nullptr);
			~FileListModel() override;

			QString parentDirectory() const;
			void setParentDirectory(const QString& dir);

			LibraryId libraryId() const;
			QStringList files() const;

			QModelIndexList searchResults(const QString& substr) override;

			QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
			QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

			int rowCount(const QModelIndex& parent = QModelIndex()) const override;
			int columnCount(const QModelIndex& parent = QModelIndex()) const override;

			QMimeData* mimeData(const QModelIndexList& indexes) const override;
			Qt::ItemFlags flags(const QModelIndex& index) const override;

		private:
			bool checkRowForSearchstring(int row, const QString& substr) const;

		private slots:
			void pixmapFetched(const QString& path);
	};
}

#endif
