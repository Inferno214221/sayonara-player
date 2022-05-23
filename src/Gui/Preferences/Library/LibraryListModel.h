/* LibraryListModel.h */

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

#ifndef SAYONARA_PLAYER_LIBRARYLISTMODEL_H
#define SAYONARA_PLAYER_LIBRARYLISTMODEL_H

#include <QAbstractListModel>
#include "Utils/Pimpl.h"

using LibName = QString;
using LibPath = QString;

namespace Library
{
	class Manager;
}

class LibraryListModel :
	public QAbstractListModel
{
	Q_OBJECT
	PIMPL(LibraryListModel)

	public:
		explicit LibraryListModel(Library::Manager* libraryManager, QObject* parent = nullptr);
		~LibraryListModel() override;

		// NOLINTNEXTLINE(google-default-arguments)
		[[nodiscard]] int rowCount(const QModelIndex& index = QModelIndex()) const override;
		[[nodiscard]] QVariant data(const QModelIndex& index, int role) const override;

		void appendRow(const LibName& name, const LibPath& path);
		void renameRow(int row, const LibName& newName);
		void changePath(int row, const LibPath& path);
		void moveRow(int from, int to);
		void removeRow(int row);

		[[nodiscard]] QString name(int idx) const;
		[[nodiscard]] QString path(int idx) const;

		void reset();
		bool commit();
};

#endif // SAYONARA_PLAYER_LIBRARYLISTMODEL_H
