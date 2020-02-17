/* Genre.cpp */

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

	Private()=default;

	Private(const Private& other) :
		CASSIGN(name),
		CASSIGN(id)
	{}

	Private(Private&& other) noexcept :
		CMOVE(name),
		CMOVE(id)
	{}

	Private& operator=(const Private& other)
	{
		ASSIGN(name);
		ASSIGN(id);

		return *this;
	}

	Private& operator=(Private&& other) noexcept
	{
		MOVE(name);
		MOVE(id);

		return *this;
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
	m->name = Util::stringToFirstUpper(name);
	m->id = m->calc_id(name);
}

Genre::~Genre() = default;

Genre::Genre(const Genre& other)
{
	m = Pimpl::make<Private>(*(other.m));
}

Genre::Genre(Genre&& other) noexcept
{
	m = Pimpl::make<Private>( std::move(*(other.m)) );
}

Genre& Genre::operator=(const Genre& other)
{
	*m = *(other.m);
	return *this;
}

Genre& Genre::operator=(Genre&& other) noexcept
{
	*m = std::move(*(other.m));
	return *this;
}


GenreID Genre::calc_id(const QString& name)
{
	return Genre::Private::calc_id(name);
}


GenreID Genre::id() const
{
	return m->id;
}

QString Genre::name() const
{
	return m->name;
}

void Genre::setName(const QString& name)
{
	m->name = Util::stringToFirstUpper(name);
	m->id = Genre::Private::calc_id(name);
}

bool Genre::isEqual(const Genre& other) const
{
	return (m->id == other.id());
}

bool Genre::operator ==(const Genre& other) const
{
	return isEqual(other);
}

bool Genre::operator <(const Genre& other) const
{
	return (m->id < other.id());
}

bool Genre::operator >(const Genre& other) const
{
	return (m->id > other.id());
}

