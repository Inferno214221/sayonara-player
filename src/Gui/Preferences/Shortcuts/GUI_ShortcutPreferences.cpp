/* GUI_ShortcutPreferences.cpp */

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

#include "GUI_ShortcutPreferences.h"
#include "GUI_ShortcutEntry.h"
#include "Gui/Preferences/ui_GUI_ShortcutPreferences.h"
#include "Gui/Utils/Shortcuts/ShortcutHandler.h"
#include "Utils/Language/Language.h"
#include "Utils/Set.h"

#include <QLineEdit>
#include <QPushButton>
#include <QTimer>
#include <QStringList>

#define ADD_TO_MAP(x) _btn_le_map[btn_##x] = le_##x

struct GUI_ShortcutPreferences::Private
{
	ShortcutHandler*			sch = nullptr;
	QList<GUI_ShortcutEntry*>	entries;
	QStringList					errorStrings;

	Private() :
		sch(ShortcutHandler::instance())
	{}
};

GUI_ShortcutPreferences::GUI_ShortcutPreferences(const QString& identifier) :
	Base(identifier)
{
	m = Pimpl::make<Private>();
}

GUI_ShortcutPreferences::~GUI_ShortcutPreferences()
{
	if(ui)
	{
		delete ui; ui=nullptr;
	}
}


void GUI_ShortcutPreferences::initUi()
{
	if(isUiInitialized()){
		return;
	}

	setupParent(this, &ui);

	this->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

	ui->cbTest->setVisible(false);

	const QList<ShortcutIdentifier> shortcuts = m->sch->allIdentifiers();
	for(ShortcutIdentifier shortcut : shortcuts)
	{
		GUI_ShortcutEntry* entry = new GUI_ShortcutEntry(shortcut);

		connect(entry, &GUI_ShortcutEntry::sigTestPressed,
				this, &GUI_ShortcutPreferences::testPressed);
		connect(entry, &GUI_ShortcutEntry::sigSequenceEntered,
				this, &GUI_ShortcutPreferences::sequenceEntered);

		ui->layoutEntries->addWidget(entry);

		m->entries << entry;
	}

	connect(ui->cbTest, &QCheckBox::toggled, ui->cbTest, [=]()
	{
		if(ui->cbTest->isChecked())
		{
			ui->cbTest->setText(Lang::get(Lang::Success));
			QTimer::singleShot(2500, ui->cbTest, SLOT(hide()));
		}
	});
}


QString GUI_ShortcutPreferences::actionName() const
{
	return tr("Shortcuts");
}


bool GUI_ShortcutPreferences::commit()
{
	m->errorStrings.clear();

	Util::Set<QKeySequence> sequences;

	foreach(GUI_ShortcutEntry* entry, m->entries)
	{
		QList<QKeySequence> lst = entry->sequences();
		for(const QKeySequence& s : lst)
		{
			QString str = s.toString().trimmed();
			if( sequences.contains(str) &&
				str.size() > 0)
			{
				m->errorStrings << str;
			}

			sequences.insert(str);
		}

		entry->commit();
	}

	return m->errorStrings.isEmpty();
}


void GUI_ShortcutPreferences::revert()
{
	foreach(GUI_ShortcutEntry* entry, m->entries)
	{
		entry->revert();
	}
}


void GUI_ShortcutPreferences::testPressed(const QList<QKeySequence>& sequences)
{
	if(sequences.isEmpty()){
		return;
	}

	ui->cbTest->setVisible(true);
	ui->cbTest->setText(tr("Press shortcut") + ": " + sequences.first().toString(QKeySequence::NativeText));
	ui->cbTest->setChecked(false);

	for(const QKeySequence& sequence : sequences){
		ui->cbTest->setShortcut(sequence);
	}

	ui->cbTest->setFocus();
}

void GUI_ShortcutPreferences::sequenceEntered()
{
	auto* entry = static_cast<GUI_ShortcutEntry*>(sender());
	QList<QKeySequence> sequences = entry->sequences();

	foreach(const GUI_ShortcutEntry* lst_entry, m->entries)
	{
		if(lst_entry == entry){
			continue;
		}

		const QList<QKeySequence> saved_sequences = lst_entry->sequences();
		for(const QKeySequence& seq1 : sequences)
		{
			QString seq1_str = seq1.toString(QKeySequence::NativeText);

			for(const QKeySequence& seq2 : saved_sequences)
			{
				QString seq2_str = seq2.toString(QKeySequence::NativeText);
				if(seq1_str == seq2_str && !seq1_str.isEmpty()){
					entry->showSequenceError();
					break;
				}
			}
		}
	}
}

void GUI_ShortcutPreferences::retranslate()
{
	ui->retranslateUi(this);
}

QString GUI_ShortcutPreferences::errorString() const
{
	return tr("Double shortcuts found") + ":" + m->errorStrings.join("\n");
}

