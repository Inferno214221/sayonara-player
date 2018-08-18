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

//static QList<QKeySequence> merge_key_sequences(const QList<QKeySequence>& sequences)
//{
//	QList<QKeySequence> ret = sequences;

//	for(QKeySequence ks : sequences)
//	{
//		QString str = ks.toString();

//		if(ks.toString().contains("return", Qt::CaseInsensitive))
//		{
//			str.replace("return", "Enter", Qt::CaseInsensitive);
//			if(!ret.contains(QKeySequence(str))){
//				ret << QKeySequence(str);
//			}
//		}

//		else if(ks.toString().contains("enter", Qt::CaseInsensitive))
//		{
//			str.replace("num+enter", "enter", Qt::CaseInsensitive);
//			str.replace("enter", "Return", Qt::CaseInsensitive);
//			if(!ret.contains(QKeySequence(str))){
//				ret << QKeySequence(str);
//			}
//		}
//	}

//	return ret;
//}


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
		QKeySequence seq = QKeySequence::fromString(str, QKeySequence::NativeText);
		sequences << seq;
	}

	if(sequences.isEmpty()){
		sequences << QKeySequence();
	}

	return sequences;
}

QKeySequence Shortcut::sequence() const
{
	QList<QKeySequence> sequences = this->sequences();
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

	const QList<QKeySequence> sequences = this->sequences();//merge_key_sequences(this->sequences());

	for(QKeySequence s : sequences)
	{
		QShortcut* shortcut = new QShortcut(parent);
		shortcut->setContext(context);
		shortcut->setKey(s);

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
		m->shortcuts.removeDuplicates();
	}

	const QList<QKeySequence> new_ks = this->sequences();//merge_key_sequences(this->sequences());
	foreach(QShortcut* sc, m->qt_shortcuts)
	{
		if(new_ks.size() > 0)
		{
			sc->setKey(new_ks.first());
		}
	}
}
