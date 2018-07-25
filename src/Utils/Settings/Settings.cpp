/* Settings.cpp */

/* Copyright (C) 2011-2017 Lucio Carreras
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
#include "Utils/typedefs.h"

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

	bool has_empty = std::any_of(m->settings.begin(), m->settings.end(), [](AbstrSetting* s){
		return (s==nullptr);
	});

	m->initialized = (!has_empty);

	return m->initialized;
}


#include "Utils/Crypt.h"
#include "Utils/Utils.h"

void Settings::apply_fixes()
{
	/** Create priv keys **/
	QByteArray priv_key = this->get<Set::Player_PrivId>();
	if(priv_key.isEmpty())
	{
		StringPair lfm_pw = this->get<Set::LFM_Login>();
		QString proxy_pw = this->get<Set::Proxy_Password>();

		priv_key = ::Util::random_string(32).toLocal8Bit();
		this->set<Set::Player_PrivId>(priv_key);

		QByteArray pub_key = ::Util::random_string(32).toLocal8Bit();
		this->set<Set::Player_PublicId>(pub_key);

		lfm_pw.second = Util::Crypt::encrypt(lfm_pw.second);
		proxy_pw = Util::Crypt::encrypt(proxy_pw);

		this->set<Set::LFM_Login>(lfm_pw);
		this->set<Set::Proxy_Password>(proxy_pw);
	}
}
