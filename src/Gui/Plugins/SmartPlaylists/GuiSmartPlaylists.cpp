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
#include "Components/SmartPlaylists/SmartPlaylistCreator.h"
#include "Components/SmartPlaylists/SmartPlaylistManager.h"
#include "Components/LibraryManagement/LibraryManager.h"
#include "Utils/Algorithm.h"
#include "Utils/Language/Language.h"
#include "Utils/Library/LibraryInfo.h"

namespace
{
	constexpr const auto Dice = "⚂";

	void setupMenuButton(const int currentIndex, Gui::MenuToolButton* button)
	{
		const auto hasCurrentIndex = (currentIndex >= 0);

		button->showAction(Gui::ContextMenu::EntryNew, true);
		button->showAction(Gui::ContextMenu::EntryEdit, hasCurrentIndex);
		button->showAction(Gui::ContextMenu::EntryDelete, hasCurrentIndex);
	}

	QList<std::shared_ptr<SmartPlaylist>> sortSmartPlaylists(QList<std::shared_ptr<SmartPlaylist>> smartPlaylists)
	{
		Util::Algorithm::sort(smartPlaylists, [](const auto& smartPlaylist1, const auto& smartPlaylist2) {
			const auto type1 = smartPlaylist1->type();
			const auto type2 = smartPlaylist2->type();
			if(type1 != type2)
			{
				return type1 < type2;
			}

			for(auto i = 0; i < smartPlaylist1->count(); i++)
			{
				const auto value1 = smartPlaylist1->value(i);
				const auto value2 = smartPlaylist2->value(i);
				if(value1 != value2)
				{
					return value1 < value2;
				}
			}

			return smartPlaylist1->id() < smartPlaylist2->id();
		});

		return smartPlaylists;
	}

	QString getSmartPlaylistDescription(const SmartPlaylistPtr& smartPlaylist, Library::InfoAccessor* libraryManager)
	{
		const auto libraryId = smartPlaylist->libraryId();
		const auto libraryInfo = libraryManager->libraryInfo(libraryId);
		const auto libraryName = libraryInfo.name();

		const auto name = smartPlaylist->isRandomized()
		                  ? smartPlaylist->name() + " + " + Dice
		                  : smartPlaylist->name();

		return (libraryId == -1) || (libraryManager->count() == 1)
		       ? name
		       : QString("%1: %2").arg(libraryName, name);
	}
}

struct GuiSmartPlaylists::Private
{
	SmartPlaylistManager* smartPlaylistManager;
	Library::InfoAccessor* libraryManager;

	Private(SmartPlaylistManager* smartPlaylistManager, Library::InfoAccessor* libraryManager) :
		smartPlaylistManager {smartPlaylistManager},
		libraryManager {libraryManager} {}
};

GuiSmartPlaylists::GuiSmartPlaylists(SmartPlaylistManager* smartPlaylistManager, Library::InfoAccessor* libraryManager,
                                     QWidget* parent) :
	PlayerPlugin::Base(parent)
{
	m = Pimpl::make<Private>(smartPlaylistManager, libraryManager);
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
		const auto spid = Spid(ui->comboPlaylist->itemData(index).toInt());
		m->smartPlaylistManager->selectPlaylist(spid);
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
	auto* dialog = new MinMaxIntegerDialog(m->libraryManager, this);

	const auto status = dialog->exec();
	if(status == MinMaxIntegerDialog::Accepted)
	{
		const auto smartPlaylist = SmartPlaylists::createFromType(dialog->type(),
		                                                          -1,
		                                                          dialog->values(),
		                                                          dialog->isRandomized(),
		                                                          dialog->libraryId());
		m->smartPlaylistManager->insertPlaylist(smartPlaylist);
	}

	dialog->deleteLater();
}

void GuiSmartPlaylists::editClicked()
{
	const auto id = Spid(ui->comboPlaylist->currentData().toInt());
	auto smartPlaylist = m->smartPlaylistManager->smartPlaylist(id);
	auto* dialog = new MinMaxIntegerDialog(smartPlaylist, m->libraryManager, this);

	const auto status = dialog->exec();
	if(status == MinMaxIntegerDialog::Accepted)
	{
		const auto values = dialog->values();
		for(int i = 0; i < values.count(); i++)
		{
			smartPlaylist->setValue(i, values[i]);
		}

		smartPlaylist->setRandomized(dialog->isRandomized());
		smartPlaylist->setLibraryId(dialog->libraryId());

		m->smartPlaylistManager->updatePlaylist(id, smartPlaylist);
	}

	dialog->deleteLater();
}

void GuiSmartPlaylists::deleteClicked()
{
	const auto id = Spid(ui->comboPlaylist->currentData().toInt());
	m->smartPlaylistManager->deletePlaylist(id);
}

void GuiSmartPlaylists::setupPlaylists()
{
	const auto currentId = Spid(ui->comboPlaylist->currentData().toInt());

	ui->comboPlaylist->blockSignals(true);
	ui->comboPlaylist->clear();

	const auto smartPlaylists = sortSmartPlaylists(m->smartPlaylistManager->smartPlaylists());
	for(const auto& smartPlaylist: smartPlaylists)
	{
		const auto name = getSmartPlaylistDescription(smartPlaylist, m->libraryManager);
		ui->comboPlaylist->addItem(name, smartPlaylist->id());
	}

	const auto newIndex = ui->comboPlaylist->findData(currentId.id);
	if(newIndex >= 0)
	{
		ui->comboPlaylist->setCurrentIndex(newIndex);
	}

	ui->comboPlaylist->blockSignals(false);
}
