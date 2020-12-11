/* EqualizerSetting.cpp */

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

#include "EqualizerSetting.h"
#include "Utils/Logger/Logger.h"
#include "Utils/globals.h"

#include <QStringList>

struct EqualizerSetting::Private
{
	QString name;
	ValueArray values;
	ValueArray defaultValues;
	int id;

	Private(int id, const QString& name, const ValueArray& values,
	        const ValueArray& defaultValues) :
		name(name),
		values(values),
		defaultValues(defaultValues),
		id(id) {}
};

EqualizerSetting::EqualizerSetting(int id, const QString& name)
{
	ValueArray values;
	values.fill(0);

	m = Pimpl::make<Private>(id, name, values, values);
}

EqualizerSetting::EqualizerSetting(int id, const QString& name,
                                   const EqualizerSetting::ValueArray& values)
{
	ValueArray defaultValues;
	defaultValues.fill(0);
	m = Pimpl::make<Private>(id, name, values, values);
}

EqualizerSetting::EqualizerSetting(int id, const QString& name,
                                   const ValueArray& values,
                                   const ValueArray& defaultValues)
{
	m = Pimpl::make<Private>(id, name, values, defaultValues);
}

EqualizerSetting::EqualizerSetting(const EqualizerSetting& other)
{
	m = Pimpl::make<Private>(other.id(),
	                         other.name(),
	                         other.values(),
	                         other.defaultValues());
}

EqualizerSetting::~EqualizerSetting() = default;

EqualizerSetting& EqualizerSetting::operator=(const EqualizerSetting& s)
{
	m->id = s.id();
	m->name = s.name();
	m->values = s.values();
	m->defaultValues = s.defaultValues();

	return *this;
}

QString EqualizerSetting::name() const
{
	return m->name;
}

void EqualizerSetting::setName(const QString& name)
{
	m->name = name;
}

void EqualizerSetting::setValue(int idx, int val)
{
	if(!Util::between(idx, m->values))
	{
		return;
	}

	m->values[idx] = val;
}

int EqualizerSetting::value(int idx) const
{
	if(!Util::between(idx, m->values))
	{
		return 0;
	}

	return m->values[idx];
}

const EqualizerSetting::ValueArray& EqualizerSetting::values() const
{
	return m->values;
}

void EqualizerSetting::setValues(const EqualizerSetting::ValueArray& values)
{
	m->values = values;
}

const EqualizerSetting::ValueArray& EqualizerSetting::defaultValues() const
{
	return m->defaultValues;
}

void
EqualizerSetting::setDefaultValues(const EqualizerSetting::ValueArray& values)
{
	m->defaultValues = values;
}

EqualizerSetting::ValueArray::const_iterator EqualizerSetting::begin() const
{
	return m->values.begin();
}

EqualizerSetting::ValueArray::const_iterator EqualizerSetting::end() const
{
	return m->values.end();
}

int EqualizerSetting::id() const
{
	return m->id;
}

void EqualizerSetting::setId(int id)
{
	m->id = id;
}

bool EqualizerSetting::isDefault() const
{
	return std::equal(m->values.begin(),
	                  m->values.end(),
	                  m->defaultValues.begin(),
	                  m->defaultValues.end());
}

