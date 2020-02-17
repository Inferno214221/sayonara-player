/* DatabaseSettings.cpp */

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

#include "Database/Settings.h"
#include "Database/Query.h"

#include "Utils/Settings/Settings.h"
#include "Utils/Logger/Logger.h"

using DB::Query;

DB::Settings::Settings(const QString& connection_name, DbId databaseId) :
	DB::Module(connection_name, databaseId) {}

DB::Settings::~Settings() {}

bool DB::Settings::loadSettings(QList<SettingKey>& found_keys)
{
	found_keys.clear();

	const SettingArray& settings = ::Settings::instance()->settings();

	for(AbstrSetting* s : settings)
	{
		if(!s || !s->isDatabaseSetting()) {
			continue;
		}

		QString value;
		QString db_key = s->dbKey();

		bool success = loadSetting(db_key, value);
		if(success) {
			s->assignValue(value);
			found_keys << s->getKey();
		}

		else {

			spLog(Log::Debug, this) << "Setting " << db_key << ": Not found. Use default value...";
			s->assignDefaultValue();
			spLog(Log::Debug, this) << "Load Setting " << db_key << ": " << s->valueToString();
		}
	}

	return true;
}

bool DB::Settings::loadSettings()
{
	QList<SettingKey> keys;
	return loadSettings(keys);
}

bool DB::Settings::storeSettings()
{
	const SettingArray& settings = ::Settings::instance()->settings();

	db().transaction();

	for(AbstrSetting* s : settings)
	{
		if(!s) {
			continue;
		}

		if(s->isDatabaseSetting())
		{
			storeSetting(
				s->dbKey(),
				s->valueToString()
			);
		}
	}

	db().commit();

	return true;
}


bool DB::Settings::loadSetting(QString key, QString& tgt_value)
{
	Query q = runQuery
	(
		"SELECT value FROM settings WHERE key = :key;",
		{":key", key},
		QString("Cannot load setting %1").arg(key)
	);

	if (q.hasError()) {
		return false;
	}

	if(q.next())
	{
		tgt_value = q.value(0).toString();
		return true;
	}

	return false;
}


bool DB::Settings::storeSetting(QString key, const QVariant& value)
{
	Query q = runQuery
	(
		"SELECT value FROM settings WHERE key = :key;",
		{":key", key},
		QString("Store setting: Cannot fetch setting %1").arg(key)
	);

	if (q.hasError()) {
		return false;
	}

	if (!q.next())
	{
		Query q2 = insert("settings",
		{
			{"key", key},
			{"value", value}
		}, QString("Store setting: Cannot insert setting %1").arg(key));

		if (q2.hasError()) {
			return false;
		}

		spLog(Log::Debug, this) << "Inserted " << key << " first time";
	}

	else
	{
		Query q2 = update("settings",
			{{"value", value}},
			{"key", key}
		, QString("Store setting: Cannot update setting %1").arg(key));

		if (q2.hasError()) {
			return false;
		}
	}

	return true;
}

bool DB::Settings::dropSetting(const QString& key)
{
	Query q = runQuery
	(
		"DELETE FROM settings WHERE key = :key;",
		{":key", key},
		QString("Drop setting: Cannot drop setting %1").arg(key)
	);

	return (!q.hasError());
}

