/* Shortcut.h */

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

#ifndef SHORTCUT_H
#define SHORTCUT_H

#include "ShortcutIdentifier.h"
#include "Utils/Pimpl.h"

#include <QShortcut>

#define ShortcutHandlerPrivate private

class QKeySequence;
class QWidget;
/**
 * @brief A single shortcut managed by ShortcutHandler.
 * This class holds information about the default shortcuts,
 * the user defined shortcuts, a name attached to each shortcut
 * an identifier which is written into the database and a list
 * of the corresponding shortcuts in the Qt format
 * @ingroup Shortcuts
 */
class Shortcut
{

	private:
		PIMPL(Shortcut)

		Shortcut();

		/**
		 * @brief Converts the sequences from get_sequences() to a list of qt specific
		 * shortcuts and writes them into the _qt_shortcuts field
		 * @param parent the widget the shortcut is mapped to
		 * @return a list of shortcuts in the Qt format
		 */
		QList<QShortcut*> init_qt_shortcut(QWidget* parent, Qt::ShortcutContext context);


	friend class ShortcutHandler;
	ShortcutHandlerPrivate:
		void set_qt_shortcuts(const QList<QShortcut*>& qshortcuts);
		void remove_qt_shortcut(QShortcut* qshortcut);
		QList<QShortcut*> qt_shortcuts() const;


	public:
		/**
		 * @brief Shortcut
		 * @param identifier an unique identifier used to write the shortcut into the database
		 * @param name the name displayed in the Shortcut configuration dialog
		 * @param default_shortcut one default shortcut
		 */
		Shortcut(ShortcutIdentifier identifier, const QString& default_shortcut);

		/**
		 * @brief Shortcut
		 * @param identifier an unique identifier used to write the shortcut into the database
		 * @param name the name displayed in the Shortcut configuration dialog
		 * @param default_shortcuts a list of default shortcuts
		 */
		Shortcut(ShortcutIdentifier identifier, const QStringList& default_shortcuts);

		/**
		 * @brief Copy constructor
		 * @param other
		 */
		Shortcut(const Shortcut& other);

		Shortcut& operator=(const Shortcut& other);

		~Shortcut();

		/**
		 * @brief get a raw and invalid shortcut. This function is used instead of the default constructor
		 * @return an uninitialized shortcut
		 */
		static Shortcut getInvalid();

		/**
		 * @brief
		 * @param shortcuts map new user-readable key sequences to this shortcut
		 */
		void					change_shortcut(const QStringList& shortcuts);

		/**
		 * @brief get the human-readable name of the shortcut
		 * @return
		 */
		QString					name() const;

		/**
		 * @brief get a human-readable list of mapped default shortcuts
		 * @return
		 */
		QStringList				default_shorcut() const;

		/**
		 * @brief get a list key squences mapped to this shortcut
		 * @return
		 */
		QList<QKeySequence>		sequences() const;
		QKeySequence			sequence() const;

		/**
		 * @brief get a human-readable list of mapped shortcuts
		 * @return
		 */
		const QStringList&		shortcuts() const;

		/**
		 * @brief get the unique identifier
		 * @return
		 */
		ShortcutIdentifier		identifier() const;
		QString					db_key() const;

		/**
		 * @brief Check if the shortcut is valid or if it was retrieved via getInvalid()
		 * @return
		 */
		bool					is_valid() const;

		template<typename T>
		/**
		 * @brief create a qt shortcut for a widget
		 * @param parent the widget the shortcut is attached to
		 * @param func a lambda function which will be triggered when shortcut is pressed
		 */
		void connect(QWidget* parent, T func, Qt::ShortcutContext context=Qt::WindowShortcut)
		{
			QList<QShortcut*> shortcuts = init_qt_shortcut(parent, context);
			for(QShortcut* sc : shortcuts)
			{
				parent->connect(sc, &QShortcut::activated, func);
			}
		}


		/**
		 * @brief create a qt shortcut for a widget
		 * @param parent the widget the shortcut is attached to
		 * @param the receiver object of the shortcut
		 * @param the slot which is triggered when pressing that shortcut
		 */
		void connect(QWidget* parent, QObject* receiver, const char* slot, Qt::ShortcutContext context=Qt::WindowShortcut);
};

#endif // SHORTCUT_H
