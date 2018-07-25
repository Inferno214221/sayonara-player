/* Settings.h */

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
#pragma once
#ifndef SAYONARA_SETTINGS_H_
#define SAYONARA_SETTINGS_H_


#include "Utils/Settings/Setting.h"
#include "Utils/Settings/SettingNotifier.h"
#include "Utils/Singleton.h"

#include <array>

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
		template< typename T>
		const typename T::Data& get() const
		{
			using DataType = typename T::Data;
			constexpr SettingKey keyIndex = T::key;

			using SettingPtr=Setting<DataType, keyIndex>*;

			SettingPtr s = static_cast<SettingPtr>( setting(keyIndex) );
			return s->value();
		}

		/* set a setting, define by a unique, REGISTERED key */
		template< typename T>
		void set(const typename T::Data& val)
		{
			using DataType = typename T::Data;
			constexpr SettingKey keyIndex = T::key;

			using SettingPtr=Setting<DataType, keyIndex>*;
			SettingPtr s = static_cast<SettingPtr>( setting(keyIndex) );

			if( s->assign_value(val))
			{
				using KeyClass=SettingIdentifier<DataType, keyIndex>;

				SettingNotifier< KeyClass >* sn = SettingNotifier< KeyClass >::instance();
				sn->val_changed();
			}
		}

		/* get a setting, defined by a unique, REGISTERED key */
		template< typename T>
		void shout() const
		{
			using DataType = typename T::Data;
			constexpr SettingKey keyIndex = T::key;
			using KeyClass=SettingIdentifier<DataType, keyIndex>;

			SettingNotifier< KeyClass >* sn = SettingNotifier< KeyClass >::instance();
			sn->val_changed();
		}

		void apply_fixes();

};


#endif // SAYONARA_SETTINGS_H_
