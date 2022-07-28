/* GuiSmartPlaylists.cpp */
/*
 * Copyright (C) 2011-2022 Michael Lugmair
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

#include "GuiSmartPlaylists.h"
#include "ui_GuiSmartPlaylists.h"
#include "MinMaxIntegerDialog.h"

#include "Components/SmartPlaylists/SmartPlaylist.h"
#include "Components/SmartPlaylists/SmartPlaylistManager.h"
#include "Components/SmartPlaylists/SmartPlaylistCreator.h"
#include "Utils/Language/Language.h"

namespace
{
	void setupMenuButton(const int currentIndex, Gui::MenuToolButton* button)
	{
		const auto hasCurrentIndex = (currentIndex >= 0);

		button->showAction(Gui::ContextMenu::EntryNew, true);
		button->showAction(Gui::ContextMenu::EntryEdit, hasCurrentIndex);
		button->showAction(Gui::ContextMenu::EntryDelete, hasCurrentIndex);
	}
}

struct GuiSmartPlaylists::Private
{
	SmartPlaylistManager* smartPlaylistManager;

	explicit Private(SmartPlaylistManager* smartPlaylistManager) :
		smartPlaylistManager {smartPlaylistManager} {}
};

GuiSmartPlaylists::GuiSmartPlaylists(SmartPlaylistManager* smartPlaylistManager, QWidget* parent) :
	PlayerPlugin::Base(parent)
{
	m = Pimpl::make<Private>(smartPlaylistManager);
}

GuiSmartPlaylists::~GuiSmartPlaylists() noexcept
{
	if(ui)
	{
		delete ui;
		ui = nullptr;
	}
}

void GuiSmartPlaylists::selectedIndexChanged(const int index)
{
	if(index >= 0)
	{
		m->smartPlaylistManager->selectPlaylist(index);
	}

	setupMenuButton(ui->comboPlaylist->currentIndex(), ui->btnMenu);
}

QString GuiSmartPlaylists::name() const { return "smart-playlists"; }

QString GuiSmartPlaylists::displayName() const { return Lang::get(Lang::SmartPlaylists); }

void GuiSmartPlaylists::retranslate() { setupPlaylists(); }

void GuiSmartPlaylists::initUi()
{
	setupParent(this, &ui);

	setupPlaylists();

	connect(ui->comboPlaylist, combo_activated_int, this, &GuiSmartPlaylists::selectedIndexChanged);
	connect(m->smartPlaylistManager,
	        &SmartPlaylistManager::sigPlaylistsChanged,
	        this,
	        &GuiSmartPlaylists::setupPlaylists);

	connect(ui->btnMenu, &Gui::MenuToolButton::sigNew, this, &GuiSmartPlaylists::newClicked);
	connect(ui->btnMenu, &Gui::MenuToolButton::sigDelete, this, &GuiSmartPlaylists::deleteClicked);
	connect(ui->btnMenu, &Gui::MenuToolButton::sigEdit, this, &GuiSmartPlaylists::editClicked);

	setupMenuButton(ui->comboPlaylist->currentIndex(), ui->btnMenu);
}

void GuiSmartPlaylists::newClicked()
{
	auto* dialog = new MinMaxIntegerDialog(this);

	const auto status = dialog->exec();
	if(status == MinMaxIntegerDialog::Accepted)
	{
		const auto smartPlaylist =
			SmartPlaylists::createFromType(dialog->type(), -1, dialog->values());

		m->smartPlaylistManager->insertPlaylist(smartPlaylist);
	}

	dialog->deleteLater();
}

void GuiSmartPlaylists::editClicked()
{
	const auto smartPlaylist = m->smartPlaylistManager->smartPlaylist(ui->comboPlaylist->currentIndex());
	auto* dialog = new MinMaxIntegerDialog(smartPlaylist, this);

	const auto status = dialog->exec();
	if(status == MinMaxIntegerDialog::Accepted)
	{
		const auto values = dialog->values();
		for(int i = 0; i < values.count(); i++)
		{
			smartPlaylist->setValue(i, values[i]);
		}

		m->smartPlaylistManager->updatePlaylist(ui->comboPlaylist->currentIndex(), smartPlaylist);
	}

	dialog->deleteLater();
}

void GuiSmartPlaylists::deleteClicked()
{
	m->smartPlaylistManager->deletePlaylist(ui->comboPlaylist->currentIndex());
}

void GuiSmartPlaylists::setupPlaylists()
{
	ui->comboPlaylist->blockSignals(true);
	ui->comboPlaylist->clear();

	const auto currentIndex = ui->comboPlaylist->currentIndex();
	const auto smartPlaylists = m->smartPlaylistManager->smartPlaylists();
	for(const auto& smartPlaylist: smartPlaylists)
	{
		ui->comboPlaylist->addItem(smartPlaylist->name());
	}

	const auto newIndex = (currentIndex < ui->comboPlaylist->count())
	                      ? currentIndex
	                      : currentIndex - 1;
	if(newIndex >= 0)
	{
		ui->comboPlaylist->setCurrentIndex(newIndex);
	}

	ui->comboPlaylist->blockSignals(false);
}
