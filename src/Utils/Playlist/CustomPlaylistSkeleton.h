/* CustomPlaylistSkeleton.h */

/* Copyright (C) 2011-2019  Lucio Carreras
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

#ifndef CUSTOM_PLAYLIST_SKELETON_H
#define CUSTOM_PLAYLIST_SKELETON_H

#include "Utils/Pimpl.h"

/**
 * @brief The CustomPlaylistSkeleton class
 * @ingroup PlaylistHelper
 */
class CustomPlaylistSkeleton
{
	PIMPL(CustomPlaylistSkeleton)

	public:
		int id() const;
		void set_id(int id);

		QString name() const;
		void set_name(const QString& name);

		bool temporary() const;
		void set_temporary(bool temporary);

		virtual int num_tracks() const;
		virtual void set_num_tracks(int num_tracks);

		CustomPlaylistSkeleton();
		CustomPlaylistSkeleton(const CustomPlaylistSkeleton& other);
		CustomPlaylistSkeleton& operator=(const CustomPlaylistSkeleton& other);
		virtual ~CustomPlaylistSkeleton();
};

#endif

