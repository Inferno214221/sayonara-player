/* PlayActionEventHandler.h, (Created on 28.02.2024) */

/* Copyright (C) 2011-2024 Michael Lugmair
 *
 * This file is part of Sayonara Player
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
#ifndef SAYONARA_PLAYER_PLAYACTIONEVENTHANDLER_H
#define SAYONARA_PLAYER_PLAYACTIONEVENTHANDLER_H

#include <memory>

class AbstractLibrary;
class LibraryPlaylistInteractor;
namespace Library
{
	class PlayActionEventHandler
	{
		public:
			enum class TrackSet :
				uint8_t
			{
				All,
				Selected
			};

			virtual ~PlayActionEventHandler() noexcept = default;

			virtual void playNext(TrackSet trackSet) = 0;
			virtual void append(TrackSet trackSet) = 0;
			virtual void play(TrackSet trackSet) = 0;
			virtual void playInNewTab(TrackSet trackSet) = 0;

			static std::shared_ptr<PlayActionEventHandler>
			create(LibraryPlaylistInteractor* playlistInteractor, AbstractLibrary* library);
	};

	using PlayActionEventHandlerPtr = std::shared_ptr<PlayActionEventHandler>;
}

#endif //SAYONARA_PLAYER_PLAYACTIONEVENTHANDLER_H
