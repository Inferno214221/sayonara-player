/* ExtensionSet.cpp */

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


#include "ExtensionSet.h"
#include "Utils/Set.h"
#include <QString>

using Gui::ExtensionSet;

struct ExtensionSet::Private
{
	Util::Set<QString> enabledExtensions;
	Util::Set<QString> disabledExtensions;
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

void ExtensionSet::addExtension(const QString& ext, bool enabled)
{
	if(enabled)
	{
		m->enabledExtensions.insert(ext.toLower());
	}

	else
	{
		m->disabledExtensions.insert(ext.toLower());
	}
}

void ExtensionSet::removeExtension(const QString& ext)
{
	m->enabledExtensions.remove(ext.toLower());
	m->disabledExtensions.remove(ext.toLower());
}

void ExtensionSet::clear()
{
	m->enabledExtensions.clear();
	m->disabledExtensions.clear();
}

bool ExtensionSet::containsExtension(const QString& ext)
{
	return (m->enabledExtensions.contains(ext.toLower())) ||
	       (m->disabledExtensions.contains(ext.toLower()));
}

ExtensionSet& ExtensionSet::operator<<(const QString& ext)
{
	addExtension(ext, true);
	return *this;
}

void ExtensionSet::setEnabled(const QString& ext, bool b)
{
	if(b)
	{
		enable(ext);
	}

	else
	{
		disable(ext);
	}
}

void ExtensionSet::disable(const QString& ext)
{
	m->disabledExtensions.insert(ext.toLower());
	m->enabledExtensions.remove(ext.toLower());
}

void ExtensionSet::enable(const QString& ext)
{
	m->disabledExtensions.remove(ext.toLower());
	m->enabledExtensions.insert(ext.toLower());
}

bool ExtensionSet::hasEnabledExtensions() const
{
	return (m->enabledExtensions.count() > 0);
}

bool ExtensionSet::hasDisabledExtensions() const
{
	return (m->disabledExtensions.count() > 0);
}

bool ExtensionSet::isEnabled(const QString& ext) const
{
	return (m->enabledExtensions.contains(ext.toLower()));
}

QStringList ExtensionSet::enabledExtensions() const
{
	QStringList lst(m->enabledExtensions.toList());
	std::sort(lst.begin(), lst.end());
	return lst;
}

QStringList ExtensionSet::disabledExtensions() const
{
	QStringList lst(m->disabledExtensions.toList());
	std::sort(lst.begin(), lst.end());
	return lst;
}

QStringList ExtensionSet::extensions() const
{
	QStringList lst;

	lst << enabledExtensions();
	lst << disabledExtensions();

	std::sort(lst.begin(), lst.end());
	return lst;
}
