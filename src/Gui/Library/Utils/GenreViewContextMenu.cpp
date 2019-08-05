/* GenreViewContextMenu.cpp */

/* Copyright (C) 2011-2019 Lucio Carreras
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



#include "GenreViewContextMenu.h"
#include "Utils/Language/Language.h"
#include "Utils/Settings/Settings.h"

using Library::GenreViewContextMenu;

struct GenreViewContextMenu::Private
{
	QAction* toggle_tree_action=nullptr;
};

GenreViewContextMenu::GenreViewContextMenu(QWidget* parent) :
	Gui::ContextMenu(parent)
{
	m = Pimpl::make<Private>();

	bool show_tree = GetSetting(Set::Lib_GenreTree);
	m->toggle_tree_action = new QAction(this);
	m->toggle_tree_action->setCheckable(true);
	m->toggle_tree_action->setChecked(show_tree);
	m->toggle_tree_action->setText(Lang::get(Lang::Tree));

	this->register_action(m->toggle_tree_action);

	connect( m->toggle_tree_action, &QAction::triggered, this, &GenreViewContextMenu::toggle_tree_triggered);
}

GenreViewContextMenu::~GenreViewContextMenu() = default;

void GenreViewContextMenu::toggle_tree_triggered()
{
	SetSetting(Set::Lib_GenreTree, m->toggle_tree_action->isChecked());
}

void GenreViewContextMenu::language_changed()
{
	ContextMenu::language_changed();
	m->toggle_tree_action->setText(Lang::get(Lang::Tree));
}
