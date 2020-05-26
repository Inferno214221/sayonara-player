/* DirChooserDialog.cpp */

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

#include "DirChooserDialog.h"
#include <QDir>
#include <QList>
#include <QUrl>
#include <QStandardPaths>
#include <QStringList>
#include <QListView>
#include <QTreeView>
#include "Utils/Language/Language.h"

using Library::DirChooserDialog;

DirChooserDialog::DirChooserDialog(QWidget* parent) :
	QFileDialog(parent)
{
	this->setDirectory(QDir::homePath());
	this->setWindowTitle(Lang::get(Lang::ImportDir));
	this->setOption(QFileDialog::ShowDirsOnly, true);
	this->setFilter(QDir::Filter::Dirs);
	this->setAcceptMode(QFileDialog::AcceptOpen);
	this->setFileMode(QFileDialog::Directory);

	const QList<QStandardPaths::StandardLocation> locations
		{
			QStandardPaths::HomeLocation,
			QStandardPaths::DesktopLocation,
			QStandardPaths::DownloadLocation,
			QStandardPaths::MusicLocation,
			QStandardPaths::TempLocation
		};

	QList<QUrl> sidebarUrls = this->sidebarUrls();
	for(const QStandardPaths::StandardLocation& location : locations)
	{
		const QStringList standardLocations = QStandardPaths::standardLocations(location);
		for(const QString& standardLocation : standardLocations)
		{
			const QUrl url = QUrl::fromLocalFile(standardLocation);
			if(sidebarUrls.contains(url))
			{
				continue;
			}

			sidebarUrls << url;
		}
	}

	this->setSidebarUrls(sidebarUrls);

	auto* listView = this->findChild<QListView*>("listView");
	if(listView)
	{
		listView->setSelectionMode(QAbstractItemView::MultiSelection);

		auto* treeView = this->findChild<QTreeView*>();
		if(treeView)
		{
			treeView->setSelectionMode(QAbstractItemView::MultiSelection);
		}
	}
}

DirChooserDialog::~DirChooserDialog() = default;
