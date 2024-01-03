/* GUI_PlaylistChooser.cpp */

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

#include "GUI_PlaylistChooser.h"
#include "GUI_TargetPlaylistDialog.h"
#include "Gui/Plugins/ui_GUI_PlaylistChooser.h"
#include "Gui/Utils/InputDialog/LineInputDialog.h"

#include "Components/Playlist/PlaylistChooser.h"
#include "Components/Playlist/PlaylistHandler.h"

#include "Utils/Algorithm.h"
#include "Utils/Language/Language.h"
#include "Utils/Message/Message.h"
#include "Utils/Playlist/CustomPlaylist.h"
#include "Utils/Language/Language.h"

using Gui::ContextMenu;
using Gui::ContextMenuEntries;
using Gui::MenuToolButton;

using Playlist::Chooser;

struct GUI_PlaylistChooser::Private
{
	Chooser* playlistChooser = nullptr;

	Private(Playlist::Chooser* playlistChooser) :
		playlistChooser(playlistChooser) {}
};

GUI_PlaylistChooser::GUI_PlaylistChooser(Playlist::Chooser* playlistChooser, QWidget* parent) :
	PlayerPlugin::Base(parent)
{
	m = Pimpl::make<Private>(playlistChooser);
}

GUI_PlaylistChooser::~GUI_PlaylistChooser()
{
	if(ui)
	{
		delete ui;
		ui = nullptr;
	}
}

void GUI_PlaylistChooser::initUi()
{
	setupParent(this, &ui);

	ui->btnActions->showAction(ContextMenu::EntryRename, true);
	ui->btnActions->showAction(ContextMenu::EntryDelete, true);

	connect(ui->btnActions, &MenuToolButton::sigRename, this, &GUI_PlaylistChooser::renameTriggered);
	connect(ui->btnActions, &MenuToolButton::sigDelete, this, &GUI_PlaylistChooser::deleteTriggered);
	connect(ui->comboPlaylists, combo_activated_int, this, &GUI_PlaylistChooser::playlistSelected);
	connect(m->playlistChooser, &Chooser::sigPlaylistsChanged, this, &GUI_PlaylistChooser::playlistsChanged);

	playlistsChanged();
}

void GUI_PlaylistChooser::retranslate()
{
	ui->retranslateUi(this);

	const auto& playlists = m->playlistChooser->playlists();
	if(playlists.isEmpty())
	{
		ui->comboPlaylists->clear();
		ui->comboPlaylists->addItem(tr("No playlists found"), -1);
	}
}

QString GUI_PlaylistChooser::name() const
{
	return "Playlists";
}

QString GUI_PlaylistChooser::displayName() const
{
	return Lang::get(Lang::Playlists);
}

void GUI_PlaylistChooser::playlistsChanged()
{
	if(!isUiInitialized())
	{
		return;
	}

	QString old_text = ui->comboPlaylists->currentText();

	const auto& playlists =
		m->playlistChooser->playlists();

	ui->comboPlaylists->clear();

	for(const auto& playlist: playlists)
	{
		ui->comboPlaylists->addItem(playlist.name(), playlist.id());
	}

	ui->btnActions->setEnabled(!playlists.isEmpty());
	if(playlists.isEmpty())
	{
		ui->comboPlaylists->addItem(tr("No playlists found"), -1);
	}

	int cur_idx = std::max(ui->comboPlaylists->findText(old_text), 0);
	ui->comboPlaylists->setCurrentIndex(cur_idx);
}

void GUI_PlaylistChooser::renameTriggered()
{
	auto* dialog = new Gui::LineInputDialog
		(
			Lang::get(Lang::Rename),
			Lang::get(Lang::Rename),
			ui->comboPlaylists->currentText(),
			this
		);

	connect(dialog, &Gui::LineInputDialog::sigClosed, this, &GUI_PlaylistChooser::renameDialogClosed);
	dialog->show();
}

void GUI_PlaylistChooser::renameDialogClosed()
{
	using Util::SaveAsAnswer;
	auto* dialog = static_cast<Gui::LineInputDialog*>(sender());

	Gui::LineInputDialog::ReturnValue val = dialog->returnValue();
	if(val == Gui::LineInputDialog::ReturnValue::Ok)
	{
		int id = ui->comboPlaylists->currentData().toInt();
		QString new_name = dialog->text();

		SaveAsAnswer answer = m->playlistChooser->renamePlaylist(id, new_name);
		if(answer != SaveAsAnswer::Success)
		{
			QString error_msg = tr("Could not rename playlist");
			if(answer == SaveAsAnswer::InvalidName || answer == SaveAsAnswer::NameAlreadyThere)
			{
				error_msg += "<br>" + tr("Name is invalid");
			}

			Message::error(error_msg);
		}
	}
}

void GUI_PlaylistChooser::deleteTriggered()
{
	int id = ui->comboPlaylists->currentData().toInt();
	QString name = ui->comboPlaylists->currentText();

	Message::Answer answer = Message::question_yn(tr("Do you really want to delete %1?").arg(name));
	if(answer == Message::Answer::Yes)
	{
		bool success = m->playlistChooser->deletePlaylist(id);
		if(!success)
		{
			Message::error(tr("Could not delete playlist %1").arg(name));
		}
	}
}

void GUI_PlaylistChooser::playlistSelected(int idx)
{
	int id = m->playlistChooser->findPlaylist(ui->comboPlaylists->currentText());
	int data = ui->comboPlaylists->itemData(idx).toInt();

	if(data < 0)
	{
		return;
	}

	if(id >= 0)
	{
		m->playlistChooser->loadSinglePlaylist(id);
	}
}

