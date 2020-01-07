/* PlaylistChooser.h */

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

#ifndef PLAYLISTCHOOSER_H_
#define PLAYLISTCHOOSER_H_

#include "Utils/Playlist/CustomPlaylistFwd.h"
#include "Utils/Pimpl.h"
#include "Utils/globals.h"
#include <QObject>

namespace Playlist
{
	/**
	 * @brief The Chooser class is used to select playlists out of
	 * all saved playlists
	 * @ingroup Playlist
	 */
	class Chooser :
			public QObject
	{
		Q_OBJECT
		PIMPL(Chooser)

		public:
			Chooser(QObject* parent);
			~Chooser();

			void load_single_playlist(int id);
			int find_playlist(const QString& name) const;

			const CustomPlaylistSkeletons& playlists();

			Util::SaveAsAnswer rename_playlist(int id, const QString& new_name);
			bool delete_playlist(int id);

		signals:
			void sig_playlists_changed();

		private slots:
			void playlists_changed();

			void playlist_deleted(int id);
			void playlist_added(int id, const QString& name);
			void playlist_renamed(int id, const QString& old_name, const QString& new_name);
	};
}

#endif /* PLAYLISTS_H_ */
