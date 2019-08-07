/* PlaylistStopBehavior.h */

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



#ifndef STOPBEHAVIOR_H
#define STOPBEHAVIOR_H

#include "Utils/Pimpl.h"

class MetaDataList;
class MetaData;

namespace Playlist
{
	class StopBehavior
	{
		PIMPL(StopBehavior)

		public:
			StopBehavior();
			virtual ~StopBehavior();

			virtual MetaDataList tracks() const=0;
			virtual MetaData track(int index) const=0;
			virtual int count() const=0;

			int restore_track_before_stop();

			int track_idx_before_stop() const;
			void set_track_idx_before_stop(int idx);
	};
}

#endif // STOPBEHAVIOR_H
