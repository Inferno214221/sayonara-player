/* Setting.h */

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

#pragma once
#ifndef SAYONARA_PLAYER_SETTING_H
#define SAYONARA_PLAYER_SETTING_H

#include "Utils/Settings/SettingConverter.h"
#include "Utils/Settings/SettingKey.h"
#include "Utils/Pimpl.h"

class AbstrSetting
{
	PIMPL(AbstrSetting)

	private:
		AbstrSetting();
		AbstrSetting(const AbstrSetting& other);
		AbstrSetting& operator=(const AbstrSetting& other);
		AbstrSetting(AbstrSetting&& other) noexcept;
		AbstrSetting& operator=(AbstrSetting&& other) noexcept;

	protected:
		explicit AbstrSetting(SettingKey key);
		AbstrSetting(SettingKey key, const char* dbKey);

	public:
		virtual ~AbstrSetting();

		[[nodiscard]] SettingKey getKey() const;
		[[nodiscard]] QString dbKey() const;
		[[nodiscard]] bool isDatabaseSetting() const;

		void assignValue(const QString& value);

		virtual bool loadValueFromString(const QString& str) = 0;
		[[nodiscard]] virtual QString valueToString() const = 0;
		virtual void assignDefaultValue() = 0;
};

template<typename KeyClass>
class Setting :
	public AbstrSetting
{
	public:
		Setting() = delete;
		Setting(const Setting&) = delete;

		Setting(const char* databaseKey, const typename KeyClass::Data& value) :
			AbstrSetting(KeyClass::key, databaseKey),
			m_value {value},
			m_defaultValue {value} {}

		explicit Setting(const typename KeyClass::Data& value) :
			AbstrSetting(KeyClass::key),
			m_value {value},
			m_defaultValue {value} {}

		~Setting() override = default;

		void assignDefaultValue() override { m_value = m_defaultValue; }

		[[nodiscard]] QString valueToString() const override { return SettingConverter::toString(m_value); }

		bool loadValueFromString(const QString& str) override { return SettingConverter::fromString(str, m_value); }

		const typename KeyClass::Data& value() const { return m_value; }

		//const typename KeyClass::Data& default_value() const		{			return m_defaultValue;		}

		bool assignValue(const typename KeyClass::Data& val)
		{
			if(m_value == val)
			{
				return false;
			}

			m_value = val;

			return true;
		}

	private:
		typename KeyClass::Data m_value;
		typename KeyClass::Data m_defaultValue;
};

#endif // SAYONARA_PLAYER_SETTING_H
