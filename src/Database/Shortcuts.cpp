/* Shortcuts.cpp */

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

#include "Shortcuts.h"
#include "Query.h"
#include "Utils/RawShortcutMap.h"

#include <QSqlQuery>

using DB::Shortcuts;

DB::Shortcuts::Shortcuts(const QString& connectionName, const DbId databaseId) :
	Module(connectionName, databaseId) {}

DB::Shortcuts::~Shortcuts() {}

RawShortcutMap DB::Shortcuts::getAllShortcuts()
{
	RawShortcutMap rsm;

	auto q = runQuery(
		"SELECT identifier, shortcut from Shortcuts;",
		"Cannot fetch all shortcuts");

	if(hasError(q))
	{
		return rsm;
	}

	while(q.next())
	{
		const auto identifier = q.value(0).toString();
		const auto shortcut = q.value(1).toString();

		auto shortcuts = rsm[identifier];
		shortcuts << shortcut;

		rsm[identifier] = shortcuts;
	}

	return rsm;
}

bool DB::Shortcuts::setShortcuts(const QString& identifier, const QStringList& shortcuts)
{
	db().transaction();

	clearShortcuts(identifier);

	for(const auto& shortcut: shortcuts)
	{
		auto q = insert("Shortcuts",
		                {
			                {"identifier", identifier},
			                {"shortcut",   shortcut}
		                }, "Cannot insert shortcut " + identifier);

		if(hasError(q))
		{
			db().rollback();
			return false;
		}
	}

	return db().commit();
}

bool DB::Shortcuts::clearShortcuts(const QString& identifier)
{
	const auto q = runQuery(
		"DELETE FROM Shortcuts WHERE identifier=:identifier;",
		{":identifier", identifier},
		"Cannot clear Shortcuts");

	return !hasError(q);
}
