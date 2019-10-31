/* PlaylistDBInterface.h */

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

#ifndef PLAYLISTDBINTERFACE_H
#define PLAYLISTDBINTERFACE_H

#include "Utils/Pimpl.h"
#include "Utils/globals.h"

class QString;
class MetaDataList;

namespace Playlist
{
	/**
	 * @brief The PlaylistDBInterface class
	 * @ingroup Playlists
	 */
	class DBInterface
	{
		PIMPL(DBInterface)

		public:
			explicit DBInterface(const QString& name);
			virtual ~DBInterface();

			int get_id() const;
			void set_id(int db_id);

			QString get_name() const;
			void set_name(const QString& name);

			bool is_temporary() const;
			void set_temporary(bool b);

			bool insert_temporary_into_db();
			Util::SaveAsAnswer save();
			Util::SaveAsAnswer save_as(const QString& str, bool force_override);
			Util::SaveAsAnswer rename(const QString& str);
			bool delete_playlist();
			bool remove_from_db();

			virtual const MetaDataList& tracks() const = 0;
			virtual void set_changed(bool b) = 0;
			virtual bool was_changed() const = 0;
			virtual bool is_storable() const =0;

			static QString request_new_db_name(QString prefix);
	};
}

#endif // PLAYLISTDBINTERFACE_H
