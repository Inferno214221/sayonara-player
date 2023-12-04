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
	bool valid {false};

	Private(const Seconds timestamp, const QString& name, const bool valid) :
		timestamp {timestamp},
		name {name},
		valid {valid} {}

	Private(const Private& other) = default;
	Private(Private&& other) = default;
	Private& operator=(const Private& other) = default;
	Private& operator=(Private&& other) = default;
};

Bookmark::Bookmark(const Seconds timestamp, const QString& name, bool valid) :
	m {Pimpl::make<Private>(timestamp, name, valid)} {}

Bookmark::Bookmark(const Seconds timestamp) :
	m {Pimpl::make<Private>(timestamp, QString {}, false)} {}

Bookmark::~Bookmark() = default;

Bookmark::Bookmark(const Bookmark& other) :
	Bookmark(other.m->timestamp, other.m->name, other.m->valid) {}

Bookmark::Bookmark(Bookmark&& other) noexcept :
	m {std::move(other.m)} {}

Bookmark& Bookmark::operator=(const Bookmark& other)
{
	*m = *(other.m);
	return *this;
}

Bookmark& Bookmark::operator=(Bookmark&& other) noexcept
{
	m = std::move(other.m);
	return *this;
}

Seconds Bookmark::timestamp() const { return m->timestamp; }

QString Bookmark::name() const { return m->name; }

bool Bookmark::isValid() const { return m->valid; }
