/* Icons.h */

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

#ifndef ICON_LOADER_H_
#define ICON_LOADER_H_

class QString;
class QStringList;
class QIcon;
class QPixmap;

template <typename T>
class QList;


namespace Gui
{
	/**
	 * @brief The Icons class
	 * @ingroup GUIHelper
	 */
	namespace Icons
	{
		enum IconMode
		{
			Automatic,
			ForceStdIcon,
			ForceSayonaraIcon
		};

		enum IconName
		{
			Append=0,		// Playlist
			AudioFile,
			Backward,
			Clear,
			Close,
			Delete,
			Dynamic,
			Edit,
			Exit,
			File,
			FileManager,
			Folder,
			FolderOpen,
			Forward,
			Gapless,
			ImageFile,
			Info,
			LocalLibrary,
			Lyrics,
			New,
			Next,
			Open,
			Pause,
			Play,
			PlayBorder,
			PlaylistFile,
			PlaySmall,
			Preferences,
			Previous,
			Record,
			Refresh,
			Remove,
			Rename,
			Repeat1,
			RepeatAll,
			Save,
			SaveAs,
			Search,
			Shuffle,
			Shutdown,
			Star,
			StarDisabled,
			Stop,
			Table,
			Undo,
			Vol1,
			Vol2,
			Vol3,
			VolMute
		};

		QIcon icon(IconName name);
		QIcon icon(IconName name, IconMode mode);

		QPixmap pixmap(IconName name);
		QPixmap pixmap(IconName name, IconMode mode);

		void set_standard_theme(const QString& name);
		QString standard_theme();

		void force_standard_icons(bool b);

		void change_theme();
	}
}

#endif
