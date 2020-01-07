/* DirectoryContextMenu.cpp */

/* Copyright (C) 2011-2020  Lucio Carreras
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

#include "DirectoryContextMenu.h"

#include "Gui/Utils/Icons.h"
#include "Gui/Utils/PreferenceAction.h"

#include "Utils/Language/Language.h"

#include <QAction>

struct DirectoryContextMenu::Private
{
	QAction*	action_create_dir=nullptr;
	QAction*	action_rename=nullptr;
	QAction*	action_rename_by_tag=nullptr;
	QAction*	action_collapse_all=nullptr;
	DirectoryContextMenu::Mode mode;

	Private(DirectoryContextMenu::Mode mode, DirectoryContextMenu* parent) :
		mode(mode)
	{
		action_create_dir = new QAction(parent);
		action_rename = new QAction(parent);
		action_rename_by_tag = new QAction(parent);
		action_collapse_all = new QAction(parent);
	}
};

DirectoryContextMenu::DirectoryContextMenu(DirectoryContextMenu::Mode mode, QWidget* parent) :
	Library::ContextMenu(parent)
{
	m = Pimpl::make<Private>(mode, this);

	this->show_actions
	(
		(Library::ContextMenu::EntryPlay |
		Library::ContextMenu::EntryPlayNewTab |
		Library::ContextMenu::EntryDelete |
		Library::ContextMenu::EntryInfo |
		Library::ContextMenu::EntryEdit |
		Library::ContextMenu::EntryLyrics |
		Library::ContextMenu::EntryAppend |
		Library::ContextMenu::EntryPlayNext)
	);

	QAction* separator = this->addSeparator();

	QAction* action	= this->get_action(Library::ContextMenu::EntryDelete);
	if(action){
		this->insertActions(
			action,
			{separator, m->action_create_dir, m->action_rename, m->action_rename_by_tag}
		);
	}

	connect(m->action_create_dir, &QAction::triggered, this, &DirectoryContextMenu::sig_create_dir_clicked);
	connect(m->action_rename, &QAction::triggered, this, &DirectoryContextMenu::sig_rename_clicked);
	connect(m->action_rename_by_tag, &QAction::triggered, this, &DirectoryContextMenu::sig_rename_by_tag_clicked);
	connect(m->action_collapse_all, &QAction::triggered, this, &DirectoryContextMenu::sig_collapse_all_clicked);

	action = this->add_preference_action(new Gui::LibraryPreferenceAction(this));

	separator = this->addSeparator();
	this->insertActions(
		action,
		{m->action_collapse_all, separator}
	);

	switch(mode)
	{
		case DirectoryContextMenu::Mode::Dir:
			this->show_action(Library::ContextMenu::EntryLyrics, false);
			break;
		case DirectoryContextMenu::Mode::File:
			m->action_create_dir->setVisible(false);
			m->action_collapse_all->setVisible(false);
			break;
		default:
			break;
	}

	skin_changed();
	language_changed();
}

DirectoryContextMenu::~DirectoryContextMenu() = default;

void DirectoryContextMenu::set_create_dir_visible(bool b)
{
	if(m && m->action_create_dir){
		m->action_create_dir->setVisible(b);
	}
}

void DirectoryContextMenu::set_rename_visible(bool b)
{
	if(m && m->action_rename){
		m->action_rename->setVisible(b);
	}
}

void DirectoryContextMenu::set_rename_by_tag_visible(bool b)
{
	if(m && m->action_rename_by_tag){
		m->action_rename_by_tag->setVisible(b);
	}
}

void DirectoryContextMenu::set_collapse_all_visibled(bool b)
{
	if(m && m->action_collapse_all){
		m->action_collapse_all->setVisible(b);
	}
}

void DirectoryContextMenu::language_changed()
{
	Library::ContextMenu::language_changed();

	if(m && m->action_rename)
	{
		m->action_rename->setText(Lang::get(Lang::Rename));
		m->action_rename_by_tag->setText(tr("Rename by metadata"));
		m->action_create_dir->setText(tr("Create directory"));
		m->action_collapse_all->setText(tr("Collapse all"));
	}
}

void DirectoryContextMenu::skin_changed()
{
	Library::ContextMenu::skin_changed();

	using namespace Gui;
	if(m && m->action_rename)
	{
		m->action_rename->setIcon(Icons::icon(Icons::Rename));
		m->action_rename_by_tag->setIcon(Icons::icon(Icons::Rename));
		m->action_create_dir->setIcon(Icons::icon(Icons::New));
	}
}

