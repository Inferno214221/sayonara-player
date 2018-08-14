/* DatabaseSettings.cpp */

/* Copyright (C) 2011-2017  Lucio Carreras
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

DB::Settings::Settings(const QString& connection_name, DbId db_id) :
	DB::Module(connection_name, db_id) {}

DB::Settings::~Settings() {}

bool DB::Settings::load_settings()
{
	const SettingArray& settings = ::Settings::instance()->settings();

	for(AbstrSetting* s : settings)
	{
		if(!s || !s->is_db_setting()) {
			continue;
		}

		QString value;
		QString db_key = s->db_key();

		bool success = load_setting(db_key, value);
		if(success) {
			s->assign_value(value);
		}

		else {
			sp_log(Log::Info, this) << "Setting " << db_key << ": Not found. Use default value...";
			s->assign_default_value();
			sp_log(Log::Info, this) << "Load Setting " << db_key << ": " << s->value_to_string();
		}
	}

	return true;
}

bool DB::Settings::store_settings()
{
	const SettingArray& settings = ::Settings::instance()->settings();

	db().transaction();

	for(AbstrSetting* s : settings)
	{
		if(!s) {
			continue;
		}

		if(s->is_db_setting())
		{
			store_setting(
				s->db_key(),
				s->value_to_string()
			);
		}
	}

	db().commit();

	return true;
}


bool DB::Settings::load_setting(QString key, QString& tgt_value)
{
	Query q(this);
	q.prepare("SELECT value FROM settings WHERE key = ?;");
	q.addBindValue(QVariant(key));

	if (!q.exec()) {
		q.show_error(QString("Cannot load setting ") + key);
		return false;
	}

	if(q.next()) {
		tgt_value = q.value(0).toString();
		return true;
	}

	return false;
}


bool DB::Settings::store_setting(QString key, const QVariant& value)
{
	Query q(this);
	q.prepare("SELECT value FROM settings WHERE key = :key;");
	q.bindValue(":key", key);

	if (!q.exec()) {
		q.show_error(QString("Store setting: Cannot fetch setting ") + key);
		return false;
	}

	if (!q.next()) {
		q.prepare("INSERT INTO settings VALUES(:key, :val);");
		q.bindValue(":key", key);
		q.bindValue(":value", value);

		if (!q.exec()) {
			q.show_error(QString("Store setting: Cannot insert setting ") + key);
			return false;
		}

		sp_log(Log::Info) << "Inserted " << key << " first time";
	}

	q.prepare("UPDATE settings SET value=:value WHERE key=:key;");
	q.bindValue(":key", key);
	q.bindValue(":value", value);

	if (!q.exec()) {
		q.show_error(QString("Store setting: Cannot update setting ") + key);
		return false;
	}

	return true;
}

