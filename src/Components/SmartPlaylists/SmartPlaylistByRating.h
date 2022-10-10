/* SmartPlaylistByRating.h */
/*
 * Copyright (C) 2011-2022 Michael Lugmair
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
#ifndef SAYONARA_PLAYER_SMARTPLAYLISTBYRATING_H
#define SAYONARA_PLAYER_SMARTPLAYLISTBYRATING_H

#include "SmartPlaylist.h"

class SmartPlaylistByRating :
	public SmartPlaylist
{
	public:
		static constexpr const auto ClassType = "star-rating";

		SmartPlaylistByRating(int id, int ratingFrom, int ratingTo, bool isRandomized, LibraryId libraryId);
		~SmartPlaylistByRating() override;

		[[nodiscard]] int minimumValue() const override;
		[[nodiscard]] int maximumValue() const override;

		[[nodiscard]] QString classType() const override;
		[[nodiscard]] QString displayClassType() const override;
		[[nodiscard]] QString name() const override;
		[[nodiscard]] SmartPlaylists::Type type() const override;

		MetaDataList filterTracks(MetaDataList tracks) override;
};

#endif //SAYONARA_PLAYER_SMARTPLAYLISTBYRATING_H
