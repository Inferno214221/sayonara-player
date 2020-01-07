/* SoundcloudData.h */

/* Copyright (C) 2011-2020  Lucio Carreras
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

#ifndef SOUNDCLOUDDATA_H
#define SOUNDCLOUDDATA_H

#include "Database/Base.h"

#include <QObject>

/* this is the database interface
 * TODO: make database connector my parent
 * TODO: create real (new) database
 */
class MetaData;
class MetaDataList;
class Artist;
class AlbumList;

namespace SC
{
	class Database :
			public DB::Base
	{
		public:
			Database();
			~Database() override;

			// todo: assure to be called
			bool apply_fixes() override;

			QString load_setting(const QString& key);
			bool save_setting(const QString& key, const QString& value);
			bool insert_setting(const QString& key, const QString& value);
	};
}

#endif // SOUNDCLOUDDATA_H
