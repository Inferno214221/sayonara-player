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

#include "DirectoryChooser.h"
#include "Gui/Utils/GuiUtils.h"
#include "Utils/Macros.h"
#include "Utils/Settings/Settings.h"

#include <QDir>
#include <QList>
#include <QUrl>
#include <QStandardPaths>
#include <QListView>
#include <QTreeView>

namespace
{
	QFileDialog::Options directoryDialogOptions()
	{
#ifdef DISABLE_NATIVE_DIR_DIALOGS // snap
		const auto forceNativeDialog = GetSetting(Set::Player_ForceNativeDirDialog);
		if(!forceNativeDialog)
		{
			return static_cast<QFileDialog::Options>(QFileDialog::Option::ShowDirsOnly |
													 QFileDialog::Option::DontUseNativeDialog);
		}
#endif

		return static_cast<QFileDialog::Options>(QFileDialog::Option::ShowDirsOnly);
	}

	void setupSidebarUrls(QFileDialog* fileDialog)
	{
		constexpr const auto locations = std::array {
			QStandardPaths::HomeLocation,
			QStandardPaths::DesktopLocation,
			QStandardPaths::DownloadLocation,
			QStandardPaths::MusicLocation,
			QStandardPaths::TempLocation
		};

		auto sidebarUrls = fileDialog->sidebarUrls();
		for(const auto& location: locations)
		{
			const auto standardLocations = QStandardPaths::standardLocations(location);
			for(const auto& standardLocation: standardLocations)
			{
				const auto url = QUrl::fromLocalFile(standardLocation);
				if(!sidebarUrls.contains(url))
				{
					sidebarUrls << url;
				}
			}
		}

		fileDialog->setSidebarUrls(sidebarUrls);
	}

	void enableMultiSelectionInWidget(QFileDialog* dialog)
	{
		if(auto* listView = dialog->findChild<QListView*>("listView"); listView)
		{
			listView->setSelectionMode(QAbstractItemView::MultiSelection);
		}

		if(auto* treeView = dialog->findChild<QTreeView*>(); treeView)
		{
			treeView->setSelectionMode(QAbstractItemView::MultiSelection);
		}
	}
}

namespace Gui
{
	DirectoryChooser::DirectoryChooser(const QString& title, const QString& initialDirectory,
	                                   bool enableMultiSelection, QWidget* parent) :
		QFileDialog(parent)
	{
		setOptions(directoryDialogOptions()); // should be set as early as possible

		setWindowTitle(title.isEmpty() ? tr("Choose directory") : title);
		setDirectory(initialDirectory.isEmpty() ? QDir::homePath() : initialDirectory);
		setAcceptMode(QFileDialog::AcceptOpen);
		setFilter(QDir::Filter::Dirs); // only show dirs
		setFileMode(QFileDialog::FileMode::Directory); // choose dirs when clicking "open"

		if(enableMultiSelection)
		{
			setOption(QFileDialog::DontUseNativeDialog, true);
			enableMultiSelectionInWidget(this);
		}

		setupSidebarUrls(this);
	}

	DirectoryChooser::~DirectoryChooser() = default;

	QString DirectoryChooser::getDirectory(const QString& title,
	                                       const QString& initialDirectory,
	                                       const bool resolveSymlinks,
	                                       QWidget* parent)
	{
		auto directoryChooser = DirectoryChooser(title, initialDirectory, false, parent);
		directoryChooser.setOption(QFileDialog::DontResolveSymlinks, !resolveSymlinks);

		const auto ret = directoryChooser.exec();
		return ((ret == QFileDialog::Accepted) && !directoryChooser.selectedFiles().isEmpty())
		       ? directoryChooser.selectedFiles()[0]
		       : QString {};
	}

	QStringList DirectoryChooser::getDirectories(const QString& title, const QString& initialDirectory, QWidget* parent)
	{
		auto directoryChooser = DirectoryChooser(title, initialDirectory, true, parent);

		const auto ret = directoryChooser.exec();
		return (ret == QFileDialog::Accepted) && !directoryChooser.selectedFiles().isEmpty()
		       ? directoryChooser.selectedFiles()
		       : QStringList {};
	}

}