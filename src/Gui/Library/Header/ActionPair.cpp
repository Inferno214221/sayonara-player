/* ActionPair.cpp */

/* Copyright (C) 2011-2020  Lucio Carreras
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

#include "ActionPair.h"

using Library::ActionPair;

struct ActionPair::Private
{
	Lang::Term term;
	Library::SortOrder sortorder;
	bool ascending;

	Private(Lang::Term term, bool ascending, Library::SortOrder sortorder) :
		term(term),
		sortorder(sortorder),
		ascending(ascending)
	{}
};

ActionPair::ActionPair(Lang::Term term, bool ascending, Library::SortOrder sortorder)
{
	m = Pimpl::make<Private>(term, ascending, sortorder);
}

ActionPair::ActionPair(const Library::ActionPair& other)
{
	m = Pimpl::make<Private>(other.m->term, other.m->ascending, other.m->sortorder);
}

Library::ActionPair& ActionPair::operator=(const Library::ActionPair& other)
{
	m->term = other.m->term;
	m->ascending = other.m->ascending;
	m->sortorder = other.m->sortorder;

	return *this;
}

ActionPair::~ActionPair() = default;

QString ActionPair::name() const
{
	QString asc_desc = Lang::get(Lang::Ascending);
	if(!m->ascending)
	{
		asc_desc = Lang::get(Lang::Descending);
	}

	return QString("%1 (%2)").arg(Lang::get(m->term), asc_desc);
}

Library::SortOrder ActionPair::sortorder() const
{
	return m->sortorder;
}
