/* AbstractCoverLookup.cpp */

/* Copyright (C) 2011-2024 Michael Lugmair (Lucio Carreras)
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
	Cover::Location coverLocation;

	Private(const Cover::Location& coverLocation) :
		coverLocation(coverLocation) {}
};

Cover::LookupBase::LookupBase(const Cover::Location& coverLocation, QObject* parent) :
	QObject(parent)
{
	m = Pimpl::make<Private>(coverLocation);
}

Cover::LookupBase::~LookupBase() = default;

Cover::Location Cover::LookupBase::coverLocation() const
{
	return m->coverLocation;
}

void Cover::LookupBase::setCoverLocation(const Cover::Location& coverLocation)
{
	m->coverLocation = coverLocation;
}
