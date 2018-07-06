/* DatabaseSearchMode.cpp */

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

#include "Database/DatabaseSearchMode.h"
#include "Database/SayonaraQuery.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Utils.h"

using DB::SearchMode;

struct SearchMode::Private
{
	bool initialized;
	Library::SearchModeMask search_mode;

	Private() :
		initialized(false),
		search_mode(0)
	{}
};

SearchMode::SearchMode(const QSqlDatabase& db, DbId db_id) :
	DB::Module(db, db_id)
{
	m = Pimpl::make<SearchMode::Private>();
}

SearchMode::~SearchMode() {}


void SearchMode::init()
{
	if(m->initialized){
		return;
	}

	Settings* settings = Settings::instance();
	AbstrSetting* s = settings->setting(SettingKey::Lib_SearchMode);
	QString db_key = s->db_key();

	Query q_select(this);
	q_select.prepare("SELECT value FROM settings WHERE key = :key;");
	q_select.bindValue(":key", Util::cvt_not_null(db_key));
	if(q_select.exec())
	{
		if(q_select.next()){
			m->search_mode = q_select.value(0).toInt();
			m->initialized = true;
		}

		else {
			sp_log(Log::Warning, this) << "Cannot find library search mode";
		}
	}

	else {
		q_select.show_error("Cannot fetch library search mode");
	}
}

Library::SearchModeMask SearchMode::search_mode()
{
	init();

	return m->search_mode;
}

void SearchMode::update_search_mode()
{
	Settings* settings = Settings::instance();
	AbstrSetting* s = settings->setting(SettingKey::Lib_SearchMode);
	QString db_key = s->db_key();

	Library::SearchModeMask search_mode = settings->get<Set::Lib_SearchMode>();

	Query q_update(this);
	q_update.prepare("UPDATE settings SET value=:search_mode WHERE key = :key;");
	q_update.bindValue(":search_mode",	search_mode);
	q_update.bindValue(":key",			Util::cvt_not_null(db_key));
	if(!q_update.exec()){
		q_update.show_error("Cannot update search mode");
	}

	m->search_mode = search_mode;
	m->initialized = true;
}
