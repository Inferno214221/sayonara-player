/* Shortcut.cpp */

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

#include "Shortcut.h"
#include "ShortcutHandler.h"
#include "Database/Connector.h"
#include "Database/Shortcuts.h"
#include "GUI/Utils/Widgets/Widget.h"

#include "Utils/RawShortcutMap.h"
#include "Utils/Logger/Logger.h"

#include <QKeySequence>
#include <QWidget>

struct Shortcut::Private
{
	QStringList			default_shortcuts;
	QStringList			shortcuts;
	ShortcutIdentifier	identifier;

	QList<QShortcut*>	qt_shortcuts;

	Private(ShortcutIdentifier id) :
		identifier(id)
	{}
};

Shortcut::Shortcut()
{
	m = Pimpl::make<Private>(ShortcutIdentifier::Invalid);
}

Shortcut::Shortcut(ShortcutIdentifier identifier, const QStringList& default_shortcuts)
{
	m = Pimpl::make<Private>(identifier);

	m->default_shortcuts = default_shortcuts;
	for(QString& str : m->default_shortcuts){
		str.replace(" +", "+");
		str.replace("+ ", "+");
	}

	m->shortcuts = m->default_shortcuts;
}

Shortcut::Shortcut(ShortcutIdentifier identifier, const QString& default_shortcut) :
	Shortcut(identifier, QStringList(default_shortcut)) {}

Shortcut::Shortcut(const Shortcut& other) :
	Shortcut()
{
	m->identifier =			other.m->identifier;
	m->default_shortcuts =	other.m->default_shortcuts;
	m->shortcuts =			other.m->shortcuts;
	m->qt_shortcuts =		other.m->qt_shortcuts;
}

Shortcut::~Shortcut() {}

Shortcut& Shortcut::operator =(const Shortcut& other)
{
	m->identifier =			other.m->identifier;
	m->default_shortcuts =	other.m->default_shortcuts;
	m->shortcuts =			other.m->shortcuts;
	m->qt_shortcuts =		other.m->qt_shortcuts;

	return (*this);
}

QString Shortcut::name() const
{
	return ShortcutHandler::instance()->shortcut_text(m->identifier);
}

QStringList Shortcut::default_shorcut() const
{
	return m->default_shortcuts;
}

QList<QKeySequence> Shortcut::sequences() const
{
	QList<QKeySequence> sequences;
	const QStringList& shortcuts = this->shortcuts();

	for(const QString& str : shortcuts)
	{
		QKeySequence seq = QKeySequence::fromString(str, QKeySequence::NativeText);
		sequences << seq;
	}

	if(sequences.isEmpty()){
		sequences << QKeySequence();
	}

	return sequences;
}

const QStringList& Shortcut::shortcuts() const
{
	return m->shortcuts;
}

ShortcutIdentifier Shortcut::identifier() const
{
	return m->identifier;
}

QString Shortcut::identifier_string() const
{
	return ShortcutHandler::instance()->identifier(m->identifier);
}

Shortcut Shortcut::getInvalid()
{
	return Shortcut();
}

bool Shortcut::is_valid() const
{
	return (m->identifier != ShortcutIdentifier::Invalid);
}

void Shortcut::connect(QWidget* parent, QObject* receiver, const char* slot, Qt::ShortcutContext context)
{
	QList<QShortcut*> shortcuts = init_qt_shortcut(parent, context);
	for(QShortcut* sc : shortcuts)
	{
		parent->connect(sc, SIGNAL(activated()), receiver, slot);
	}
}


QList<QShortcut*> Shortcut::init_qt_shortcut(QWidget* parent, Qt::ShortcutContext context)
{
	QList<QShortcut*> lst;

	const QList<QKeySequence> sequences = this->sequences();
	for(const QKeySequence& sequence : sequences)
	{
		QShortcut* shortcut = new QShortcut(parent);

		shortcut->setContext(context);
		shortcut->setKey(sequence);

		lst << shortcut;
	}

	ShortcutHandler::instance()->qt_shortcuts_added(m->identifier, lst);

	return lst;
}

void Shortcut::add_qt_shortcuts(const QList<QShortcut*>& qt_shortcuts)
{
	m->qt_shortcuts << qt_shortcuts;
}


void Shortcut::change_shortcut(const QStringList& shortcuts)
{
	m->shortcuts.clear();
	for(QString str : shortcuts)
	{
		str.replace(" +", "+");
		str.replace("+ ", "+");

		m->shortcuts << str;

		if(str.contains("Enter"))
		{
			QString re(str);
			re.replace("Enter", "Return");
			m->shortcuts << re;
		}

		if(str.contains("Return"))
		{
			QString re(str);
			re.replace("Return", "Enter");
			m->shortcuts << re;
		}

		m->shortcuts.removeDuplicates();
	}

	foreach(QShortcut* sc, m->qt_shortcuts)
	{
		QList<QKeySequence> sequences = this->sequences();
		for(const QKeySequence& ks : sequences){
			sc->setKey(ks);
		}
	}
}
