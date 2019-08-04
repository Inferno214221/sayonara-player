/* ExtensionSet.cpp */

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


#include "ExtensionSet.h"
#include "Utils/Set.h"
#include <QString>

using Gui::ExtensionSet;

struct ExtensionSet::Private
{
	Util::Set<QString> enabled_extensions;
	Util::Set<QString> disabled_extensions;
};

ExtensionSet::ExtensionSet()
{
	m = Pimpl::make<Private>();
}

ExtensionSet::~ExtensionSet() {}

ExtensionSet::ExtensionSet(const ExtensionSet& other)
{
	m = Pimpl::make<Private>();
	*m = *(other.m);
}

ExtensionSet& ExtensionSet::operator=(const ExtensionSet& other)
{
	*m = *(other.m);
	return *this;
}

void ExtensionSet::add_extension(const QString& ext, bool enabled)
{
	if(enabled){
		m->enabled_extensions.insert(ext.toLower());
	}

	else {
		m->disabled_extensions.insert(ext.toLower());
	}
}

void ExtensionSet::remove_extension(const QString& ext)
{
	m->enabled_extensions.remove(ext.toLower());
	m->disabled_extensions.remove(ext.toLower());
}

void ExtensionSet::clear()
{
	m->enabled_extensions.clear();
	m->disabled_extensions.clear();
}

bool ExtensionSet::contains_extension(const QString& ext)
{
	return (m->enabled_extensions.contains(ext.toLower())) ||
			(m->disabled_extensions.contains(ext.toLower()));
}

ExtensionSet& ExtensionSet::operator<<(const QString& ext)
{
	add_extension(ext, true);
	return *this;
}

void ExtensionSet::set_enabled(const QString& ext, bool b)
{
	if(b){
		enable(ext);
	}

	else {
		disable(ext);
	}
}

void ExtensionSet::disable(const QString& ext)
{
	m->disabled_extensions.insert(ext.toLower());
	m->enabled_extensions.remove(ext.toLower());
}

void ExtensionSet::enable(const QString& ext)
{
	m->disabled_extensions.remove(ext.toLower());
	m->enabled_extensions.insert(ext.toLower());
}

bool ExtensionSet::has_enabled_extensions() const
{
	return (m->enabled_extensions.count() > 0);
}

bool ExtensionSet::has_disabled_extensions() const
{
	return (m->disabled_extensions.count() > 0);
}

bool ExtensionSet::is_enabled(const QString& ext) const
{
	return (m->enabled_extensions.contains(ext.toLower()));
}

QStringList ExtensionSet::enabled_extensions() const
{
	QStringList lst(m->enabled_extensions.toList());
	std::sort(lst.begin(), lst.end());
	return lst;
}

QStringList ExtensionSet::disabled_extensions() const
{
	QStringList lst(m->disabled_extensions.toList());
	std::sort(lst.begin(), lst.end());
	return lst;
}

QStringList ExtensionSet::extensions() const
{
	QStringList lst;

	lst << enabled_extensions();
	lst << disabled_extensions();

	std::sort(lst.begin(), lst.end());
	return lst;
}
