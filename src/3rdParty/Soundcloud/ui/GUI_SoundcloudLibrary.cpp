/* GUI_SoundCloudLibrary.cpp */

/* Copyright (C) 2011-2016  Lucio Carreras
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


#include "ui/GUI_SoundcloudLibrary.h"
#include "GUI/Helper/ContextMenu/LibraryContextMenu.h"


GUI_SoundCloudLibrary::GUI_SoundCloudLibrary(SoundcloudLibrary* library, QWidget *parent) :
	GUI_AbstractLibrary(library, parent),
	Ui::GUI_SoundcloudLibrary()
{
	setup_parent(this);

	QAction* action_add_artist;

	_artist_search = new GUI_SoundcloudArtistSearch(library, this);
	_library_menu = new QMenu(this);
	action_add_artist = _library_menu->addAction(tr("Add artist"));

	setAcceptDrops(false);

	connect(action_add_artist, &QAction::triggered, this, &GUI_SoundCloudLibrary::btn_add_clicked);

	LibraryContexMenuEntries entry_mask =
			(LibraryContextMenu::EntryPlayNext |
			LibraryContextMenu::EntryInfo |
			LibraryContextMenu::EntryDelete |
			LibraryContextMenu::EntryAppend |
			LibraryContextMenu::EntryRefresh);

	tb_title->set_rc_menu(entry_mask);
	lv_album->set_rc_menu(entry_mask);
	lv_artist->set_rc_menu(entry_mask);

	btn_info->hide();

	library->load();
}


QComboBox* GUI_SoundCloudLibrary::get_libchooser() const
{
	return combo_lib_chooser;
}


QMenu* GUI_SoundCloudLibrary::get_menu() const {
	return _library_menu;
}

AbstractLibrary::TrackDeletionMode GUI_SoundCloudLibrary::show_delete_dialog(int n_tracks){

	Q_UNUSED(n_tracks)
	return AbstractLibrary::TrackDeletionMode::OnlyLibrary;
}

void GUI_SoundCloudLibrary::btn_add_clicked(){
	_artist_search->show();
}




SoundcloudLibraryContainer::SoundcloudLibraryContainer(QObject *parent) :
	LibraryContainerInterface(parent)
{

}

QString SoundcloudLibraryContainer::get_name() const
{
	return "soundcloud";
}

QString SoundcloudLibraryContainer::get_display_name() const
{
	return tr("Soundcloud");
}

QIcon SoundcloudLibraryContainer::get_icon() const
{
	return QIcon(":/sc_icons/ui/icon.png");
}


QWidget* SoundcloudLibraryContainer::get_ui() const
{
	return static_cast<QWidget*>(ui);
}

QComboBox* SoundcloudLibraryContainer::get_libchooser()
{
	if(ui){
		return ui->get_libchooser();
	}

	return nullptr;
}

QMenu*SoundcloudLibraryContainer::get_menu()
{
	if(ui){
		return ui->get_menu();
	}

	return nullptr;
}

void SoundcloudLibraryContainer::init_ui()
{
	SoundcloudLibrary* library = new SoundcloudLibrary(this);
	ui = new GUI_SoundCloudLibrary(library);
}

