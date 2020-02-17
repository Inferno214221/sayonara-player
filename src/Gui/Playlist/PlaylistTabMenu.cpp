
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

#include "PlaylistTabMenu.h"

#include "Gui/Utils/Icons.h"
#include "Gui/Utils/PreferenceAction.h"

#include "Utils/Language/Language.h"

using namespace Gui;
using Playlist::MenuEntry;
using Playlist::TabMenu;

struct TabMenu::Private
{
	QAction*	action_open_file=nullptr;
	QAction*	action_open_dir=nullptr;
	QAction*	action_delete=nullptr;
	QAction*	action_save=nullptr;
	QAction*	action_save_as=nullptr;
	QAction*    action_save_to_file=nullptr;
	QAction*	action_reset=nullptr;
	QAction*	action_close=nullptr;
	QAction*	action_close_others=nullptr;
	QAction*	action_rename=nullptr;
	QAction*	action_clear=nullptr;

	bool has_preference_action;

	Private() :
		has_preference_action(false)
	{}
};

TabMenu::TabMenu(QWidget* parent) :
	WidgetTemplate<QMenu>(parent)
{
	m = Pimpl::make<Private>();

	m->action_open_file = new QAction(this);

	m->action_open_dir = new QAction(this);
	m->action_reset = new QAction(this);
	m->action_rename = new QAction(this);
	m->action_delete = new QAction(this);
	m->action_save = new QAction(this);
	m->action_save_as = new QAction(this);
	m->action_save_to_file = new QAction(this);
	m->action_clear = new QAction(this);
	m->action_close = new QAction(this);

	m->action_close_others = new QAction(this);

	QList<QAction*> actions;
	actions << m->action_open_file
			<< m->action_open_dir
			<< this->addSeparator()
			<< m->action_reset
			<< this->addSeparator()
			<< m->action_rename
			<< m->action_save
			<< m->action_save_as
			<< m->action_save_to_file
			<< m->action_delete
			<< this->addSeparator()
			<< m->action_clear
			<< this->addSeparator()
			<< m->action_close_others
			<< m->action_close;

	this->addActions(actions);

	connect(m->action_open_file, &QAction::triggered, this, &TabMenu::sigOpenFileClicked);
	connect(m->action_open_dir, &QAction::triggered, this, &TabMenu::sigOpenDirClicked);
	connect(m->action_reset, &QAction::triggered, this, &TabMenu::sigResetClicked);
	connect(m->action_rename, &QAction::triggered, this, &TabMenu::sigRenameClicked);
	connect(m->action_delete, &QAction::triggered, this, &TabMenu::sigDeleteClicked);
	connect(m->action_save, &QAction::triggered, this, &TabMenu::sigSaveClicked);
	connect(m->action_save_as, &QAction::triggered, this, &TabMenu::sigSaveAsClicked);
	connect(m->action_save_to_file, &QAction::triggered, this, &TabMenu::sigSaveToFileClicked);
	connect(m->action_clear, &QAction::triggered, this, &TabMenu::sigClearClicked);
	connect(m->action_close, &QAction::triggered, this, &TabMenu::sigCloseClicked);
	connect(m->action_close_others, &QAction::triggered, this, &TabMenu::sigCloseOthersClicked);

	addPreferenceAction(new PlaylistPreferenceAction(this));
}

TabMenu::~TabMenu()
{
	this->clear();
}

void TabMenu::languageChanged()
{
	m->action_open_file->setText(Lang::get(Lang::OpenFile).triplePt());
	m->action_open_dir->setText(Lang::get(Lang::OpenDir).triplePt());
	m->action_reset->setText(Lang::get(Lang::Reset));
	m->action_rename->setText(Lang::get(Lang::Rename).triplePt());
	m->action_delete->setText(Lang::get(Lang::Delete));
	m->action_save->setText(Lang::get(Lang::Save));
	m->action_save_as->setText(Lang::get(Lang::SaveAs).triplePt());
	m->action_save_to_file->setText(Lang::get(Lang::SaveToFile));
	m->action_clear->setText(Lang::get(Lang::Clear));
	m->action_close->setText(Lang::get(Lang::Close));
	m->action_close_others->setText(Lang::get(Lang::CloseOthers));

	m->action_rename->setShortcut(QKeySequence("F2"));
	m->action_save->setShortcut(QKeySequence::Save);
	m->action_save_as->setShortcut(QKeySequence::SaveAs);
	m->action_open_file->setShortcut(QKeySequence::Open);

	QKeySequence ks(QKeySequence::Open);
	m->action_open_dir->setShortcut(QKeySequence("Shift+" + ks.toString()));
}

void TabMenu::skinChanged()
{
	m->action_open_file->setIcon(Icons::icon(Icons::Open) );
	m->action_open_dir->setIcon(Icons::icon(Icons::Open) );

	m->action_reset->setIcon(Icons::icon(Icons::Undo));
	m->action_rename->setIcon(Icons::icon(Icons::Rename));
	m->action_delete->setIcon(Icons::icon(Icons::Delete));
	m->action_save->setIcon(Icons::icon(Icons::Save));
	m->action_save_as->setIcon(Icons::icon(Icons::SaveAs));
	m->action_save_to_file->setIcon(Icons::icon(Icons::SaveAs));
	m->action_clear->setIcon(Icons::icon(Icons::Clear));
	m->action_close->setIcon(Icons::icon(Icons::Close));
	m->action_close_others->setIcon(Icons::icon(Icons::Close));
}

void TabMenu::showMenuItems(Playlist::MenuEntries entries)
{
	m->action_open_file->setVisible(entries & MenuEntry::OpenFile);
	m->action_open_dir->setVisible(entries & MenuEntry::OpenDir);
	m->action_reset->setVisible(entries & MenuEntry::Reset);
	m->action_rename->setVisible(entries & MenuEntry::Rename);
	m->action_delete->setVisible(entries & MenuEntry::Delete);
	m->action_save->setVisible(entries & MenuEntry::Save);
	m->action_save_as->setVisible(entries & MenuEntry::SaveAs);
	m->action_save_to_file->setVisible(entries & MenuEntry::SaveToFile);
	m->action_clear->setVisible(entries & MenuEntry::Clear);
	m->action_close->setVisible(entries & MenuEntry::Close);
	m->action_close_others->setVisible(entries & MenuEntry::CloseOthers);
}


void TabMenu::showClose(bool b)
{
	m->action_close->setVisible(b);
	m->action_close_others->setVisible(b);
}

void TabMenu::addPreferenceAction(Gui::PreferenceAction* action)
{
	QList<QAction*> actions;

	if(!m->has_preference_action){
		actions << this->addSeparator();
	}

	actions << action;

	this->addActions(actions);
	m->has_preference_action = true;
}
