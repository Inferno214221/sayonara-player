/* IconProvider.cpp */

/* Copyright (C) 2011-2017  Lucio Carreras
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
#include "Utils/Settings/Settings.h"

#include "GUI/Utils/GuiUtils.h"

IconProvider::IconProvider() :
	QFileIconProvider()
{
	_settings = Settings::instance();

	_folder_icon.addPixmap(Gui::Util::pixmap("folder"), QIcon::Normal, QIcon::Off);
	_folder_icon.addPixmap(Gui::Util::pixmap("folder_open"), QIcon::Normal, QIcon::On);
}

IconProvider::~IconProvider() {}

QIcon IconProvider::icon(IconType type) const
{
	return QFileIconProvider::icon(type);
}

QIcon IconProvider::icon(const QFileInfo &info) const
{
	if(_settings->get(Set::Player_Style) == 0){
		return QFileIconProvider::icon(info);
	}

	if(info.isDir()){
		return _folder_icon;
	}

	if(info.isFile() && Util::File::is_playlistfile(info.filePath())){
		return Gui::Util::icon("playlistfile");
	}

	return QFileIconProvider::icon(info);
}
