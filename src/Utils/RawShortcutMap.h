/* RawShortcutMap.h */

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

#ifndef RAWSHORTCUTMAP_H
#define RAWSHORTCUTMAP_H

class QString;
class QStringList;

#include <QMap>

//this class has to be inline because we need this in the settings registry

/**
 * @brief The RawShortcutMap struct consisting of a specifier writable into database and a shortcut.
 * This class is used for converting a shortcut map into its database representation
 * @ingroup Shortcuts
 */
struct RawShortcutMap :
		public QMap<QString, QStringList>
{
	QString toString() const;
	static RawShortcutMap fromString(const QString& setting);
};

#endif // RAWSHORTCUTMAP_H
