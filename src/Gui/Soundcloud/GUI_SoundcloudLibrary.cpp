/* GUI_SoundCloudLibrary.cpp */

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

#include "GUI_SoundcloudLibrary.h"
#include "Gui/Soundcloud/ui_GUI_SoundcloudLibrary.h"
#include "Gui/Soundcloud/GUI_SoundcloudArtistSearch.h"
#include "Gui/Utils/ContextMenu/LibraryContextMenu.h"
#include "Components/Streaming/Soundcloud/SoundcloudLibrary.h"

#include "Utils/Settings/Settings.h"

#include <QShortcut>
#include <QMenu>
#include <QAction>

using SC::GUI_ArtistSearch;

struct SC::GUI_Library::Private
{
	GUI_ArtistSearch*	artist_search=nullptr;
	QMenu*              library_menu=nullptr;
};

using SC::GUI_Library;

GUI_Library::GUI_Library(Library* library, QWidget *parent) :
	GUI_AbstractLibrary(library, parent)
{
	setup_parent(this, &ui);
	setAcceptDrops(false);

	this->setFocusProxy(ui->le_search);

	m = Pimpl::make<GUI_Library::Private>();

	m->artist_search = new GUI_ArtistSearch(library, this);
	m->library_menu = new QMenu(this);

	QAction* action_add_artist = m->library_menu->addAction(tr("Add artist"));
	connect(action_add_artist, &QAction::triggered, this, &GUI_Library::btn_add_clicked);

	library->load();
}

GUI_Library::~GUI_Library()
{
	if(ui)
	{
		delete ui; ui = nullptr;
	}
}


QMenu* GUI_Library::get_menu() const
{
	return m->library_menu;
}

QFrame* GUI_Library::header_frame() const
{
	return ui->header_frame;
}

QList<::Library::Filter::Mode> GUI_Library::search_options() const
{
	return {::Library::Filter::Fulltext};
}

Library::TrackDeletionMode GUI_Library::show_delete_dialog(int n_tracks)
{
	Q_UNUSED(n_tracks)
	return ::Library::TrackDeletionMode::OnlyLibrary;
}

void GUI_Library::btn_add_clicked()
{
	m->artist_search->show();
}

Library::TableView* GUI_Library::lv_artist() const
{
	return ui->tv_artists;
}

Library::TableView* GUI_Library::lv_album() const
{
	return ui->tv_albums;
}

Library::TableView* GUI_Library::lv_tracks() const
{
	return ui->tv_tracks;
}

Library::SearchBar* GUI_Library::le_search() const
{
	return ui->le_search;
}


void GUI_Library::showEvent(QShowEvent *e)
{
	GUI_AbstractLibrary::showEvent(e);

	this->lv_album()->resizeRowsToContents();
	this->lv_artist()->resizeRowsToContents();
	this->lv_tracks()->resizeRowsToContents();

	ui->splitter_artists->restoreState(GetSetting(Set::Lib_SplitterStateArtist));
	ui->splitter_tracks->restoreState(GetSetting(Set::Lib_SplitterStateTrack));
}

