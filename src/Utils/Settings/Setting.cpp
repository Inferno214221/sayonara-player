/* Setting.cpp */

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

#include "Setting.h"
#include "Utils/Logger/Logger.h"

#include <QVariant>

struct AbstrSetting::Private
{
	QString databaseKey;
	SettingKey key {SettingKey::Num_Setting_Keys};
	bool isDatabaseSetting {false};

	Private() = default;

	Private(QString databaseKey, const SettingKey key, const bool isDatabaseSetting) :
		databaseKey {std::move(databaseKey)},
		key {key},
		isDatabaseSetting {isDatabaseSetting} {}

	Private(const Private& other) = default;
	Private(Private&& other) = default;
	Private& operator=(const Private& other) = default;
	Private& operator=(Private&& other) = default;
};

AbstrSetting::AbstrSetting() :
	m {Pimpl::make<Private>()} {}

AbstrSetting::AbstrSetting(SettingKey key) :
	m {Pimpl::make<Private>(QString {}, key, false)} {}

AbstrSetting::AbstrSetting(SettingKey key, const char* databaseKey) :
	m {Pimpl::make<Private>(databaseKey, key, true)} {}

AbstrSetting::AbstrSetting(const AbstrSetting& other) :
	m {Pimpl::make<Private>(*other.m)} {}

AbstrSetting& AbstrSetting::operator=(const AbstrSetting& other)
{
	*m = *(other.m);
	return *this;
}

AbstrSetting::AbstrSetting(AbstrSetting&& other) noexcept :
	m {Pimpl::make<Private>(std::move(*other.m))} {}

AbstrSetting& AbstrSetting::operator=(AbstrSetting&& other) noexcept
{
	*m = std::move(*(other.m));
	return *this;
}

AbstrSetting::~AbstrSetting() = default;

SettingKey AbstrSetting::getKey() const { return m->key; }

QString AbstrSetting::dbKey() const { return m->databaseKey; }

bool AbstrSetting::isDatabaseSetting() const { return m->isDatabaseSetting; }

void AbstrSetting::assignValue(const QString& value)
{
	if(m->isDatabaseSetting)
	{
		const auto success = loadValueFromString(value);
		if(!success)
		{
			spLog(Log::Warning, this) << "Setting " << m->databaseKey << ": Cannot convert. Use default value...";
			assignDefaultValue();
		}
	}
}
