/* PlaylistDBInterface.h */

/* Copyright (C) 2011-2024 Michael Lugmair (Lucio Carreras)
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
class CustomPlaylist;

namespace Playlist
{
	class Playlist;
	class DBInterface
	{
		PIMPL(DBInterface)

		public:
			explicit DBInterface(const QString& name);
			virtual ~DBInterface();

			[[nodiscard]] int id() const;
			void setId(int databaseId);

			[[nodiscard]] QString name() const;
			void setName(const QString& name);

			[[nodiscard]] bool isTemporary() const;
			void setTemporary(bool b);

			[[nodiscard]] bool isLocked() const;
			virtual void setLocked(bool b);

			bool lock();
			bool unlock();

			Util::SaveAsAnswer save();
			Util::SaveAsAnswer saveAs(const QString& newName);
			Util::SaveAsAnswer rename(const QString& newName);
			bool deletePlaylist();

			virtual void setChanged(bool b) = 0;
			[[nodiscard]] virtual bool wasChanged() const = 0;

			[[nodiscard]] virtual const MetaDataList& tracks() const = 0;
	};

	QString requestNewDatabaseName(QString prefix);
	void reloadFromDatabase(Playlist& playlist);
}

#endif // PLAYLISTDBINTERFACE_H
