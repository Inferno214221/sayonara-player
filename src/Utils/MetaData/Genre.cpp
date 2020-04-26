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

	static GenreID calcId(const QString& name)
	{
		if(name.trimmed().isEmpty()){
			return 0;
		}

		QByteArray nameData = name.trimmed().toLocal8Bit();
		return GenreID(qHash(nameData));
	}

	Private()=default;

	Private(const Private& other)=default;
	Private(Private&& other) noexcept=default;
	Private& operator=(const Private& other)=default;
	Private& operator=(Private&& other) noexcept=default;
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
	m->id = m->calcId(name);
}

Genre::~Genre() = default;

Genre::Genre(const Genre& other)
{
	m = Pimpl::make<Private>(*(other.m));
}

Genre::Genre(Genre&& other) noexcept
{
	m = Pimpl::make<Private>(std::move(*(other.m)));
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

GenreID Genre::calculateId(const QString& name)
{
	return Genre::Private::calcId(name);
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
	m->name = name.trimmed();
	m->id = Genre::Private::calcId(m->name);
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

