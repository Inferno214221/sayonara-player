/* AbstractCoverLookup.cpp */

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

#include "AbstractCoverLookup.h"
#include "CoverLocation.h"

struct Cover::LookupBase::Private
{
	Cover::Location cl;

	Private(const Cover::Location& cl) : cl(cl) {}
};

Cover::LookupBase::LookupBase(const Cover::Location& cl, QObject *parent) :
    QObject(parent)
{
	m = Pimpl::make<Private>(cl);
}

Cover::LookupBase::~LookupBase() = default;

Cover::Location Cover::LookupBase::cover_location() const
{
	return m->cl;
}

void Cover::LookupBase::set_cover_location(const Cover::Location& cl)
{
	m->cl = cl;
}
