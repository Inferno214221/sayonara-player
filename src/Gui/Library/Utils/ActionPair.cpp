/* ActionPair.cpp */

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



#include "ActionPair.h"

ActionPair::ActionPair() {}
ActionPair::ActionPair(const QString& name, Library::SortOrder so) :
	name(name),
	so(so)
{}

ActionPair::ActionPair(Lang::Term t1, Lang::Term t2, Library::SortOrder so)
{
	this->name = QString("%1 (%2)").arg(Lang::get(t1), Lang::get(t2));
	this->so = so;
}
