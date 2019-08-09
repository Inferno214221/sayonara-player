/* Genre.cpp */

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



#include "Genre.h"
#include "Utils/Utils.h"
#include <QHash>

struct Genre::Private
{
	QString name;
	GenreID id;

	static GenreID calc_id(const QString& name)
	{
		if(name.trimmed().isEmpty()){
			return 0;
		}

		QByteArray name_data = name.trimmed().toLower().toLocal8Bit();
		return GenreID(qHash(name_data));
	}
};

Genre::Genre()
{
	m = Pimpl::make<Private>();
	m->id = 0;
}

Genre::Genre(const QString& name)
{
	m = Pimpl::make<Private>();
	m->name = name;
	m->id = m->calc_id(name);
}

Genre::~Genre() {}

GenreID Genre::calc_id(const QString& name)
{
	return Genre::Private::calc_id(name);
}

Genre::Genre(const Genre& other)
{
	m = Pimpl::make<Private>();
	m->name = other.name();
	m->id = other.id();
}

Genre& Genre::operator =(const Genre& other)
{
	m->name = (other.name());
	m->id = (other.id());
	return *this;
}


GenreID Genre::id() const
{
	return m->id;
}

QString Genre::name() const
{
	return Util::cvt_str_to_first_upper(m->name);
}

void Genre::set_name(const QString& name)
{
	m->name = name;
	m->id = Genre::Private::calc_id(name);
}

bool Genre::is_equal(const Genre& other) const
{
	return (m->id == other.id());
}

bool Genre::operator ==(const Genre& other) const
{
	return is_equal(other);
}

bool Genre::operator <(const Genre& other) const
{
	return (m->id < other.id());
}

bool Genre::operator >(const Genre& other) const
{
	return (m->id > other.id());
}

