/* Settings.h */

/* Copyright (C) 2011-2024 Michael Lugmair (Lucio Carreras)
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
#ifndef SAYONARA_PLAYER_SETTINGS_H
#define SAYONARA_PLAYER_SETTINGS_H

#include "Utils/Settings/SettingKey.h"
#include "Utils/Settings/Setting.h"
#include "Utils/Settings/SettingNotifier.h"
#include "Utils/Singleton.h"

#include <array>
#include <cassert>

#define GetSetting(x) Settings::instance()->get<x>()
#define SetSetting(x, y) Settings::instance()->set<x>(y)
#define ListenSetting(x, y) Set::listen<x>(this, &y)
#define ListenSettingNoCall(x, y) Set::listen<x>(this, &y, false)

using SettingArray = std::array<AbstrSetting*, static_cast<unsigned int>(SettingKey::Num_Setting_Keys)>;

class Settings
{
		SINGLETON(Settings)
	PIMPL(Settings)

	public:
		[[nodiscard]] AbstrSetting* setting(SettingKey keyIndex) const;

		const SettingArray& settings();

		void registerSetting(AbstrSetting* s);

		bool checkSettings();

		template<typename KeyClass>
		const typename KeyClass::Data& get() const
		{
			using SettingPtr = Setting<KeyClass>*;
			auto* s = static_cast<SettingPtr>(setting(KeyClass::key));
			assert(s);
			return s->value();
		}

		template<typename KeyClass>
		void set(const typename KeyClass::Data& val)
		{
			using SettingPtr = Setting<KeyClass>*;
			auto* s = static_cast<SettingPtr>(setting(KeyClass::key));
			assert(s);
			if(s->assignValue(val))
			{
				SettingNotifier<KeyClass>::instance()->valueChanged();
			}
		}

		template<typename KeyClass>
		void shout() const
		{
			auto* settingNotifier = SettingNotifier<KeyClass>::instance();
			settingNotifier->valueChanged();
		}

		void applyFixes();
};

#endif // SAYONARA_PLAYER_SETTINGS_H
