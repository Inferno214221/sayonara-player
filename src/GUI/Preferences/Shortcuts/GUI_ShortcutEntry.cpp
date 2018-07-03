
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

#include "GUI_ShortcutEntry.h"

#include "GUI/Preferences/ui_GUI_ShortcutEntry.h"
#include "GUI/Utils/Icons.h"
#include "GUI/Utils/Shortcuts/ShortcutHandler.h"

#include "Utils/Language.h"

#include <QKeySequence>
#include <QShortcut>
#include <QMessageBox>

struct GUI_ShortcutEntry::Private
{
	ShortcutHandler*		sch=nullptr;
	QString					identifier;

	Private(const QString& identifier) :
		sch(ShortcutHandler::instance()),
		identifier(identifier)
	{}
};

GUI_ShortcutEntry::GUI_ShortcutEntry(const QString& identifier, QWidget* parent) :
	Widget(parent)
{
	m = Pimpl::make<Private>(identifier);
	Shortcut sc = m->sch->get_shortcut(identifier);

	ui = new Ui::GUI_ShortcutEntry();
	ui->setupUi(this);

	ui->le_entry->setPlaceholderText(tr("Enter shortcut"));
	ui->lab_description->setText(sc.get_name());
	ui->le_entry->setText(sc.get_shortcuts().join(", "));

	connect(ui->btn_edit, &QPushButton::clicked, this, &GUI_ShortcutEntry::edit_clicked);
	connect(ui->btn_default, &QPushButton::clicked, this, &GUI_ShortcutEntry::default_clicked);
	connect(ui->btn_test, &QPushButton::clicked, this, &GUI_ShortcutEntry::test_clicked);
	connect(ui->le_entry, &ShortcutLineEdit::sig_sequence_entered, this, &GUI_ShortcutEntry::sig_sequence_entered);

	skin_changed();
}

GUI_ShortcutEntry::~GUI_ShortcutEntry()
{
	if(ui){ delete ui; ui = nullptr; }
}

QList<QKeySequence> GUI_ShortcutEntry::get_sequences() const
{
	return ui->le_entry->get_sequences();
}

void GUI_ShortcutEntry::show_sequence_error()
{
	ui->le_entry->clear();
	QMessageBox::warning(this, Lang::get(Lang::Error), tr("Shortcut already in use"));
}

void GUI_ShortcutEntry::commit()
{
	m->sch->set_shortcut(m->identifier, ui->le_entry->text().split(", "));
}

void GUI_ShortcutEntry::clear()
{
	ui->le_entry->clear();
}

void GUI_ShortcutEntry::revert()
{
	Shortcut sc = m->sch->get_shortcut(m->identifier);

	ui->le_entry->setText(
		sc.get_shortcuts().join(", ")
	);
}


void GUI_ShortcutEntry::default_clicked()
{
	Shortcut sc = m->sch->get_shortcut(m->identifier);

	ui->le_entry->setText(
		sc.get_default().join(", ")
	);
}

void GUI_ShortcutEntry::test_clicked()
{
	QStringList splitted = ui->le_entry->text().split(", ");
	QList<QKeySequence> sequences;

	for(const QString& str : splitted){
		sequences << QKeySequence::fromString(str, QKeySequence::NativeText);
	}

	emit sig_test_pressed(sequences);
}

void GUI_ShortcutEntry::language_changed()
{
	ui->retranslateUi(this);

	Shortcut sc = m->sch->get_shortcut(m->identifier);
	ui->lab_description->setText(sc.get_name());

	ui->btn_default->setToolTip(Lang::get(Lang::Default));
	ui->btn_edit->setToolTip(Lang::get(Lang::Edit));
	ui->btn_test->setToolTip(tr("Test"));
}

void GUI_ShortcutEntry::skin_changed()
{
	using namespace Gui;
	ui->btn_default->setIcon(Icons::icon(Icons::Undo));
	ui->btn_edit->setIcon(Icons::icon(Icons::Edit));
	ui->btn_test->setIcon(Icons::icon(Icons::Info));
}


void GUI_ShortcutEntry::edit_clicked()
{
	ui->le_entry->clear();
	ui->le_entry->setFocus();
}

