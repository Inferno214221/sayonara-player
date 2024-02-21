/* PlaylistChooser.h */

/* Copyright (C) 2011-2024 Michael Lugmair (Lucio Carreras)
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

#include "Utils/Pimpl.h"
#include "Utils/globals.h"

#include <QList>
#include <QObject>

class CustomPlaylist;

namespace Playlist
{
	class Creator;
	class Chooser :
		public QObject
	{
		Q_OBJECT
		PIMPL(Chooser)

		signals:
			void sigPlaylistsChanged();

		public:
			Chooser(Creator* playlistCreator, QObject* parent);
			~Chooser() override;

			void loadSinglePlaylist(int id);
			[[nodiscard]] int findPlaylist(const QString& playlist) const;

			const QList<CustomPlaylist>& playlists();

			Util::SaveAsAnswer renamePlaylist(int id, const QString& newName);
			bool deletePlaylist(int id);

		private slots:
			void playlistsChanged();

			void playlistDeleted(int id);
			void playlistAdded(int id, const QString& name);
			void playlistRenamed(int id, const QString& old_name, const QString& new_name);
	};
}

#endif /* PLAYLISTS_H_ */
