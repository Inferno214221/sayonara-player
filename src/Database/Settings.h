/* DatabaseSettings.h */

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

#ifndef DATABASESETTINGS_H
#define DATABASESETTINGS_H

#include "Database/Module.h"
#include "Utils/Settings/SettingKey.h"

class QVariant;

namespace DB
{
	class Settings :
			private Module
	{
	public:
		Settings(const QString& connection_name, DbId db_id);
		~Settings();

		bool load_setting(QString key, QString& val);
		bool store_setting(QString key, const QVariant& val);
		bool drop_setting(const QString& key);

		bool load_settings();
		bool load_settings(QList<SettingKey>& found_keys);
		bool store_settings();
	};
}

#endif // DATABASESETTINGS_H
