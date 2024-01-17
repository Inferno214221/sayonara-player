/* PlaylistModifiers.h */
/*
 * Copyright (C) 2011-2024 Michael Lugmair
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

#ifndef SAYONARA_PLAYER_PLAYLISTMODIFIERS_H
#define SAYONARA_PLAYER_PLAYLISTMODIFIERS_H

#include "Utils/typedefs.h"
#include "Utils/Library/Sortorder.h"

class MetaDataList;

namespace Playlist
{
	class Playlist;

	enum Reason
	{
		DynamicPlayback = 0,
		ReloadFromDatabase,
		Library,
		AsyncPlaylistCreator,
		StreamHistory,
		TracksDeleted,
		UserInterface,
		Undefined
	};

	enum class Operation :
		uint8_t
	{
		Arrange = 0,
		Append,
		Clear,
		Duplicate,
		EnableAll,
		Insert,
		Rebuild,
		Remove
	};

	void reverse(Playlist& playlist, Reason reason);
	void randomize(Playlist& playlist, Reason reason);
	void sortTracks(Playlist& playlist, Library::TrackSortorder sortOrder, Reason reason);

	IndexSet moveTracks(Playlist& playlist, const IndexSet& indexes, int targetRow, Reason reason);
	IndexSet copyTracks(Playlist& playlist, const IndexSet& indexes, int targetRow, Reason reason);
	void insertTracks(Playlist& playlist, const MetaDataList& tracks, int targetRow, Reason reason);
	void appendTracks(Playlist& playlist, const MetaDataList& tracks, Reason reason);
	void removeTracks(Playlist& playlist, const IndexSet& indexes, Reason reason);
	void clear(Playlist& playlist, Reason reason);
	int count(const Playlist& playlist);
	void enableAll(Playlist& playlist, Reason reason);

	MilliSeconds runningTime(const Playlist& playlist);
	void jumpToNextAlbum(Playlist& playlist);
	int currentTrackWithoutDisabled(const Playlist& playlist);
}

#endif //SAYONARA_PLAYER_PLAYLISTMODIFIERS_H
