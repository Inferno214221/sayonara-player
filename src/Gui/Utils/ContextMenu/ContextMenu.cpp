/* ContextMenu.cpp */

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

#include "ContextMenu.h"
#include "Gui/Utils/Icons.h"
#include "Gui/Utils/GuiUtils.h"
#include "Gui/Utils/PreferenceAction.h"

#include "Utils/Algorithm.h"
#include "Utils/Language/Language.h"

#include <QTimer>
#include <QKeySequence>

using namespace Gui;

namespace Algorithm=::Util::Algorithm;

struct ContextMenu::Private
{
	QAction*	actionNew=nullptr;
	QAction*	actionEdit=nullptr;
	QAction*	actionOpen=nullptr;
	QAction*	actionUndo=nullptr;
	QAction*	actionSave=nullptr;
	QAction*	actionSaveAs=nullptr;
	QAction*	actionRename=nullptr;
	QAction*	actionDelete=nullptr;
	QAction*	actionDefault=nullptr;
	QAction*	preferencePseparator=nullptr;

	QList<QAction*>		actions;
	QTimer*				timer=nullptr;

	bool				hasSpecialActions;
	bool				hasPreferenceActions;

	Private(QObject* parent) :
		hasSpecialActions(false),
		hasPreferenceActions(false)
	{
		timer = new QTimer(parent);
	}
};

ContextMenu::ContextMenu(QWidget* parent) :
	Gui::WidgetTemplate<QMenu>(parent)
{
	m = Pimpl::make<Private>(this);

	m->actionOpen = new QAction(this);
	m->actionEdit = new QAction(this);
	m->actionNew = new QAction(this);
	m->actionUndo = new QAction(this);
	m->actionDefault = new QAction(this);
	m->actionSave = new QAction(this);
	m->actionSaveAs = new QAction(this);
	m->actionRename = new QAction(this);
	m->actionDelete = new QAction(this);

	m->actions << addSeparator()
			<< m->actionNew
			<< m->actionOpen
			<< m->actionEdit
			<< m->actionSave
			<< m->actionSaveAs
			<< m->actionRename
			<< addSeparator()
			<< m->actionUndo
			<< m->actionDefault
			<< addSeparator()
			<< m->actionDelete
			<< addSeparator();

	this->addActions(m->actions);

	for(QAction* action : Algorithm::AsConst(m->actions))
	{
		action->setVisible(false);
	}

	connect(m->actionOpen, &QAction::triggered, this, &ContextMenu::sigOpen);
	connect(m->actionNew, &QAction::triggered, this, &ContextMenu::sigNew);
	connect(m->actionEdit, &QAction::triggered, this, &ContextMenu::sigEdit);
	connect(m->actionUndo, &QAction::triggered, this, &ContextMenu::sigUndo);
	connect(m->actionDefault, &QAction::triggered, this, &ContextMenu::sigDefault);
	connect(m->actionSave, &QAction::triggered, this, &ContextMenu::sigSave);
	connect(m->actionSaveAs, &QAction::triggered, this, &ContextMenu::sigSaveAs);
	connect(m->actionRename, &QAction::triggered, this, &ContextMenu::sigRename);
	connect(m->actionDelete, &QAction::triggered, this, &ContextMenu::sigDelete);
}

ContextMenu::~ContextMenu() = default;

void ContextMenu::languageChanged()
{
	m->actionNew->setText(Lang::get(Lang::New));
	m->actionEdit->setText(Lang::get(Lang::Edit));
	m->actionOpen->setText(Lang::get(Lang::Open));
	m->actionSave->setText(Lang::get(Lang::Save));
	m->actionSaveAs->setText(Lang::get(Lang::SaveAs).triplePt());
	m->actionRename->setText(Lang::get(Lang::Rename));
	m->actionUndo->setText(Lang::get(Lang::Undo));
	m->actionDefault->setText(Lang::get(Lang::Default));
	m->actionDelete->setText(Lang::get(Lang::Delete));

	m->actionOpen->setShortcut(QKeySequence::Open);
	m->actionNew->setShortcut(QKeySequence::New);
	m->actionUndo->setShortcut(QKeySequence::Undo);
	m->actionSave->setShortcut(QKeySequence::Save);
	m->actionSaveAs->setShortcut(QKeySequence::SaveAs);
	m->actionRename->setShortcut(QKeySequence("F2"));
	m->actionDelete->setShortcut(QKeySequence::Delete);
}

