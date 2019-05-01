/* IconProvider.cpp */

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

#include "DirectoryIconProvider.h"
#include "Utils/Utils.h"
#include "Utils/FileUtils.h"

#include "GUI/Utils/GuiUtils.h"
#include "GUI/Utils/Icons.h"
#include "GUI/Utils/Style.h"

IconProvider::IconProvider() :
	QFileIconProvider()
{}

IconProvider::~IconProvider() {}

QIcon IconProvider::icon(IconType type) const
{
	if(type==IconType::Folder)
	{
		QPixmap pm = Gui::Icons::pixmap(Gui::Icons::Folder);
		QPixmap pm_open = Gui::Icons::pixmap(Gui::Icons::FolderOpen);

		QIcon icon;

		icon.addPixmap(pm, QIcon::Mode::Normal, QIcon::State::On);
		icon.addPixmap(pm_open, QIcon::Mode::Selected, QIcon::State::On);

		return icon;
	}

	return QFileIconProvider::icon(type);
}

QIcon IconProvider::icon(const QFileInfo &info) const
{
	if(!Style::is_dark())
	{
		return QFileIconProvider::icon(info);
	}

	if(info.isDir())
	{
		QPixmap pm = Gui::Icons::pixmap(Gui::Icons::Folder);
		QPixmap pm_open = Gui::Icons::pixmap(Gui::Icons::FolderOpen);

		QIcon icon;

		icon.addPixmap(pm, QIcon::Mode::Normal, QIcon::State::On);
		icon.addPixmap(pm_open, QIcon::Mode::Selected, QIcon::State::On);

		return icon;
	}

	if(info.isFile() && Util::File::is_playlistfile(info.filePath()))
	{
		return Gui::Icons::icon(Gui::Icons::PlaylistFile);
	}

	return QFileIconProvider::icon(info);
}
