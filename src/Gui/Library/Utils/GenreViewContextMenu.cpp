/* GenreViewContextMenu.cpp */

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

#include "GenreViewContextMenu.h"
#include "Utils/Language/Language.h"
#include "Utils/Settings/Settings.h"

using Library::GenreViewContextMenu;

struct GenreViewContextMenu::Private
{
	QAction* actionToggleTree = nullptr;
};

GenreViewContextMenu::GenreViewContextMenu(QWidget* parent) :
	Gui::ContextMenu(parent)
{
	m = Pimpl::make<Private>();

	bool showTree = GetSetting(Set::Lib_GenreTree);
	m->actionToggleTree = new QAction(this);
	m->actionToggleTree->setCheckable(true);
	m->actionToggleTree->setChecked(showTree);
	m->actionToggleTree->setText(Lang::get(Lang::Tree));

	this->registerAction(m->actionToggleTree);

	connect(m->actionToggleTree, &QAction::triggered, this, &GenreViewContextMenu::toggleTreeTriggered);
}

GenreViewContextMenu::~GenreViewContextMenu() = default;

void GenreViewContextMenu::toggleTreeTriggered()
{
	SetSetting(Set::Lib_GenreTree, m->actionToggleTree->isChecked());
}

void GenreViewContextMenu::languageChanged()
{
	ContextMenu::languageChanged();
	m->actionToggleTree->setText(Lang::get(Lang::Tree));
}
