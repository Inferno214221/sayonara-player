/* PlaylistDBInterface.h */

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

#ifndef PLAYLISTDBINTERFACE_H
#define PLAYLISTDBINTERFACE_H

#include "Utils/Pimpl.h"
#include "Utils/globals.h"

class QString;
class MetaDataList;

namespace Playlist
{
	class DBInterface
	{
		PIMPL(DBInterface)

		public:
			explicit DBInterface(const QString& name);
			virtual ~DBInterface();

			int id() const;
			void setId(int databaseId);

			QString name() const;
			void setName(const QString& name);

			bool isTemporary() const;
			void setTemporary(bool b);

			bool insertTemporaryIntoDatabase();
			Util::SaveAsAnswer save();

			Util::SaveAsAnswer saveAs(const QString& newName);
			bool isSaveAsPossible() const;

			Util::SaveAsAnswer rename(const QString& newName);
			bool deletePlaylist();

			MetaDataList fetchTracksFromDatabase() const;

			virtual const MetaDataList& tracks() const = 0;
			virtual void setChanged(bool b) = 0;
			virtual bool wasChanged() const = 0;
	};

	QString requestNewDatabaseName(QString prefix);
}

#endif // PLAYLISTDBINTERFACE_H
