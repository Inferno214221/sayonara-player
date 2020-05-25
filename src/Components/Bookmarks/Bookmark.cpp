/* Bookmark.cpp */

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

#include "Bookmark.h"
#include <QString>

struct Bookmark::Private
{
	Seconds timestamp;
	QString name;
	bool valid;

	Private() :
		timestamp(0),
		valid(false)
	{}
};

Bookmark::Bookmark(Seconds timestamp)
{
	m = Pimpl::make<Private>();
	m->timestamp = timestamp;
}

Bookmark::Bookmark(Seconds timestamp, const QString& name, bool valid) :
	Bookmark(timestamp)
{
	m->name = name;
	m->valid = valid;
}

Bookmark::~Bookmark() = default;

Bookmark::Bookmark(const Bookmark& other) :
	Bookmark(other.m->timestamp, other.m->name, other.m->valid)
{}

Bookmark& Bookmark::operator=(const Bookmark& other)
{
	m->timestamp = other.m->timestamp;
	m->name = other.m->name;
	m->valid = other.m->valid;

	return *this;
}

Seconds Bookmark::timestamp() const
{
	return m->timestamp;
}

QString Bookmark::name() const
{
	return m->name;
}

bool Bookmark::isValid() const
{
	return m->valid;
}
