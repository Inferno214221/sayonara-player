/* ArtistView.cpp */

/* Copyright (C) 2011-2017  Lucio Carreras
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



#include "ArtistView.h"

#include "Components/Library/AbstractLibrary.h"
#include "GUI/Library/ArtistModel.h"
#include "GUI/Library/Utils/ColumnIndex.h"
#include "GUI/Library/Utils/ColumnHeader.h"

#include "GUI/Utils/Delegates/StyledItemDelegate.h"
#include "GUI/Utils/ContextMenu/LibraryContextMenu.h"

#include "Utils/Settings/Settings.h"
#include "Utils/Library/Sorting.h"
#include "Utils/Language.h"


using namespace Library;

struct ArtistView::Private
{
	AbstractLibrary*	library=nullptr;
	QAction*			album_artist_action=nullptr;
};

ArtistView::ArtistView(QWidget* parent) :
	Library::TableView(parent)
{
	m = Pimpl::make<Private>();
}

ArtistView::~ArtistView() {}

AbstractLibrary* ArtistView::library() const
{
	return m->library;
}

void ArtistView::init_view(AbstractLibrary* library)
{
	m->library = library;

	ArtistModel* artist_model = new ArtistModel(this, m->library);

	this->set_item_model(artist_model);
	this->setItemDelegate(new Gui::StyledItemDelegate(this));
	this->set_metadata_interpretation(MD::Interpretation::Artists);

	connect(m->library, &AbstractLibrary::sig_all_artists_loaded, this, &ArtistView::artists_ready);

	Set::listen<Set::Lib_UseViewClearButton>(this, &ArtistView::use_clear_button_changed);
}

void ArtistView::init_context_menu()
{
	ItemView::init_context_menu();
	LibraryContextMenu* menu = context_menu();

	m->album_artist_action = new QAction(menu);
	m->album_artist_action->setCheckable(true);
	m->album_artist_action->setChecked(_settings->get<Set::Lib_ShowAlbumArtists>());
	Set::listen<Set::Lib_ShowAlbumCovers>(this, &ArtistView::album_artists_changed);

	connect(m->album_artist_action, &QAction::triggered, this, &ArtistView::album_artists_triggered);

	QAction* action = menu->get_action(LibraryContextMenu::EntryCoverView);
	menu->insertAction(action, m->album_artist_action);

	language_changed();
}


ColumnHeaderList ArtistView::column_headers() const
{
	ColumnHeaderList columns;

	columns << std::make_shared<ColumnHeader>(ColumnHeader::Sharp, true, SortOrder::NoSorting, SortOrder::NoSorting, 20);
	columns << std::make_shared<ColumnHeader>(ColumnHeader::Artist, false, SortOrder::ArtistNameAsc, SortOrder::ArtistNameDesc, 1.0, 160);
	columns << std::make_shared<ColumnHeader>(ColumnHeader::NumTracks, true, SortOrder::ArtistTrackcountAsc, SortOrder::ArtistTrackcountDesc, 80);

	return columns;
}

BoolList ArtistView::visible_columns() const
{
	BoolList columns = _settings->get<Set::Lib_ColsArtist>();
	columns[0] = false;
	return columns;
}

void ArtistView::save_visible_columns(const BoolList& columns)
{
	_settings->set<Set::Lib_ColsArtist>(columns);
}

SortOrder ArtistView::sortorder() const
{
	Library::Sortings so = _settings->get<Set::Lib_Sorting>();
	return so.so_artists;
}

void ArtistView::save_sortorder(SortOrder s)
{
	m->library->change_artist_sortorder(s);
}

void ArtistView::language_changed()
{
	TableView::language_changed();

	if(m->album_artist_action){
		m->album_artist_action->setText(Lang::get(Lang::ShowAlbumArtists));
	}
}

void ArtistView::selection_changed(const IndexSet& indexes)
{
	TableView::selection_changed(indexes);
	m->library->selected_artists_changed(indexes);
}


void ArtistView::play_clicked()
{
	TableView::play_clicked();
	m->library->prepare_fetched_tracks_for_playlist(false);
}

void ArtistView::play_new_tab_clicked()
{
	TableView::play_new_tab_clicked();
	m->library->prepare_fetched_tracks_for_playlist(true);
}

void ArtistView::play_next_clicked()
{
	TableView::play_next_clicked();
	m->library->play_next_fetched_tracks();
}

void ArtistView::append_clicked()
{
	TableView::append_clicked();
	m->library->append_fetched_tracks();
}

void ArtistView::refresh_clicked()
{
	TableView::refresh_clicked();
	m->library->refresh_artist();
}

void ArtistView::artists_ready()
{
	const ArtistList& artists = m->library->artists();
	this->fill<ArtistList, ArtistModel>(artists);
}

void ArtistView::use_clear_button_changed()
{
	bool b = _settings->get<Set::Lib_UseViewClearButton>();
	use_clear_button(b);
}

void ArtistView::album_artists_triggered(bool b)
{
	Q_UNUSED(b)
	_settings->set<Set::Lib_ShowAlbumArtists>(m->album_artist_action->isChecked());
}

void ArtistView::run_merge_operation(const ItemView::MergeData& mergedata)
{
	m->library->merge_artists(mergedata.source_ids, mergedata.target_id);
}

void ArtistView::album_artists_changed()
{
	m->album_artist_action->setChecked(_settings->get<Set::Lib_ShowAlbumArtists>());
}
