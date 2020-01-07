/* GUI_PlaylistChooser.cpp */

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
	Chooser*	playlist_chooser=nullptr;
};

GUI_PlaylistChooser::GUI_PlaylistChooser(QWidget *parent) :
	PlayerPlugin::Base(parent)
{
	m = Pimpl::make<Private>();
}


GUI_PlaylistChooser::~GUI_PlaylistChooser()
{
	if(ui){
		delete ui; ui=nullptr;
	}
}


void GUI_PlaylistChooser::init_ui()
{
	if(!m->playlist_chooser){
		m->playlist_chooser = new Playlist::Chooser(this);
	}

	setup_parent(this, &ui);

	ui->btn_actions->show_action(ContextMenu::EntryRename, true);
	ui->btn_actions->show_action(ContextMenu::EntryDelete, true);

	connect(ui->btn_actions, &MenuToolButton::sig_rename, this, &GUI_PlaylistChooser::rename_triggered);
	connect(ui->btn_actions, &MenuToolButton::sig_delete, this, &GUI_PlaylistChooser::delete_triggered);
	connect(ui->combo_playlists, combo_activated_int, this, &GUI_PlaylistChooser::playlist_selected);
	connect(m->playlist_chooser, &Chooser::sig_playlists_changed, this, &GUI_PlaylistChooser::playlists_changed);

	playlists_changed();
}


void GUI_PlaylistChooser::retranslate_ui()
{
	ui->retranslateUi(this);

	const CustomPlaylistSkeletons& skeletons =
			m->playlist_chooser->playlists();

	if(skeletons.isEmpty())
	{
		ui->combo_playlists->clear();
		ui->combo_playlists->addItem(tr("No playlists found"), -1);
	}
}


QString GUI_PlaylistChooser::get_name() const
{
	return "Playlists";
}


QString GUI_PlaylistChooser::get_display_name() const
{
	return Lang::get(Lang::Playlists);
}


void GUI_PlaylistChooser::playlists_changed()
{
	if(!is_ui_initialized()){
		return;
	}

	QString old_text = ui->combo_playlists->currentText();

	const CustomPlaylistSkeletons& skeletons =
			m->playlist_chooser->playlists();

	ui->combo_playlists->clear();

	for(const CustomPlaylistSkeleton& skeleton : skeletons)
	{
		ui->combo_playlists->addItem(skeleton.name(), skeleton.id());
	}

	ui->btn_actions->setEnabled(!skeletons.isEmpty());
	if(skeletons.isEmpty())
	{
		ui->combo_playlists->addItem(tr("No playlists found"), -1);
	}

	int cur_idx = std::max(ui->combo_playlists->findText(old_text), 0);
	ui->combo_playlists->setCurrentIndex(cur_idx);
}

void GUI_PlaylistChooser::rename_triggered()
{
	auto* dialog = new Gui::LineInputDialog
	(
		Lang::get(Lang::Rename),
		Lang::get(Lang::Rename),
		ui->combo_playlists->currentText(),
		this
	);

	connect(dialog, &Gui::LineInputDialog::sig_closed, this, &GUI_PlaylistChooser::rename_dialog_closed);
	dialog->show();
}

void GUI_PlaylistChooser::rename_dialog_closed()
{
	using Util::SaveAsAnswer;
	auto* dialog = static_cast<Gui::LineInputDialog*>(sender());

	Gui::LineInputDialog::ReturnValue val = dialog->return_value();
	if(val == Gui::LineInputDialog::ReturnValue::Ok)
	{
		int id = ui->combo_playlists->currentData().toInt();
		QString new_name = dialog->text();

		SaveAsAnswer answer = m->playlist_chooser->rename_playlist(id, new_name);
		if(answer != SaveAsAnswer::Success)
		{
			QString error_msg = tr("Could not rename playlist");
			if(answer == SaveAsAnswer::InvalidName || answer == SaveAsAnswer::NameAlreadyThere )
			{
				error_msg += "<br>" + tr("Name is invalid");
			}

			Message::error(error_msg);
		}
	}
}

void GUI_PlaylistChooser::delete_triggered()
{
	int id = ui->combo_playlists->currentData().toInt();
	QString name = ui->combo_playlists->currentText();

	Message::Answer answer = Message::question_yn(tr("Do you really want to delete %1?").arg(name));
	if(answer == Message::Answer::Yes)
	{
		bool success = m->playlist_chooser->delete_playlist(id);
		if(!success)
		{
			Message::error(tr("Could not delete playlist %1").arg(name));
		}
	}
}


void GUI_PlaylistChooser::playlist_selected(int idx)
{
	int id = m->playlist_chooser->find_playlist(ui->combo_playlists->currentText());
	int data = ui->combo_playlists->itemData(idx).toInt();

	if(data < 0){
		return;
	}

	if(id >= 0){
		m->playlist_chooser->load_single_playlist(id);
	}
}

