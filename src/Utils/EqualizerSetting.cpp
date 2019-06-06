/* EqualizerSetting.cpp */

/* Copyright (C) 2011-2019  Lucio Carreras
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

static const int EqualizerSettingSize=10;

struct EqualizerSetting::Private
{
	QString			name;
	ValueArray		values;

	Private(const QString& name) :
		name(name)
	{
		values.fill(0);
	}

	Private(const QString& name, const ValueArray& values) :
		name(name),
		values(values)
	{}
};

EqualizerSetting::EqualizerSetting(const QString& name)
{
	m = Pimpl::make<Private>(name);
}

EqualizerSetting::EqualizerSetting(const QString& name, const ValueArray& values) :
	EqualizerSetting(name)
{
	m->values = values;
}

EqualizerSetting::EqualizerSetting(const EqualizerSetting& other)
{
	m = Pimpl::make<Private>(other.name(), other.values());
}

EqualizerSetting::~EqualizerSetting() {}

EqualizerSetting& EqualizerSetting::operator=(const EqualizerSetting& s)
{
	m->name = s.name();
	m->values = s.values();
	return *this;
}


bool EqualizerSetting::loadFromString(const QString& str)
{
	QStringList lst = str.split(':');
	m->name = lst.takeFirst();

	if(lst.size() < EqualizerSettingSize)
	{
		sp_log(Log::Warning, "EQ_Setting") << "EQ Setting " << str << " has too few parameters " << lst.size();
		return false;
	}

	for(int i=0; i<EqualizerSettingSize; i++)
	{
		this->set_value(i, lst[i].toInt());
	}

	return true;
}


QString EqualizerSetting::toString() const
{
	QString str(m->name);

	for(int val : m->values)
	{
		str += QString(":%1").arg(val);
	}

	return str;
}


bool EqualizerSetting::operator==(const EqualizerSetting& s) const
{
	QString str = toString();
	QString other = s.toString();
	return ( str.compare(other, Qt::CaseInsensitive) == 0 );
}


QList<EqualizerSetting> EqualizerSetting::get_defaults()
{
	QList<EqualizerSetting> defaults;

	defaults << EqualizerSetting(QString(),		ValueArray{ 0,  0,  0,  0,  0,  0,  0,  0,  0,  0});
	defaults << EqualizerSetting("Flat",		ValueArray{ 0,  0,  0,  0,  0,  0,  0,  0,  0,  0});
	defaults << EqualizerSetting("Rock",		ValueArray{ 2,  4,  8,  3,  1,  3,  7, 10, 14, 14});
	defaults << EqualizerSetting("Light Rock",	ValueArray{ 1,  1,  2,  1, -2, -3, -1,  3,  5,  8});
	defaults << EqualizerSetting("Treble",		ValueArray{ 0,  0, -3, -5, -3,  2,  8, 15, 17, 13});
	defaults << EqualizerSetting("Bass",		ValueArray{13, 17, 15,  8,  2, -3, -5, -3,  0,  0});
	defaults << EqualizerSetting("Mid",			ValueArray{ 0,  0,  5,  9, 15, 15, 12,  7,  2,  0});

	return defaults;
}

EqualizerSetting::ValueArray EqualizerSetting::get_default_values(const QString& name)
{
	ValueArray ret; ret.fill(0);

	QList<EqualizerSetting> defaults = EqualizerSetting::get_defaults();

	for(const EqualizerSetting& def : defaults)
	{
		if(def.name().compare(name, Qt::CaseInsensitive) == 0){
			ret = def.values();
		}
	}

	return ret;
}


bool EqualizerSetting::is_default_name(const QString& name)
{
	QList<EqualizerSetting> defaults = EqualizerSetting::get_defaults();

	for(const EqualizerSetting& def : defaults)
	{
		if(def.name().compare(name, Qt::CaseInsensitive) == 0){
			return true;
		}
	}

	return false;
}


QString EqualizerSetting::name() const
{
	return m->name;
}

void EqualizerSetting::set_name(const QString& name)
{
	m->name = name;
}


void EqualizerSetting::set_value(int idx, int val)
{
	if(!Util::between(idx, m->values)){
		return;
	}

	m->values[idx] = val;
}

void EqualizerSetting::set_values(const EqualizerSetting::ValueArray& values)
{
	m->values = values;
}


EqualizerSetting::ValueArray EqualizerSetting::values() const
{
	return m->values;
}

int EqualizerSetting::value(int idx) const
{
	if(!Util::between(idx, m->values)){
		return 0;
	}

	return m->values[idx];
}

bool EqualizerSetting::is_default_name() const
{
	QList<EqualizerSetting> defaults = EqualizerSetting::get_defaults();
	for(const EqualizerSetting& def : defaults)
	{
		if(def.name().compare(m->name, Qt::CaseInsensitive) == 0){
			return true;
		}
	}

	return false;
}

bool EqualizerSetting::is_default() const
{
	QList<EqualizerSetting> defaults = EqualizerSetting::get_defaults();

	for(const EqualizerSetting& def : defaults)
	{
		if(def.name().compare(m->name, Qt::CaseInsensitive) == 0){
			return( def == *this );
		}
	}

	return true;
}

