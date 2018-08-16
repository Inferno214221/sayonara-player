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

#include <QList>
#include <QObject>

#include "Utils/Singleton.h"
#include "Utils/Settings/SayonaraClass.h"
#include "Utils/Pimpl.h"

class ShortcutWidget;
class QString;
class QStringList;
class Shortcut;
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

signals:
	void sig_shortcut_changed(const QString& identifier);

public:

	/**
	 * @brief get a shortcut by its unique identifier
	 * @param identifier the identifier which is used in database
	 * @return a shortcut instance
	 */
	Shortcut get_shortcut(const QString& identifier) const;

	/**
	 * @brief set the shortcut by its unique identifier
	 * @param identifier  the identifier which is used in database
	 * @param shortcut a shortcut instance
	 */
	void set_shortcut(const QString& identifier, const QStringList& shortcut);


	/**
	 * @brief add a new shortcut instance to the handler. This is usually done
	 * by the widget the shortcut is attached to. So you can use the same shortcut
	 * on multiple widgets
	 * @param shortcut a shortcut instance
	 * @return an invalid shortcut, if source shortcut is invalid, too\n
	 * if the shortcut already exists, the instance already known is returned\n
	 * if the shortcut does not exist yet, the same shortcut as the input is returned
	 */
	Shortcut add(ShortcutWidget* parent, const QString& identifier, const QString& name, const QString& default_shortcut);


	/**
	 * @brief get all shortcuts
	 * @return
	 */
	QStringList	get_shortcuts() const;

	void set_parent_deleted(ShortcutWidget* parent);
};

#endif // SHORTCUTHANDLER_H
