/* GUI_ShortcutPreferences.cpp */

/* Copyright (C) 2011-2019  Lucio Carreras
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
	QStringList					error_strings;

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


void GUI_ShortcutPreferences::init_ui()
{
	if(is_ui_initialized()){
		return;
	}

	setup_parent(this, &ui);

	this->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

	ui->cb_test->setVisible(false);

	const QList<ShortcutIdentifier> shortcuts = m->sch->shortcuts_ids();

	for(ShortcutIdentifier shortcut : shortcuts)
	{
		GUI_ShortcutEntry* entry = new GUI_ShortcutEntry(shortcut);

		connect(entry, &GUI_ShortcutEntry::sig_test_pressed,
				this, &GUI_ShortcutPreferences::test_pressed);
		connect(entry, &GUI_ShortcutEntry::sig_sequence_entered,
				this, &GUI_ShortcutPreferences::sequence_entered);

		ui->layout_entries->addWidget(entry);

		m->entries << entry;
	}

	connect(ui->cb_test, &QCheckBox::toggled, ui->cb_test, [=]()
	{
		if(ui->cb_test->isChecked())
		{
			ui->cb_test->setText(Lang::get(Lang::Success));
			QTimer::singleShot(2500, ui->cb_test, SLOT(hide()));
		}
	});
}


QString GUI_ShortcutPreferences::action_name() const
{
	return tr("Shortcuts");
}


bool GUI_ShortcutPreferences::commit()
{
	m->error_strings.clear();

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
				m->error_strings << str;
			}

			sequences.insert(str);
		}

		entry->commit();
	}

	return m->error_strings.isEmpty();
}


void GUI_ShortcutPreferences::revert()
{
	foreach(GUI_ShortcutEntry* entry, m->entries)
	{
		entry->revert();
	}
}


void GUI_ShortcutPreferences::test_pressed(const QList<QKeySequence>& sequences)
{
	if(sequences.isEmpty()){
		return;
	}

	ui->cb_test->setVisible(true);
	ui->cb_test->setText(tr("Press shortcut") + ": " + sequences.first().toString(QKeySequence::NativeText));
	ui->cb_test->setChecked(false);

	for(const QKeySequence& sequence : sequences){
		ui->cb_test->setShortcut(sequence);
	}

	ui->cb_test->setFocus();
}

void GUI_ShortcutPreferences::sequence_entered()
{
	GUI_ShortcutEntry* entry = static_cast<GUI_ShortcutEntry*>(sender());
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
					entry->show_sequence_error();
					break;
				}
			}
		}
	}
}

void GUI_ShortcutPreferences::retranslate_ui()
{
	ui->retranslateUi(this);
}

QString GUI_ShortcutPreferences::error_string() const
{
	return tr("Double shortcuts found") + ":" + m->error_strings.join("\n");
}

