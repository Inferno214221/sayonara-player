/* ContextMenu.cpp
 *
 * Copyright (C) 2011-2024 Michael Lugmair
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
#include "Utils/Language/Language.h"
#include "Gui/Utils/Icons.h"
#include <QAction>

using SC::ContextMenu;

struct ContextMenu::Private
{
	QAction* actionAddArtist;

	Private()
	{
		actionAddArtist = new QAction();
	}
};

ContextMenu::ContextMenu(QWidget* parent) : Library::ContextMenu(parent)
{
	m = Pimpl::make<Private>();

	this->insertAction(this->beforePreferenceAction(), m->actionAddArtist);
	connect(m->actionAddArtist, &QAction::triggered, this, &SC::ContextMenu::sigAddArtistTriggered);
}

ContextMenu::~ContextMenu() = default;

void ContextMenu::languageChanged()
{
	Library::ContextMenu::languageChanged();

	m->actionAddArtist->setText(Lang::get(Lang::AddArtist));
}

ContextMenu::Entries ContextMenu::entries() const
{
	ContextMenu::Entries entries = Library::ContextMenu::entries();

	if(m->actionAddArtist->isVisible()) {
		entries |= ContextMenu::SCEntryAddArtist;
	}

	return entries;
}

void ContextMenu::showActions(ContextMenu::Entries entries)
{
	m->actionAddArtist->setVisible(entries & SCEntryAddArtist);

	Library::ContextMenu::showActions(entries);
}

void ContextMenu::showAction(ContextMenu::Entry entry, bool visible)
{
	ContextMenu::Entries entries = this->entries();
	if(visible) {
		entries |= entry;
	}

	else {
		entries &= ~(entry);
	}

	SC::ContextMenu::showActions(entries);
}
