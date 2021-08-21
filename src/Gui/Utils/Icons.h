/* Icons.h */

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

#ifndef ICON_LOADER_H_
#define ICON_LOADER_H_

class QString;
class QStringList;
class QIcon;
class QPixmap;
class QSize;

template <typename T>
class QList;


namespace Gui
{
	/**
	 * @brief Icon Utilities
	 * @ingroup Icons
	 */
	namespace Icons
	{

		/**
		 * @brief The IconMode enum
		 * @ingroup Icons
		 */
		enum IconMode
		{
			Automatic,
			ForceStdIcon,
			ForceSayonaraIcon
		};

		/**
		 * @brief The IconName enum
		 * @ingroup Icons
		 */
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
			Grid,
			ImageFile,
			Info,
			LocalLibrary,
			Logo,
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

		/**
		 * @brief icon
		 * @param name
		 * @return
		 */
		QIcon icon(IconName name);
		QIcon icon(IconName name, IconMode mode);

		/**
		 * @brief pixmap
		 * @param name
		 * @return
		 */
		QPixmap pixmap(IconName name, const QSize& size);
		QPixmap pixmap(IconName name, const QSize& size, IconMode mode);

		/**
		 * @brief change_theme
		 */
		void changeTheme();

		QString defaultSystemTheme();
		void setDefaultSystemTheme(const QString& themeName);
	}
}

#endif