void ContextMenu::skinChanged()
{
	m->actionOpen->setIcon(Icons::icon(Icons::Open));
	m->actionEdit->setIcon(Icons::icon(Icons::Edit));
	m->actionNew->setIcon(Icons::icon(Icons::New));
	m->actionUndo->setIcon(Icons::icon(Icons::Undo));
	m->actionDefault->setIcon(Icons::icon(Icons::Undo));
	m->actionSave->setIcon(Icons::icon(Icons::Save));
	m->actionSaveAs->setIcon(Icons::icon(Icons::SaveAs));
	m->actionRename->setIcon(Icons::icon(Icons::Edit));
	m->actionDelete->setIcon(Icons::icon(Icons::Delete));
}

void ContextMenu::registerAction(QAction* action)
{
	m->actions << action;

	if(!m->hasSpecialActions)
	{
		QAction* sep = this->addSeparator();
		this->insertAction(m->preferencePseparator, sep);
		m->hasSpecialActions = true;
	}

	if(m->preferencePseparator)
	{
		this->insertAction(m->preferencePseparator, action);
	}

	else {
		addAction(action);
	}
}

void ContextMenu::showActions(ContextMenuEntries entries)
{
	m->actionNew->setVisible(entries & ContextMenu::EntryNew);
	m->actionEdit->setVisible(entries & ContextMenu::EntryEdit);
	m->actionOpen->setVisible(entries & ContextMenu::EntryOpen);
	m->actionUndo->setVisible(entries & ContextMenu::EntryUndo);
	m->actionDefault->setVisible(entries & ContextMenu::EntryDefault);
	m->actionSave->setVisible(entries & ContextMenu::EntrySave);
	m->actionSaveAs->setVisible(entries & ContextMenu::EntrySaveAs);
	m->actionRename->setVisible(entries & ContextMenu::EntryRename);
	m->actionDelete->setVisible(entries & ContextMenu::EntryDelete);
}

void ContextMenu::showAction(ContextMenu::Entry entry, bool visible)
{
	ContextMenuEntries entries = this->entries();
	if(visible){
		entries |= entry;
	}

	else{
		entries &= ~(entry);
	}

	showActions(entries);
}


bool ContextMenu::hasActions()
{
	return Algorithm::contains(m->actions, [](QAction* a){
		return a->isVisible();
	});
}

ContextMenuEntries ContextMenu::entries() const
{
	ContextMenuEntries entries = ContextMenu::EntryNone;

	if(m->actionNew->isVisible()){
		entries |= ContextMenu::EntryNew;
	}
	if(m->actionEdit->isVisible()){
		entries |= ContextMenu::EntryEdit;
	}
	if(m->actionDelete->isVisible()){
		entries |= ContextMenu::EntryDelete;
	}
	if(m->actionOpen->isVisible()){
		entries |= ContextMenu::EntryOpen;
	}
	if(m->actionRename->isVisible()){
		entries |= ContextMenu::EntryRename;
	}
	if(m->actionSave->isVisible()){
		entries |= ContextMenu::EntrySave;
	}
	if(m->actionSaveAs->isVisible()){
		entries |= ContextMenu::EntrySaveAs;
	}
	if(m->actionUndo->isVisible()){
		entries |= ContextMenu::EntryUndo;
	}
	if(m->actionDefault->isVisible()){
		entries |= ContextMenu::EntryDefault;
	}

	return entries;
}


void ContextMenu::showAll()
{
	for(QAction* action : Algorithm::AsConst(m->actions))
	{
		action->setVisible(true);
	}
}

void ContextMenu::addPreferenceAction(Gui::PreferenceAction* action)
{
	QList<QAction*> actions;

	if(!m->hasPreferenceActions)
	{
		m->preferencePseparator = this->addSeparator();
		actions << m->preferencePseparator;
		m->hasPreferenceActions = true;
	}

	actions << action;

	this->addActions(actions);
}

void ContextMenu::showEvent(QShowEvent* e)
{
	for(QAction* action : Algorithm::AsConst(m->actions))
	{
		action->setDisabled(true);
	}

	QTimer::singleShot(200, this, &ContextMenu::timedOut);

	WidgetTemplate<QMenu>::showEvent(e);
}

void ContextMenu::timedOut()
{
	for(QAction* action : Algorithm::AsConst(m->actions))
	{
		action->setDisabled(false);
	}
}
