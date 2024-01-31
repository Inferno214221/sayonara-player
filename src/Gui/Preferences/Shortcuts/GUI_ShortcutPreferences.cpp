/* GUI_ShortcutPreferences.cpp */

/* Copyright (C) 2011-2024 Michael Lugmair (Lucio Carreras)
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
#include "Utils/Algorithm.h"
#include "Utils/Language/Language.h"
#include "Utils/Set.h"

#include <QLineEdit>
#include <QPushButton>
#include <QStringList>
#include <QTimer>

namespace
{
	QString toString(const QKeySequence& ks) { return ks.toString(QKeySequence::NativeText); }

	bool hasOverlapping(const QList<QKeySequence>& sequences1, const QList<QKeySequence>& sequences2)
	{
		return Util::Algorithm::contains(sequences1, [&](const auto& sequence) {
			const auto str = toString(sequence);
			return !str.isEmpty() && Util::Algorithm::contains(sequences2, [&](const auto& otherSequence) {
				return toString(otherSequence) == str;
			});
		});
	}

	bool isSequenceAlreadyUsed(GUI_ShortcutEntry* newEntry, const QList<GUI_ShortcutEntry*>& entries)
	{
		return Util::Algorithm::contains(entries, [&](const auto* entry) {
			return (entry != newEntry) && hasOverlapping(newEntry->sequences(), entry->sequences());
		});
	}
}

struct GUI_ShortcutPreferences::Private
{
	ShortcutHandler* sch = nullptr;
	QList<GUI_ShortcutEntry*> entries;
	QStringList errorStrings;

	Private() :
		sch(ShortcutHandler::instance()) {}
};

GUI_ShortcutPreferences::GUI_ShortcutPreferences(const QString& identifier) :
	Base(identifier),
	m {Pimpl::make<Private>()} {}

GUI_ShortcutPreferences::~GUI_ShortcutPreferences() = default;

void GUI_ShortcutPreferences::initUi()
{
	ui = std::make_shared<Ui::GUI_ShortcutPreferences>();
	ui->setupUi(this);

	setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

	ui->cbTest->setVisible(false);

	const auto shortcuts = m->sch->allIdentifiers();
	for(const auto& shortcut: shortcuts)
	{
		auto* entry = new GUI_ShortcutEntry(shortcut);

		connect(entry, &GUI_ShortcutEntry::sigTestPressed,
		        this, &GUI_ShortcutPreferences::testPressed);
		connect(entry, &GUI_ShortcutEntry::sigSequenceEntered,
		        this, &GUI_ShortcutPreferences::sequenceEntered);

		ui->layoutEntries->addWidget(entry);

		m->entries << entry;
	}

	connect(ui->cbTest, &QCheckBox::toggled, ui->cbTest, [this]() {
		if(ui->cbTest->isChecked())
		{
			ui->cbTest->setText(Lang::get(Lang::Success));
			QTimer::singleShot(2500, ui->cbTest, SLOT(hide()));
		}
	});
}

QString GUI_ShortcutPreferences::actionName() const { return tr("Shortcuts"); }

bool GUI_ShortcutPreferences::commit()
{
	m->errorStrings.clear();

	Util::Set<QKeySequence> sequences;

	for(auto* entry: m->entries)
	{
		const auto availableSequences = entry->sequences();
		for(const auto& sequence: availableSequences)
		{
			const auto str = sequence.toString().trimmed();
			if(sequences.contains(str) && !str.isEmpty())
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
	for(auto* entry: m->entries)
	{
		entry->revert();
	}
}

void GUI_ShortcutPreferences::testPressed(const QList<QKeySequence>& sequences)
{
	if(sequences.isEmpty())
	{
		return;
	}

	ui->cbTest->setVisible(true);
	ui->cbTest->setText(tr("Press shortcut") + ": " + sequences.first().toString(QKeySequence::NativeText));
	ui->cbTest->setChecked(false);

	for(const auto& sequence: sequences)
	{
		ui->cbTest->setShortcut(sequence);
	}

	ui->cbTest->setFocus();
}

void GUI_ShortcutPreferences::sequenceEntered()
{
	auto* entry = dynamic_cast<GUI_ShortcutEntry*>(sender());
	if(isSequenceAlreadyUsed(entry, m->entries))
	{
		entry->showSequenceError();
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

