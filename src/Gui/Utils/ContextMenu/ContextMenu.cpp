/* ContextMenu.cpp */

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

#include "ContextMenu.h"
#include "Gui/Utils/Icons.h"
#include "Gui/Utils/GuiUtils.h"
#include "Gui/Utils/PreferenceAction.h"

#include "Utils/Utils.h"
#include "Utils/Language/Language.h"

#include <QTimer>

using namespace Gui;

struct ContextMenu::Private
{
	QAction*	action_new=nullptr;
	QAction*	action_edit=nullptr;
	QAction*	action_open=nullptr;
	QAction*	action_undo=nullptr;
	QAction*	action_save=nullptr;
	QAction*	action_save_as=nullptr;
	QAction*	action_rename=nullptr;
	QAction*	action_delete=nullptr;
	QAction*	action_default=nullptr;
	QAction*	preference_separator=nullptr;

	QList<QAction*>		actions;
	QTimer*				timer=nullptr;

	bool				has_special_actions;
	bool				has_preference_actions;

	Private(QObject* parent) :
		has_special_actions(false),
		has_preference_actions(false)
	{
		timer = new QTimer(parent);
	}
};

ContextMenu::ContextMenu(QWidget *parent) :
	Gui::WidgetTemplate<QMenu>(parent)
{
	m = Pimpl::make<Private>(this);

	m->action_open = new QAction(this);
	m->action_edit = new QAction(this);
	m->action_new = new QAction(this);
	m->action_undo = new QAction(this);
	m->action_default = new QAction(this);
	m->action_save = new QAction(this);
	m->action_save_as = new QAction(this);
	m->action_rename = new QAction(this);
	m->action_delete = new QAction(this);

	m->actions << addSeparator()
			<< m->action_new
			<< m->action_open
			<< m->action_edit
			<< m->action_save
			<< m->action_save_as
			<< m->action_rename
			<< addSeparator()
			<< m->action_undo
			<< m->action_default
			<< addSeparator()
			<< m->action_delete
			<< addSeparator();

	this->addActions(m->actions);

	for(QAction* action : ::Util::AsConst(m->actions))
	{
		action->setVisible(false);
	}

	connect(m->action_open, &QAction::triggered, this, &ContextMenu::sig_open);
	connect(m->action_new, &QAction::triggered, this, &ContextMenu::sig_new);
	connect(m->action_edit, &QAction::triggered, this, &ContextMenu::sig_edit);
	connect(m->action_undo, &QAction::triggered, this, &ContextMenu::sig_undo);
	connect(m->action_default, &QAction::triggered, this, &ContextMenu::sig_default);
	connect(m->action_save, &QAction::triggered, this, &ContextMenu::sig_save);
	connect(m->action_save_as, &QAction::triggered, this, &ContextMenu::sig_save_as);
	connect(m->action_rename, &QAction::triggered, this, &ContextMenu::sig_rename);
	connect(m->action_delete, &QAction::triggered, this, &ContextMenu::sig_delete);
}

ContextMenu::~ContextMenu() {}

void ContextMenu::language_changed()
{
	 m->action_new->setText(Lang::get(Lang::New));
	 m->action_edit->setText(Lang::get(Lang::Edit));
	 m->action_open->setText(Lang::get(Lang::Open));
	 m->action_save->setText(Lang::get(Lang::Save));
	 m->action_save_as->setText(Lang::get(Lang::SaveAs).triplePt());
	 m->action_rename->setText(Lang::get(Lang::Rename));
	 m->action_undo->setText(Lang::get(Lang::Undo));
	 m->action_default->setText(Lang::get(Lang::Default));
	 m->action_delete->setText(Lang::get(Lang::Delete));
}

