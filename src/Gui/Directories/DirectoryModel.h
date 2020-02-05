
/* Copyright (C) 2011-2020  Lucio Carreras
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

namespace Library
{
	class Info;
}

/**
 * @brief The SearchableFileTreeModel class
 * @ingroup GuiDirectories
 */
class DirectoryModel :
	public SearchableModel<QFileSystemModel>
{
	Q_OBJECT
	PIMPL(DirectoryModel)

public:
	explicit DirectoryModel(QObject* parent=nullptr);
	~DirectoryModel() override;

	void search_only_dirs(bool b);
	void set_library(const Library::Info& info);
	Library::Info library_info() const;

public:
	QModelIndexList search_results(const QString& substr) override;
	Qt::ItemFlags	flags(const QModelIndex &index) const override;

private:
	using QFileSystemModel::setRootPath;
	void create_file_list(const QString& substr);

};

#endif // SEARCHABLEFileTreeView_H
