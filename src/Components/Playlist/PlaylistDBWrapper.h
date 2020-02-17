/* PlaylistDBWrapper.h */

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

#ifndef PLAYLISTDB_WRAPPER_H
#define PLAYLISTDB_WRAPPER_H

#include "Utils/Pimpl.h"
#include "Utils/Playlist/CustomPlaylistFwd.h"
#include "Database/Playlist.h"

namespace Playlist
{
	/**
	 * @brief DBWrapper is responsible for fetching playlist data from database,
	 * especially the handling between skeleton and the playlist itself
	 * Most of the functions are wrappers for the DatabasePlaylist class
	 * @ingroup Playlists
	 */
	class DBWrapper
	{
		PIMPL(DBWrapper)

		public:
			DBWrapper();
			~DBWrapper();

			bool getSkeletons(CustomPlaylistSkeletons& skeletons,
								   PlaylistStoreType type,
								   PlaylistSortOrder so=PlaylistSortOrder::IDAsc);

			bool getAllSkeletons(CustomPlaylistSkeletons& skeletons,
								   PlaylistSortOrder so=PlaylistSortOrder::IDAsc);

			bool getNonTemporarySkeletons(CustomPlaylistSkeletons& skeletons,
								   PlaylistSortOrder so=PlaylistSortOrder::IDAsc);

			bool getTemporarySkeletons(CustomPlaylistSkeletons& skeletons,
										 PlaylistSortOrder so);

			bool getAllPlaylists(CustomPlaylists& playlists,
								   PlaylistSortOrder so=PlaylistSortOrder::IDAsc);

			bool getTemporaryPlaylists(CustomPlaylists& playlists,
										 PlaylistSortOrder so=PlaylistSortOrder::IDAsc);

			bool getNonTemporaryPlaylists(CustomPlaylists& playlists,
											 PlaylistSortOrder so=PlaylistSortOrder::IDAsc);

			CustomPlaylist getPlaylistById(int id);
			CustomPlaylist getPlaylistByName(const QString& name);

			bool renamePlaylist(int id, const QString& new_name);
			bool savePlaylistAs(const MetaDataList& v_md, const QString& name);
			bool savePlaylistTemporary(const MetaDataList& v_md, const QString& name);
			bool savePlaylist(const CustomPlaylist& pl);
			bool savePlaylist(const MetaDataList& v_md, int id, bool is_temporary);

			bool deletePlaylist(int id);
			bool deletePlaylist(const QString& name);
			bool exists(const QString& name);

		private:
			void applyTags(MetaDataList& v_md);
			bool getPlaylists(CustomPlaylists& playlists,
							   PlaylistStoreType type,
							   PlaylistSortOrder sortorder);
	};

	using DBWrapperPtr=std::shared_ptr<DBWrapper>;
}

#endif // PLAYLISTDBCONNECTOR_H