void ContextMenu::skin_changed()
{
	m->action_open->setIcon(Icons::icon(Icons::Open));
	m->action_edit->setIcon(Icons::icon(Icons::Edit));
	m->action_new->setIcon(Icons::icon(Icons::New));
	m->action_undo->setIcon(Icons::icon(Icons::Undo));
	m->action_default->setIcon(Icons::icon(Icons::Undo));
	m->action_save->setIcon(Icons::icon(Icons::Save));
	m->action_save_as->setIcon(Icons::icon(Icons::SaveAs));
	m->action_rename->setIcon(Icons::icon(Icons::Edit));
	m->action_delete->setIcon(Icons::icon(Icons::Delete));
}

void ContextMenu::register_action(QAction* action)
{
	m->actions << action;

	if(!m->has_special_actions)
	{
		QAction* sep = this->addSeparator();
		this->insertAction(m->preference_separator, sep);
		m->has_special_actions = true;
	}

	if(m->preference_separator)
	{
		this->insertAction(m->preference_separator, action);
	}

	else {
		addAction(action);
	}
}

void ContextMenu::show_actions(ContextMenuEntries entries)
{
	m->action_new->setVisible(entries & ContextMenu::EntryNew);
	m->action_edit->setVisible(entries & ContextMenu::EntryEdit);
	m->action_open->setVisible(entries & ContextMenu::EntryOpen);
	m->action_undo->setVisible(entries & ContextMenu::EntryUndo);
	m->action_default->setVisible(entries & ContextMenu::EntryDefault);
	m->action_save->setVisible(entries & ContextMenu::EntrySave);
	m->action_save_as->setVisible(entries & ContextMenu::EntrySaveAs);
	m->action_rename->setVisible(entries & ContextMenu::EntryRename);
	m->action_delete->setVisible(entries & ContextMenu::EntryDelete);
}

void ContextMenu::show_action(ContextMenu::Entry entry, bool visible)
{
	ContextMenuEntries entries = this->get_entries();
	if(visible){
		entries |= entry;
	}

	else{
		entries &= ~(entry);
	}

	show_actions(entries);
}


bool ContextMenu::has_actions()
{
	return ::Util::contains(m->actions, [](QAction* a){
		return a->isVisible();
	});
}

ContextMenuEntries ContextMenu::get_entries() const
{
	ContextMenuEntries entries = ContextMenu::EntryNone;

	if(m->action_new->isVisible()){
		entries |= ContextMenu::EntryNew;
	}
	if(m->action_edit->isVisible()){
		entries |= ContextMenu::EntryEdit;
	}
	if(m->action_delete->isVisible()){
		entries |= ContextMenu::EntryDelete;
	}
	if(m->action_open->isVisible()){
		entries |= ContextMenu::EntryOpen;
	}
	if(m->action_rename->isVisible()){
		entries |= ContextMenu::EntryRename;
	}
	if(m->action_save->isVisible()){
		entries |= ContextMenu::EntrySave;
	}
	if(m->action_save_as->isVisible()){
		entries |= ContextMenu::EntrySaveAs;
	}
	if(m->action_undo->isVisible()){
		entries |= ContextMenu::EntryUndo;
	}
	if(m->action_default->isVisible()){
		entries |= ContextMenu::EntryDefault;
	}

	return entries;
}


void ContextMenu::show_all()
{
	for(QAction* action : ::Util::AsConst(m->actions))
	{
		action->setVisible(true);
	}
}

void ContextMenu::add_preference_action(PreferenceAction* action)
{
	QList<QAction*> actions;

	if(!m->has_preference_actions)
	{
		m->preference_separator = this->addSeparator();
		actions << m->preference_separator;
		m->has_preference_actions = true;
	}

	actions << action;

	this->addActions(actions);
}

void ContextMenu::showEvent(QShowEvent* e)
{
	for(QAction* action : ::Util::AsConst(m->actions))
	{
		action->setDisabled(true);
	}

	QTimer::singleShot(300, this, SLOT(timed_out()));

	WidgetTemplate<QMenu>::showEvent(e);
}

void ContextMenu::timed_out()
{
	for(QAction* action : ::Util::AsConst(m->actions))
	{
		action->setDisabled(false);
	}
}


