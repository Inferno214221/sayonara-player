/* IconProvider.cpp */

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

#include "DirectoryIconProvider.h"
#include "Utils/FileUtils.h"

#include "Utils/Algorithm.h"
#include "Gui/Utils/Icons.h"
#include "Gui/Utils/Style.h"

#include <QList>
#include <QPixmap>

using Directory::IconProvider;

namespace
{
	QList<QPixmap> extractPixmaps(const QIcon& icon)
	{
		QList<QPixmap> result;

		const auto availableSizes = icon.availableSizes();
		Util::Algorithm::transform(availableSizes, result, [&](const auto& size){
			return icon.pixmap(size);
		});

		return result;
	}

	void insertPixmaps(QIcon& icon, QIcon::Mode mode, QIcon::State state, const QList<QPixmap>& pixmaps)
	{
		for(const auto& pixmap : pixmaps)
		{
			icon.addPixmap(pixmap, mode, state);
		}
	}

	void fillIcon(QIcon& icon, Gui::Icons::IconName iconName, QIcon::Mode iconMode)
	{
		const auto fetchedIcon = Gui::Icons::icon(iconName);
		const auto pixmaps = extractPixmaps(fetchedIcon);
		insertPixmaps(icon, iconMode, QIcon::State::On, pixmaps);
	}
}

IconProvider::IconProvider() :
	QFileIconProvider()
{}

IconProvider::~IconProvider() = default;

QIcon IconProvider::icon(IconType type) const
{
	if(type==IconType::Folder)
	{
		QIcon icon;
		fillIcon(icon, Gui::Icons::Folder, QIcon::Mode::Normal);
		fillIcon(icon, Gui::Icons::FolderOpen, QIcon::Mode::Selected);
		return icon;
	}

	return QFileIconProvider::icon(type);
}

QIcon IconProvider::icon(const QFileInfo& info) const
{
	if(!Style::isDark())
	{
		return QFileIconProvider::icon(info);
	}

	if(info.isDir())
	{
		QIcon icon;
		fillIcon(icon, Gui::Icons::Folder, QIcon::Mode::Normal);
		fillIcon(icon, Gui::Icons::FolderOpen, QIcon::Mode::Selected);
		return icon;
	}

	if(info.isFile() && Util::File::isPlaylistFile(info.filePath()))
	{
		return Gui::Icons::icon(Gui::Icons::PlaylistFile);
	}

	return QFileIconProvider::icon(info);
}
