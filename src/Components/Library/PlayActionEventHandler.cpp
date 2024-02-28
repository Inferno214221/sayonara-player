/* PlayActionEventHandler.cpp, (Created on 28.02.2024) */

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
#include "PlayActionEventHandler.h"

#include "Components/Library/AbstractLibrary.h"
#include "Components/Playlist/LibraryPlaylistInteractor.h"

namespace
{
	class PlayActionEventHandlerImpl :
		public Library::PlayActionEventHandler
	{
		public:
			PlayActionEventHandlerImpl(LibraryPlaylistInteractor* playlistInteractor, AbstractLibrary* library) :
				m_library {library},
				m_playlistInteractor {playlistInteractor} {}

			~PlayActionEventHandlerImpl() noexcept override = default;

			void playNext(const TrackSet trackSet) override
			{
				m_playlistInteractor->insertAfterCurrentTrack(fetchTracks(trackSet));
			}

			void append(const TrackSet trackSet) override
			{
				m_playlistInteractor->append(fetchTracks(trackSet));
			}

			void play(const TrackSet trackSet) override
			{
				m_playlistInteractor->createPlaylist(fetchTracks(trackSet), false);
			}

			void playInNewTab(const TrackSet trackSet) override
			{
				m_playlistInteractor->createPlaylist(fetchTracks(trackSet), true);
			}

		private:
			[[nodiscard]] const MetaDataList& fetchTracks(const TrackSet trackSet) const
			{
				return (trackSet == TrackSet::All)
				       ? m_library->tracks()
				       : m_library->currentTracks();
			}

			AbstractLibrary* m_library;
			LibraryPlaylistInteractor* m_playlistInteractor;
	};
}

namespace Library
{
	std::shared_ptr<PlayActionEventHandler>
	PlayActionEventHandler::create(LibraryPlaylistInteractor* playlistInteractor, AbstractLibrary* library)
	{
		return std::make_shared<PlayActionEventHandlerImpl>(playlistInteractor, library);
	}
}
