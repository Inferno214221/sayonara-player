/* DatabaseSettings.h */

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
		Settings(const QString& connectionName, DbId databaseId);
		~Settings();

		bool loadSetting(QString key, QString& val);
		bool storeSetting(QString key, const QVariant& val);
		bool dropSetting(const QString& key);

		bool loadSettings();
		bool loadSettings(QList<SettingKey>& found_keys);
		bool storeSettings();
	};
}

#endif // DATABASESETTINGS_H
