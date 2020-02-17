/* DatabaseSearchMode.cpp */

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

#include "Database/SearchableModule.h"
#include "Database/Query.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Utils.h"
#include "Utils/Logger/Logger.h"

using DB::SearchableModule;

struct SearchableModule::Private
{
	bool initialized;
	Library::SearchModeMask search_mode;

	Private() :
		initialized(false),
		search_mode(0)
	{}
};

SearchableModule::SearchableModule(const QString& connection_name, DbId databaseId) :
	DB::Module(connection_name, databaseId)
{
	m = Pimpl::make<Private>();
}

SearchableModule::~SearchableModule() = default;

void SearchableModule::init()
{
	if(m->initialized){
		return;
	}

	Settings* settings = Settings::instance();
	AbstrSetting* s = settings->setting(SettingKey::Lib_SearchMode);
	QString db_key = s->dbKey();

	Query q_select(this);
	q_select.prepare("SELECT value FROM settings WHERE key = :key;");
	q_select.bindValue(":key", Util::convertNotNull(db_key));
	if(q_select.exec())
	{
		if(q_select.next()) {
			m->search_mode = q_select.value(0).toInt();
			m->initialized = true;
		}

		else {
			spLog(Log::Warning, this) << "Cannot find library search mode";
		}
	}

	else {
		q_select.showError("Cannot fetch library search mode");
	}
}

Library::SearchModeMask SearchableModule::init_search_mode()
{
	init();

	return m->search_mode;
}

void SearchableModule::updateSearchMode(::Library::SearchModeMask search_mode)
{
	if(m->search_mode != search_mode)
	{
		Settings* settings = Settings::instance();
		AbstrSetting* s = settings->setting(SettingKey::Lib_SearchMode);
		QString db_key = s->dbKey();

		Library::SearchModeMask search_mode = settings->get<Set::Lib_SearchMode>();

		Query q_update(this);
		q_update.prepare("UPDATE settings SET value=:search_mode WHERE key = :key;");
		q_update.bindValue(":search_mode",	search_mode);
		q_update.bindValue(":key",			Util::convertNotNull(db_key));
		if(!q_update.exec()) {
			q_update.showError("Cannot update search mode");
		}

		m->search_mode = search_mode;
	}

	m->initialized = true;
}

::Library::SearchModeMask SearchableModule::searchMode() const
{
	return m->search_mode;
}
