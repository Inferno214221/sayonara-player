/* PlaylistChooser.h */

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

#ifndef PLAYLISTCHOOSER_H_
#define PLAYLISTCHOOSER_H_

#include "Utils/Playlist/CustomPlaylistFwd.h"
#include "Utils/Pimpl.h"
#include <QObject>

/**
 * @brief The PlaylistChooser class
 * @ingroup Components
 */
class PlaylistChooser :
		public QObject
{
	Q_OBJECT
	PIMPL(PlaylistChooser)

public:
	PlaylistChooser();
	~PlaylistChooser();

	void load_single_playlist(int id);
	int find_playlist(const QString& name) const;

	const CustomPlaylistSkeletons& playlists();

signals:
	void sig_playlists_changed();

private slots:
	void playlists_changed();
};

#endif /* PLAYLISTS_H_ */
