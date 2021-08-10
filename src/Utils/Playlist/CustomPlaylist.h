/* CustomPlaylist.h */

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

#ifndef CUSTOMPLAYLIST_H
#define CUSTOMPLAYLIST_H

#include "Utils/Pimpl.h"

class QString;
class MetaDataList;

class CustomPlaylist
{
	PIMPL(CustomPlaylist)

	public:
		CustomPlaylist();
		CustomPlaylist(const CustomPlaylist& other);
		CustomPlaylist(CustomPlaylist&& other) noexcept;

		CustomPlaylist& operator=(const CustomPlaylist& other);
		CustomPlaylist& operator=(CustomPlaylist&& other) noexcept;

		~CustomPlaylist();

		int id() const;
		void setId(int id);

		QString name() const;
		void setName(const QString& name);

		bool isTemporary() const;
		void setTemporary(bool temporary);

		int tracksToFetch() const;
		void setTracksToFetch(int track);

		MetaDataList tracks() const;
		void setTracks(const MetaDataList& tracks);
		void setTracks(MetaDataList&& tracks);
};

#endif // CUSTOMPLAYLIST_H
