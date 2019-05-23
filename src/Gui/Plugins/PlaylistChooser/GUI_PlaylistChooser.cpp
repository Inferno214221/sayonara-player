/* GUI_PlaylistChooser.cpp */

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

#include "GUI_PlaylistChooser.h"
#include "GUI_TargetPlaylistDialog.h"
#include "Gui/Plugins/ui_GUI_PlaylistChooser.h"

#include "Components/PlaylistChooser/PlaylistChooser.h"

#include "Utils/Message/Message.h"

#include "Utils/Playlist/CustomPlaylist.h"
#include "Utils/Language.h"
#include "Utils/Utils.h"
#include "Utils/FileUtils.h"

#include <QDir>
#include <QFileDialog>

#include <algorithm>

struct GUI_PlaylistChooser::Private
{
	PlaylistChooser*	playlist_chooser=nullptr;

	Private()
	{}

	~Private()
	{
		if(playlist_chooser){
			delete playlist_chooser; playlist_chooser = nullptr;
		}
	}
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
		m->playlist_chooser = new PlaylistChooser();
	}

	setup_parent(this, &ui);

	connect(ui->combo_playlists, combo_activated_int, this, &GUI_PlaylistChooser::playlist_selected);
	connect(m->playlist_chooser, &PlaylistChooser::sig_playlists_changed, this, &GUI_PlaylistChooser::playlists_changed);

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

	if(skeletons.isEmpty())
	{
		ui->combo_playlists->addItem(tr("No playlists found"), -1);
	}

	int cur_idx = std::max(ui->combo_playlists->findText(old_text), 0);
	ui->combo_playlists->setCurrentIndex(cur_idx);
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

