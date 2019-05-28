/* Settings.cpp */

/* Copyright (C) 2011-2019 Lucio Carreras
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

#include <iostream>
#include "Utils/Settings/Settings.h"
#include "Utils/Settings/SettingRegistry.h"
#include "Utils/typedefs.h"

#include "Utils/Crypt.h"
#include "Utils/Utils.h"
#include "Utils/Language/Language.h"

#include <array>
#include <iterator>

struct Settings::Private
{
	QString			version;
	std::array<AbstrSetting*, static_cast<int>(SettingKey::Num_Setting_Keys)> settings;
	bool			initialized;

	Private()
	{
		std::fill(settings.begin(), settings.end(), nullptr);
		initialized = false;
	}
};

Settings::Settings()
{
	m = Pimpl::make<Private>();
}

Settings::~Settings ()
{
	for(size_t i=0; i<m->settings.size(); i++)
	{
		delete m->settings[i];
		m->settings[i] = nullptr;
	}
}

AbstrSetting* Settings::setting(SettingKey key) const
{
	return m->settings[(int) key];
}

const SettingArray& Settings::settings()
{
	return m->settings;
}

void Settings::register_setting(AbstrSetting* s)
{
	SettingKey key  = s->get_key();
	m->settings[ (int) key ] = s;
}


bool Settings::check_settings()
{
	if(m->initialized){
		return true;
	}

	SettingRegistry::init();
	bool has_empty = std::any_of(m->settings.begin(), m->settings.end(), [](AbstrSetting* s){
		return (s==nullptr);
	});

	m->initialized = (!has_empty);
	return m->initialized;
}


void Settings::apply_fixes()
{
	int settings_revision = this->get<Set::Settings_Revision>();
	if(settings_revision < 1)
	{
		// Create Crypt keys
		QByteArray priv_key = ::Util::random_string(32).toLocal8Bit();
		this->set<Set::Player_PrivId>(priv_key);

		QByteArray pub_key = ::Util::random_string(32).toLocal8Bit();
		this->set<Set::Player_PublicId>(pub_key);

		// Crypt Last FM password
		StringPair lfm_pw = this->get<Set::LFM_Login>();
		this->set<Set::LFM_Username>(lfm_pw.first);
		this->set<Set::LFM_Password>(Util::Crypt::encrypt(lfm_pw.second));
		this->set<Set::LFM_Login>(StringPair("", ""));

		// Crypt Proxy Password
		QString proxy_pw = this->get<Set::Proxy_Password>();
		this->set<Set::Proxy_Password>(Util::Crypt::encrypt(proxy_pw));

		this->set<Set::Settings_Revision>(1);
	}

	if(settings_revision < 2)
	{
		QString language = this->get<Set::Player_Language>();
		QString four_letter = Lang::convert_old_lang(language);
		this->set<Set::Player_Language>(four_letter);

		this->set<Set::Settings_Revision>(2);
	}


	if(get<Set::Player_PrivId>().isEmpty())
	{
		QByteArray id = ::Util::random_string(32).toLocal8Bit();
		this->set<Set::Player_PrivId>(id);
	}

	if(get<Set::Player_PublicId>().isEmpty())
	{
		QByteArray id = ::Util::random_string(32).toLocal8Bit();
		this->set<Set::Player_PublicId>(id);
	}
}

