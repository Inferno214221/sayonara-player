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

class Settings;

/**
 * @brief The AbstrSetting class\n
 * Every setting needs a key and a value
 * The SettingKey is only used inside the setting mechanism
 * @ingroup Settings
 */
class AbstrSetting
{
	PIMPL(AbstrSetting)

	private:
		AbstrSetting();
		AbstrSetting(const AbstrSetting& other);
		AbstrSetting& operator=(const AbstrSetting& other);

	protected:
		AbstrSetting(SettingKey key);
		AbstrSetting(SettingKey key, const char* dbKey);


	public:
		virtual ~AbstrSetting();

		SettingKey getKey() const;
		QString dbKey() const;
		bool isDatabaseSetting() const;

		void assignValue(const QString& value);

		/* Pure virtual function for DB load/save */
		virtual bool loadValueFromString(const QString& str)=0;
		virtual QString valueToString() const=0;
		virtual void assignDefaultValue()=0;
};


template<typename KeyClass>
/**
 * @brief The Setting class\n
 * T is the pure value type e.g. QString
 * @ingroup Settings
 */
class Setting : public AbstrSetting
{
	private:
		Setting()=delete;
		Setting(const Setting&)=delete;

		typename KeyClass::Data mValue;
		typename KeyClass::Data mDefaultValue;

	public:

		/* Constructor */
		Setting(const char* db_key, const typename KeyClass::Data& def) :
			AbstrSetting(KeyClass::key, db_key)
		{
			mDefaultValue = def;
			mValue = def;
		}

		Setting(const typename KeyClass::Data& def) :
			AbstrSetting(KeyClass::key)
		{
			mDefaultValue = def;
			mValue = def;
		}

		/* Destructor */
		~Setting() = default;

		void assignDefaultValue() override
		{
			mValue = mDefaultValue;
		}

		QString valueToString() const override
		{
			 return SettingConverter::toString(mValue);
		}

		bool loadValueFromString(const QString& str) override
		{
			return SettingConverter::fromString(str, mValue);
		}

		/* ... */
		const typename KeyClass::Data& value() const
		{
			return mValue;
		}

		/* ... */
		const typename KeyClass::Data& default_value() const
		{
			return mDefaultValue;
		}

		/* ... */
		bool assignValue(const typename KeyClass::Data& val)
		{
			if( mValue == val ){
				return false;
			}

			mValue = val;
			return true;
		}
};

#endif // SAYONARA_PLAYER_SETTING_H
