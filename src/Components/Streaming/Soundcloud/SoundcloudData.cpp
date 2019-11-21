/* SoundcloudData.cpp */

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

#include "SoundcloudData.h"
#include "SoundcloudWebAccess.h"

#include "Utils/Utils.h"
#include "Utils/Logger/Logger.h"
#include "Utils/globals.h"

#include "Database/Query.h"
#include "Database/Connector.h"

#include <QList>

using DB::Query;

SC::Database::Database() :
	::DB::Base(25, ":/Database", Util::sayonara_path(), "soundcloud.db")
{
	this->apply_fixes();
}

SC::Database::~Database()
{
	this->close_db();
}

QString SC::Database::load_setting(const QString& key)
{
	Query q = this->run_query
	(
		"SELECT value FROM Settings WHERE key=:key;",
		{{":key", key}},
		QString("Cannot load setting %1").arg(key)
	);

	if(q.has_error()){
		return QString();
	}

	if(q.next()){
		return q.value(0).toString();
	}

	return QString();
}

bool SC::Database::save_setting(const QString& key, const QString& value)
{
	QString v = load_setting(key);
	if(v.isNull()){
		return insert_setting(key, value);
	}

	Query q = this->update
	(
		"settings",
		{
			{"key", key},
			{"value", value}
		},
		{"key", key},
		QString("Cannot apply setting %1").arg(key)
	);

	return (q.has_error() == false);
}

bool SC::Database::insert_setting(const QString& key, const QString& value)
{
	Query q = this->insert
	(
		"settings",
		{
			{"key", key},
			{"value", value}
		},
		QString("Cannot insert setting %1").arg(key)
	);

	return (q.has_error() == false);
}


bool SC::Database::apply_fixes()
{
	QString creation_string =
		"CREATE TABLE Settings "
		"( "
		"  key VARCHAR(100) PRIMARY KEY, "
		"  value TEXT "
		");";

	bool success = check_and_create_table("Settings", creation_string);
	if(!success){
		sp_log(Log::Error, this) << "Cannot create settings table for soundcloud";
		return false;
	}

	int version;
	QString version_string = load_setting("version");
	if(version_string.isEmpty()){
		save_setting("version", "1");
		version = 1;
	}

	else{
		version = version_string.toInt();
	}

	if(version < 2){
		bool success = check_and_insert_column("tracks", "albumArtistID", "integer", "-1");
		if(success){
			save_setting("version", "2");
		}
	}

	if(version < 3) {
		bool success = check_and_insert_column("tracks", "libraryID", "integer", "0");
		if(success){
			save_setting("version", "3");
		}
	}


	if(version < 4) {
		bool success = check_and_insert_column("tracks", "fileCissearch", "varchar(256)", "");
		if(success){
			save_setting("version", "4");
		}
	}

	return true;
}
