/* ShortcutHandler.h */

/* Copyright (C) 2011-2017  Lucio Carreras
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

#ifndef SHORTCUTHANDLER_H
#define SHORTCUTHANDLER_H


#include "ShortcutIdentifier.h"
#include "Utils/Singleton.h"
#include "Utils/Settings/SayonaraClass.h"
#include "Utils/Pimpl.h"

#include <QList>
#include <QShortcut>
#include <QObject>

class ShortcutWidget;
class QString;
class QStringList;
class Shortcut;

#define ShortcutPrivate private

/**
 * @brief A singleton class for retrieving shortcuts
 * @ingroup Shortcuts
 */
class ShortcutHandler :
	public QObject,
	public SayonaraClass
{
	Q_OBJECT
	SINGLETON(ShortcutHandler)
	PIMPL(ShortcutHandler)


friend class Shortcut;
ShortcutPrivate:
	void qt_shortcuts_added(ShortcutIdentifier identifier, const QList<QShortcut*>& shortcuts);

signals:
	void sig_shortcut_changed(ShortcutIdentifier identifier);

public:
	/**
	 * @brief get a shortcut by its unique identifier
	 * @param identifier the identifier which is used in database
	 * @return a shortcut instance
	 */
	Shortcut shortcut(ShortcutIdentifier identifier) const;

	/**
	 * @brief set the shortcut by its unique identifier
	 * @param identifier  the identifier which is used in database
	 * @param shortcut a shortcut instance
	 */
	void set_shortcut(ShortcutIdentifier identifier, const QStringList& shortcut);

	/**
	 * @brief get all shortcuts
	 * @return
	 */
	QList<ShortcutIdentifier> shortcuts_ids() const;

	QString identifier(ShortcutIdentifier id) const;
	QString shortcut_text(ShortcutIdentifier id) const;
};

#endif // SHORTCUTHANDLER_H
