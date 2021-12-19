/* ShuffleBehavior.h */
/*
 * Copyright (C) 2011-2021 Michael Lugmair
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
#ifndef SAYONARA_PLAYER_PLAYLISTSHUFFLEHISTORY_H
#define SAYONARA_PLAYER_PLAYLISTSHUFFLEHISTORY_H

#include "Utils/Pimpl.h"
#include "Utils/MetaData/LibraryItem.h"

class MetaData;

namespace Playlist
{
	class Playlist;
	class ShuffleHistory
	{
		PIMPL(ShuffleHistory)

		public:
			explicit ShuffleHistory(Playlist* playlist);
			~ShuffleHistory();

			void addTrack(const MetaData& track);
			void popBack();

			[[nodiscard]] int nextTrackIndex(bool isRepeatAllEnabled) const;
			[[nodiscard]] int previousShuffleIndex() const;

			void replaceTrack(const MetaData& oldTrack, const MetaData& newTrack);
			void clear();
	};
}

#endif //SAYONARA_PLAYER_PLAYLISTSHUFFLEHISTORY_H
