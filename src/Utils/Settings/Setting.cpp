/* Setting.cpp */

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

#include "Setting.h"
#include "Utils/Logger/Logger.h"
#include "Database/Settings.h"

#include <QVariant>

struct AbstrSetting::Private
{
	QString			db_key;
	SettingKey	key;
	bool			db_setting;
};


AbstrSetting::AbstrSetting()
{
	m = Pimpl::make<Private>();
}

AbstrSetting::AbstrSetting(SettingKey key) :
	AbstrSetting()
{
	m->key = key;
	m->db_setting = false;
}

AbstrSetting::AbstrSetting(SettingKey key, const char* db_key) :
	AbstrSetting(key)
{
	m->db_key = db_key;
	m->db_setting = true;
}


AbstrSetting::AbstrSetting(const AbstrSetting& other) :
	AbstrSetting()
{
	m->key = other.m->key;
	m->db_key = other.m->db_key;
	m->db_setting = other.m->db_setting;
}

AbstrSetting& AbstrSetting::operator=(const AbstrSetting& other)
{
	m->key = other.m->key;
	m->db_key = other.m->db_key;
	m->db_setting = other.m->db_setting;

	return *this;
}

AbstrSetting::~AbstrSetting() {}

SettingKey AbstrSetting::get_key() const
{
	return m->key;
}

QString AbstrSetting::db_key() const
{
	return m->db_key;
}

bool AbstrSetting::is_db_setting() const
{
	return m->db_setting;
}

void AbstrSetting::assign_value(const QString& value)
{
	if(!m->db_setting) {
		return;
	}

	bool success = load_value_from_string(value);

	if(!success)
	{
		sp_log(Log::Warning, this) << "Setting " << m->db_key << ": Cannot convert. Use default value...";
		assign_default_value();
	}
}
