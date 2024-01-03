/* RawShortcutMap.cpp
 *
 * Copyright (C) 2011-2024 Michael Lugmair
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

#include "RawShortcutMap.h"
#include <QStringList>

QString RawShortcutMap::toString() const
{
	QStringList entries;
	const QList<QString> keys = this->keys();

	for(const QString& key : keys)
	{
		QString shortcut_name = key;
		QStringList shortcuts = this->value(key);

		entries << shortcut_name + ":" + shortcuts.join(", ");
	}

	return entries.join(";-;");
}

RawShortcutMap RawShortcutMap::fromString(const QString& setting)
{
	RawShortcutMap rsc;

	QStringList entries = setting.split(";-;");
	for(const QString& entry : entries)
	{
		QStringList sc_pair = entry.split(":");
		if(sc_pair.size() > 0)
		{
			QString key = sc_pair[0];
			QString shortcut;
			if(sc_pair.size() > 1){
				shortcut = sc_pair[1];
			}

			rsc.insert(key, shortcut.split(", "));
		}

	}

	return rsc;
}
