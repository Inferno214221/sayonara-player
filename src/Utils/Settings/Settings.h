/* Settings.h */

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
#pragma once
#ifndef SAYONARA_SETTINGS_H_
#define SAYONARA_SETTINGS_H_

#include "Utils/Settings/SettingKey.h"
#include "Utils/Settings/Setting.h"
#include "Utils/Settings/SettingNotifier.h"
#include "Utils/Singleton.h"

#include <array>
#include <QList>

#define GetSetting(x) Settings::instance()->get<x>()
#define SetSetting(x, y) Settings::instance()->set<x>(y)
#define ListenSetting(x, y) Set::listen<x>(this, &y)
#define ListenSettingNoCall(x, y) Set::listen<x>(this, &y, false)

using SettingArray=std::array<AbstrSetting*, static_cast<unsigned int>(SettingKey::Num_Setting_Keys)>;

/**
 * @brief The Settings class
 * @ingroup Settings
 */
class Settings
{
	SINGLETON(Settings)
	PIMPL(Settings)

	public:
		AbstrSetting* setting(SettingKey keyIndex) const;

		/* get all settings (used by database) */
		const SettingArray& settings();

		/* before you want to access a setting you have to register it */
		void register_setting(AbstrSetting* s);

		/* checks if all settings are registered */
		bool check_settings();

		/* get a setting, defined by a unique, REGISTERED key */
		template<typename KeyClass>
		const typename KeyClass::Data& get() const
		{
			using SettingPtr=Setting<KeyClass>*;
			SettingPtr s = static_cast<SettingPtr>( setting(KeyClass::key) );
			return s->value();
		}

		/* set a setting, define by a unique, REGISTERED key */
		template<typename KeyClass>
		void set(const typename KeyClass::Data& val)
		{
			using SettingPtr=Setting<KeyClass>*;
			SettingPtr s = static_cast<SettingPtr>( setting(KeyClass::key) );

			if( s->assign_value(val))
			{
				SettingNotifier< KeyClass >* sn = SettingNotifier< KeyClass >::instance();
				sn->val_changed();
			}
		}

		/* get a setting, defined by a unique, REGISTERED key */
		template<typename KeyClass>
		void shout() const
		{
			SettingNotifier<KeyClass >* sn = SettingNotifier< KeyClass >::instance();
			sn->val_changed();
		}

		void apply_fixes();

		QList<SettingKey> undeployed_keys() const;

};


#endif // SAYONARA_SETTINGS_H_
