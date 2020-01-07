/* Shortcut.cpp */

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

#include "Shortcut.h"
#include "ShortcutHandler.h"
#include "Gui/Utils/Widgets/Widget.h"

#include "Utils/RawShortcutMap.h"
#include "Utils/Logger/Logger.h"

#include <QKeySequence>
#include <QWidget>

struct Shortcut::Private
{
	QList<QShortcut*>	qt_shortcuts;
	QStringList			default_shortcuts;
	QStringList			shortcuts;
	ShortcutIdentifier	identifier;

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
		sequences << QKeySequence::fromString(str, QKeySequence::NativeText);
	}

	if(sequences.isEmpty()){
		sequences << QKeySequence();
	}

	return sequences;
}

QKeySequence Shortcut::sequence() const
{
	const QList<QKeySequence> sequences = this->sequences();
	if(sequences.isEmpty()){
		return QKeySequence();
	}

	return sequences.first();
}

const QStringList& Shortcut::shortcuts() const
{
	return m->shortcuts;
}

ShortcutIdentifier Shortcut::identifier() const
{
	return m->identifier;
}

QString Shortcut::db_key() const
{
	return ShortcutHandler::instance()->db_key(m->identifier);
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
	QList<QShortcut*> qshortcuts;

	const QList<QKeySequence> sequences = this->sequences();//merge_key_sequences(this->sequences());

	for(QKeySequence s : sequences)
	{
		QShortcut* shortcut = new QShortcut(parent);
		shortcut->setContext(context);
		shortcut->setKey(s);

		qshortcuts << shortcut;
	}

	ShortcutHandler::instance()->qt_shortcuts_added(this->identifier(), qshortcuts);

	return qshortcuts;
}

void Shortcut::set_qt_shortcuts(const QList<QShortcut*>& qt_shortcuts)
{
	m->qt_shortcuts = qt_shortcuts;
}

void Shortcut::remove_qt_shortcut(QShortcut* shortcut)
{
	auto it = std::remove_if(m->qt_shortcuts.begin(), m->qt_shortcuts.end(), [shortcut](QShortcut* sc){
		return (sc == shortcut);
	});

	m->qt_shortcuts.erase(it, m->qt_shortcuts.end());
}

QList<QShortcut*> Shortcut::qt_shortcuts() const
{
	return m->qt_shortcuts;
}


void Shortcut::change_shortcut(const QStringList& shortcuts)
{
	m->shortcuts.clear();
	for(QString str : shortcuts)
	{
		str.replace(" +", "+");
		str.replace("+ ", "+");

		m->shortcuts << str;
		m->shortcuts.removeDuplicates();
	}

	const QList<QKeySequence> new_ks = this->sequences();//merge_key_sequences(this->sequences());
	for(auto it=m->qt_shortcuts.begin(); it != m->qt_shortcuts.end(); it++)
	{
		if(new_ks.size() > 0)
		{
			QShortcut* sc = *it;
			sc->setKey(new_ks.first());
		}
	}
}
