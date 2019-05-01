/* MenuTool.cpp */

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

#include "MenuTool.h"
#include "GUI/Utils/PreferenceAction.h"

struct MenuToolButton::Private
{
	ContextMenu* menu=nullptr;

	Private(MenuToolButton* parent) :
		menu(new ContextMenu(parent))
	{}
};


MenuToolButton::MenuToolButton(QWidget *parent) :
	MenuButton(parent)
{
	m = Pimpl::make<Private>(this);

	connect(m->menu, &ContextMenu::sig_open, this,  &MenuToolButton::sig_open);
	connect(m->menu, &ContextMenu::sig_new, this, &MenuToolButton::sig_new);
	connect(m->menu, &ContextMenu::sig_undo, this, &MenuToolButton::sig_undo);
	connect(m->menu, &ContextMenu::sig_default, this, &MenuToolButton::sig_default);
	connect(m->menu, &ContextMenu::sig_save, this, &MenuToolButton::sig_save);
	connect(m->menu, &ContextMenu::sig_save_as, this, &MenuToolButton::sig_save_as);
	connect(m->menu, &ContextMenu::sig_rename, this, &MenuToolButton::sig_rename);
	connect(m->menu, &ContextMenu::sig_delete, this, &MenuToolButton::sig_delete);
	connect(m->menu, &ContextMenu::sig_edit, this, &MenuToolButton::sig_edit);

	prove_enabled();
}

MenuToolButton::~MenuToolButton() {}

void MenuToolButton::register_action(QAction *action)
{
	m->menu->register_action(action);
	prove_enabled();
}

void MenuToolButton::register_preference_action(PreferenceAction* action)
{
	m->menu->add_preference_action(action);
}

void MenuToolButton::show_menu(QPoint pos)
{
	MenuButton::show_menu(pos);

	this->setDisabled(true);
	m->menu->popup(pos);
	this->setEnabled(true);
}


bool MenuToolButton::prove_enabled()
{
	bool enabled = m->menu->has_actions();
	this->setEnabled(enabled);
	return enabled;
}

void MenuToolButton::show_all()
{
	m->menu->show_all();
	prove_enabled();
}

void MenuToolButton::show_action(ContextMenu::Entry entry, bool visible)
{
	m->menu->show_action(entry, visible);
	prove_enabled();
}

void MenuToolButton::show_actions(ContextMenuEntries entries)
{
	m->menu->show_actions(entries);
	prove_enabled();
}

ContextMenuEntries MenuToolButton::get_entries() const
{
	return m->menu->get_entries();
}
