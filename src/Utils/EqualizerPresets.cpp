/* EqualizerPresets.cpp */

/* Copyright (C) 2011-2017  Lucio Carreras
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

#include "EqualizerPresets.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Utils.h"
#include "Utils/globals.h"
#include <QStringList>
#include <iostream>

struct EQ_Setting::Private
{
	QString			name;
	QList<int>		values;

	Private(const QString& name) :
		name(name)
	{
		for(int i=0; i<10; i++){
			values << 0;
		}
	}

	Private(const QString& name, const QList<int>& values) :
		name(name),
		values(values)
	{}
};

EQ_Setting::EQ_Setting(const QString& name)
{
	m = Pimpl::make<Private>(name);
}

EQ_Setting::EQ_Setting(const EQ_Setting& other)
{
	m = Pimpl::make<Private>(other.name(), other.values());
}

EQ_Setting::~EQ_Setting() {}

EQ_Setting& EQ_Setting::operator=(const EQ_Setting& s)
{
	m->name = s.name();
	m->values = s.values();
	return *this;
}


EQ_Setting EQ_Setting::fromString(const QString& str)
{
	QStringList list = str.split(':');
	EQ_Setting eq(list.takeFirst());

	if(list.size() < 10)
	{
		sp_log(Log::Warning, "EQ_Setting") << "EQ Setting " << str << " has too few parameters " << list.size();
		return eq;
	}

	for(int i=0; (i<list.size()) && (i<eq.values().size()); i++)
	{
		eq.set_value(i, list[i].toInt());
	}

	return eq;
}


QString EQ_Setting::toString() const
{
	QString str(m->name);

	for(int val : m->values)
	{
		str += QString(":%1").arg(val);
	}

	return str;
}


bool EQ_Setting::operator==(const EQ_Setting& s) const
{
	QString str = toString();
	QString other = s.toString();
	return ( str.compare(other, Qt::CaseInsensitive) == 0 );
}


QList<EQ_Setting> EQ_Setting::get_defaults()
{
	QList<EQ_Setting> defaults;

	defaults << EQ_Setting::fromString(QString(":0:0:0:0:0:0:0:0:0:0"));
	defaults << EQ_Setting::fromString(QString("Flat:0:0:0:0:0:0:0:0:0:0"));
	defaults << EQ_Setting::fromString(QString("Rock:2:4:8:3:1:3:7:10:14:14"));
	defaults << EQ_Setting::fromString(QString("Light Rock:1:1:2:1:-2:-3:-1:3:5:8"));
	defaults << EQ_Setting::fromString(QString("Treble:0:0:-3:-5:-3:2:8:15:17:13"));
	defaults << EQ_Setting::fromString(QString("Bass:13:17:15:8:2:-3:-5:-3:0:0"));
	defaults << EQ_Setting::fromString(QString("Mid:0:0:5:9:15:15:12:7:2:0"));

	return defaults;
}

QList<int> EQ_Setting::get_default_values(const QString& name)
{
	QList<EQ_Setting> defaults = EQ_Setting::get_defaults();

	for(const EQ_Setting& def : defaults)
	{
		if(def.name().compare(name, Qt::CaseInsensitive) == 0){
			return def.values();
		}
	}

	return QList<int>();
}


bool EQ_Setting::is_default_name(const QString& name)
{
	QList<EQ_Setting> defaults = EQ_Setting::get_defaults();

	for(const EQ_Setting& def : defaults)
	{
		if(def.name().compare(name, Qt::CaseInsensitive) == 0){
			return true;
		}
	}

	return false;
}


QString EQ_Setting::name() const
{
	return m->name;
}

void EQ_Setting::set_name(const QString& name)
{
	m->name = name;
}


void EQ_Setting::set_value(int idx, int val)
{
	if(!between(idx, m->values)){
		return;
	}

	m->values[idx] = val;
}

void EQ_Setting::set_values(const QList<int> values)
{
	m->values = values;

	if(m->values.size() != 10){
		sp_log(Log::Warning, this) << "EQ Preset " << m->name << " should have 10 values. But it has " << m->values.size();
	}

	while(m->values.size() < 10){
		m->values << 0;
	}

	while(m->values.size() > 10)
	{
		m->values.pop_back();
	}
}


QList<int> EQ_Setting::values() const
{
	return m->values;
}

int EQ_Setting::value(int idx) const
{
	if(!between(idx, m->values)){
		return 0;
	}

	return m->values[idx];
}

bool EQ_Setting::is_default_name() const
{
	QList<EQ_Setting> defaults = EQ_Setting::get_defaults();
	for(const EQ_Setting& def : defaults)
	{
		if(def.name().compare(m->name, Qt::CaseInsensitive) == 0){
			return true;
		}
	}

	return false;
}

bool EQ_Setting::is_default() const
{
	QList<EQ_Setting> defaults = EQ_Setting::get_defaults();

	for(const EQ_Setting& def : defaults)
	{
		if(def.name().compare(m->name, Qt::CaseInsensitive) == 0){
			return( def == *this );
		}
	}

	return true;
}

