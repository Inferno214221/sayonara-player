/* Shortcut.cpp */

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

#include "Shortcut.h"
#include "ShortcutHandler.h"
#include "Gui/Utils/Widgets/Widget.h"

#include "Utils/RawShortcutMap.h"
#include "Utils/Logger/Logger.h"

#include <QKeySequence>
#include <QWidget>

struct Shortcut::Private
{
	QList<QShortcut*>	qtShortcuts;
	QStringList			defaultShortcuts;
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

Shortcut::Shortcut(ShortcutIdentifier identifier, const QStringList& shortcutStrings)
{
	m = Pimpl::make<Private>(identifier);

	m->defaultShortcuts = shortcutStrings;
	for(QString& str : m->defaultShortcuts){
		str.replace(" +", "+");
		str.replace("+ ", "+");
	}

	m->shortcuts = m->defaultShortcuts;
}

Shortcut::Shortcut(ShortcutIdentifier identifier, const QString& defaultShortcut) :
	Shortcut(identifier, QStringList(defaultShortcut)) {}

Shortcut::Shortcut(const Shortcut& other) :
	Shortcut()
{
	m->identifier =			other.m->identifier;
	m->defaultShortcuts =	other.m->defaultShortcuts;
	m->shortcuts =			other.m->shortcuts;
	m->qtShortcuts =		other.m->qtShortcuts;
}

Shortcut::~Shortcut() = default;

Shortcut& Shortcut::operator =(const Shortcut& other)
{
	m->identifier =			other.m->identifier;
	m->defaultShortcuts =	other.m->defaultShortcuts;
	m->shortcuts =			other.m->shortcuts;
	m->qtShortcuts =		other.m->qtShortcuts;

	return (*this);
}

QString Shortcut::name() const
{
	return ShortcutHandler::instance()->shortcut_text(m->identifier);
}

QStringList Shortcut::defaultShortcut() const
{
	return m->defaultShortcuts;
}

QList<QKeySequence> Shortcut::sequences() const
{
	QList<QKeySequence> sequences;

	const auto& shortcuts = this->shortcuts();
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

QString Shortcut::databaseKey() const
{
	return ShortcutHandler::instance()->databaseKey(m->identifier);
}

Shortcut Shortcut::getInvalid()
{
	return Shortcut();
}

bool Shortcut::isValid() const
{
	return (m->identifier != ShortcutIdentifier::Invalid);
}

void Shortcut::connect(QWidget* parent, QObject* receiver, const char* slot, Qt::ShortcutContext context)
{
	QList<QShortcut*> shortcuts = initQtShortcut(parent, context);
	for(QShortcut* sc : shortcuts)
	{
		parent->connect(sc, SIGNAL(activated()), receiver, slot);
	}
}


QList<QShortcut*> Shortcut::initQtShortcut(QWidget* parent, Qt::ShortcutContext context)
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

	ShortcutHandler::instance()->qtShortcutsAdded(this->identifier(), qshortcuts);

	return qshortcuts;
}

void Shortcut::setQtShortcuts(const QList<QShortcut*>& qtShortcuts)
{
	m->qtShortcuts = qtShortcuts;
}

void Shortcut::removeQtShortcut(QShortcut* shortcut)
{
	auto it = std::remove_if(m->qtShortcuts.begin(), m->qtShortcuts.end(), [shortcut](QShortcut* sc){
		return (sc == shortcut);
	});

	m->qtShortcuts.erase(it, m->qtShortcuts.end());
}

QList<QShortcut*> Shortcut::qtShortcuts() const
{
	return m->qtShortcuts;
}


void Shortcut::changeShortcut(const QStringList& shortcuts)
{
	m->shortcuts.clear();
	for(QString shortcut : shortcuts)
	{
		shortcut.replace(" +", "+");
		shortcut.replace("+ ", "+");
		shortcut.replace("enter", "Enter");
		shortcut.replace("return", "Return");

		if(shortcut.contains("Enter"))
		{
			QString scCopy(shortcut);
			scCopy.replace("Enter", "Return");
			m->shortcuts << scCopy;
		}

		else if(shortcut.contains("Return"))
		{
			QString scCopy(shortcut);
			scCopy.replace("Return", "Enter");
			m->shortcuts << scCopy;
		}

		m->shortcuts << shortcut;
	}

	m->shortcuts.removeDuplicates();

	const QList<QKeySequence> newKeySequences = this->sequences();
	if(newKeySequences.size() > 0)
	{
		const auto& qtShortcuts = m->qtShortcuts;
		for(QShortcut* sc : qtShortcuts)
		{
			sc->setKey(newKeySequences.first());
		}
	}
}
