
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

#include "GUI_ShortcutEntry.h"

#include "Gui/Preferences/ui_GUI_ShortcutEntry.h"
#include "Gui/Utils/Icons.h"

#include "Utils/Language/Language.h"
#include "Utils/Logger/Logger.h"

#include <QKeySequence>
#include <QShortcut>
#include <QMessageBox>

struct GUI_ShortcutEntry::Private
{
	ShortcutHandler* sch = nullptr;
	ShortcutIdentifier identifier;

	Private(ShortcutIdentifier identifier) :
		sch(ShortcutHandler::instance()),
		identifier(identifier) {}
};

GUI_ShortcutEntry::GUI_ShortcutEntry(ShortcutIdentifier identifier, QWidget* parent) :
	Widget(parent)
{
	m = Pimpl::make<Private>(identifier);
	Shortcut sc = m->sch->shortcut(identifier);
	if(sc.name().trimmed().isEmpty())
	{
		spLog(Log::Warning, this) << "Shortcut name is empty";
	}

	ui = new Ui::GUI_ShortcutEntry();
	ui->setupUi(this);

	ui->leEntry->setPlaceholderText(tr("Enter shortcut"));
	ui->labDescription->setText(sc.name());
	ui->leEntry->setText(sc.shortcuts().join(", "));

	connect(ui->btnEdit, &QPushButton::clicked, this, &GUI_ShortcutEntry::editClicked);
	connect(ui->btnDefault, &QPushButton::clicked, this, &GUI_ShortcutEntry::defaultClicked);
	connect(ui->btnTest, &QPushButton::clicked, this, &GUI_ShortcutEntry::testClicked);
	connect(ui->leEntry, &ShortcutLineEdit::sigSequenceEntered, this, &GUI_ShortcutEntry::sigSequenceEntered);

	skinChanged();
}

GUI_ShortcutEntry::~GUI_ShortcutEntry()
{
	if(ui)
	{
		delete ui;
		ui = nullptr;
	}
}

QList<QKeySequence> GUI_ShortcutEntry::sequences() const
{
	return ui->leEntry->sequences();
}

void GUI_ShortcutEntry::showSequenceError()
{
	ui->leEntry->clear();
	QMessageBox::warning(this, Lang::get(Lang::Error), tr("Shortcut already in use"));
}

void GUI_ShortcutEntry::commit()
{
	QStringList lst = ui->leEntry->text().split(",");
	for(auto it = lst.begin(); it != lst.end(); it++)
	{
		*it = it->trimmed();
	}

	m->sch->setShortcut(m->identifier, lst);
}

void GUI_ShortcutEntry::clear()
{
	ui->leEntry->clear();
}

void GUI_ShortcutEntry::revert()
{
	Shortcut sc = m->sch->shortcut(m->identifier);

	ui->leEntry->setText(
		sc.shortcuts().join(", ")
	);
}

void GUI_ShortcutEntry::defaultClicked()
{
	Shortcut sc = m->sch->shortcut(m->identifier);

	ui->leEntry->setText(
		sc.defaultShortcut().join(", ")
	);
}

void GUI_ShortcutEntry::testClicked()
{
	QStringList splitted = ui->leEntry->text().split(", ");
	QList<QKeySequence> sequences;

	for(const QString& str: splitted)
	{
		sequences << QKeySequence::fromString(str, QKeySequence::NativeText);
	}

	emit sigTestPressed(sequences);
}

void GUI_ShortcutEntry::languageChanged()
{
	ui->retranslateUi(this);

	Shortcut sc = m->sch->shortcut(m->identifier);
	ui->labDescription->setText(sc.name());

	ui->btnDefault->setToolTip(Lang::get(Lang::Default));
	ui->btnEdit->setToolTip(Lang::get(Lang::Edit));
	ui->btnTest->setToolTip(tr("Test"));
}

void GUI_ShortcutEntry::skinChanged()
{
	using namespace Gui;
	ui->btnDefault->setIcon(Icons::icon(Icons::Undo));
	ui->btnEdit->setIcon(Icons::icon(Icons::Edit));
	ui->btnTest->setIcon(Icons::icon(Icons::Info));
}

void GUI_ShortcutEntry::editClicked()
{
	ui->leEntry->clear();
	ui->leEntry->setFocus();
}

