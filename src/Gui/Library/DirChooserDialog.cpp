/* DirChooserDialog.cpp */

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



#include "DirChooserDialog.h"
#include <QDir>
#include <QList>
#include <QUrl>
#include <QStandardPaths>
#include <QStringList>
#include <QListView>
#include <QTreeView>
#include "Utils/Language.h"

DirChooserDialog::DirChooserDialog(QWidget* parent) :
	QFileDialog(parent)
{
	this->setDirectory(QDir::homePath());
	this->setWindowTitle(Lang::get(Lang::ImportDir));
	this->setFileMode(QFileDialog::DirectoryOnly);
	this->setOption(QFileDialog::DontUseNativeDialog, true);

	QList<QUrl> sidebar_urls = this->sidebarUrls();

	const QList<QStandardPaths::StandardLocation> locations {
		QStandardPaths::HomeLocation,
		QStandardPaths::DesktopLocation,
		QStandardPaths::DownloadLocation,
		QStandardPaths::MusicLocation,
		QStandardPaths::TempLocation
	};

	for(const QStandardPaths::StandardLocation& location : locations)
	{
		QStringList std_locations = QStandardPaths::standardLocations(location);
		for(const QString& std_location : std_locations)
		{
			QUrl url = QUrl::fromLocalFile(std_location);
			if(sidebar_urls.contains(url)){
				continue;
			}

			sidebar_urls << url;
		}
	}

	this->setSidebarUrls(sidebar_urls);

	QListView* list_view = this->findChild<QListView*>("listView");
	if(list_view != nullptr)
	{
		list_view->setSelectionMode(QAbstractItemView::MultiSelection);
		QTreeView* tree_view = this->findChild<QTreeView*>();
		if(tree_view){
			tree_view->setSelectionMode(QAbstractItemView::MultiSelection);
		}
	}
}

DirChooserDialog::~DirChooserDialog() {}
