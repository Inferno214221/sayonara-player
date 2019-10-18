/* ArtistView.cpp */

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



#include "ArtistView.h"
#include "ItemModel.h"

#include "Components/Library/AbstractLibrary.h"
#include "Components/Tagging/UserTaggingOperations.h"

#include "Gui/Library/ArtistModel.h"
#include "Gui/Library/Header/ColumnIndex.h"
#include "Gui/Library/Header/ColumnHeader.h"

#include "Gui/Utils/Delegates/StyledItemDelegate.h"
#include "Gui/Utils/ContextMenu/LibraryContextMenu.h"
#include "Gui/Utils/Shortcuts/ShortcutHandler.h"
#include "Gui/Utils/Shortcuts/Shortcut.h"

#include "Utils/Settings/Settings.h"
#include "Utils/Library/Sorting.h"
#include "Utils/Library/MergeData.h"
#include "Utils/Language/Language.h"
#include "Utils/Set.h"


template<typename T>
auto check_vector_size(const T& t) -> T
{
	T copy = t;
	if(copy.size() > 2)
	{
		copy.removeFirst();
	}

	return copy;
}

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

ArtistView::~ArtistView() = default;

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

	connect(m->library, &AbstractLibrary::sig_all_artists_loaded, this, &ArtistView::fill);

	ListenSetting(Set::Lib_UseViewClearButton, ArtistView::use_clear_button_changed);
}

void ArtistView::init_context_menu()
{
	ShortcutHandler* sch = ShortcutHandler::instance();

	ItemView::init_context_menu();

	Library::ContextMenu* menu = context_menu();

	m->album_artist_action = new QAction(menu);
	m->album_artist_action->setCheckable(true);
	m->album_artist_action->setChecked(GetSetting(Set::Lib_ShowAlbumArtists));
	m->album_artist_action->setShortcut(sch->shortcut(ShortcutIdentifier::AlbumArtists).sequence());

	ListenSetting(Set::Lib_ShowAlbumCovers, ArtistView::album_artists_changed);

	connect(m->album_artist_action, &QAction::triggered, this, &ArtistView::album_artists_triggered);

	QAction* action = menu->get_action(Library::ContextMenu::EntryCoverView);
	menu->insertAction(action, m->album_artist_action);

	language_changed();
}


ColumnHeaderList ArtistView::column_headers() const
{
	ColumnHeaderList columns;

	columns << std::make_shared<ColumnHeader>(ColumnHeader::Sharp, true, SortOrder::NoSorting, SortOrder::NoSorting, 20);
	columns << std::make_shared<ColumnHeader>(ColumnHeader::Artist, false, SortOrder::ArtistNameAsc, SortOrder::ArtistNameDesc, 160, true);
	columns << std::make_shared<ColumnHeader>(ColumnHeader::NumTracks, true, SortOrder::ArtistTrackcountAsc, SortOrder::ArtistTrackcountDesc, 80);

	return check_vector_size(columns);
}

QByteArray ArtistView::column_header_state() const
{
	return GetSetting(Set::Lib_ColStateArtists);
}

void ArtistView::save_column_header_state(const QByteArray& state)
{
	SetSetting(Set::Lib_ColStateArtists, state);
}

SortOrder ArtistView::sortorder() const
{
	Library::Sortings so = GetSetting(Set::Lib_Sorting);
	return so.so_artists;
}

void ArtistView::apply_sortorder(SortOrder s)
{
	m->library->change_artist_sortorder(s);
}

void ArtistView::language_changed()
{
	TableView::language_changed();

	if(m->album_artist_action)
	{
		ShortcutHandler* sch = ShortcutHandler::instance();
		m->album_artist_action->setText(Lang::get(Lang::ShowAlbumArtists));
		m->album_artist_action->setShortcut(sch->shortcut(ShortcutIdentifier::AlbumArtists).sequence());
	}
}

bool ArtistView::is_mergeable() const
{
	return true;
}

MD::Interpretation ArtistView::metadata_interpretation() const
{
	return MD::Interpretation::Artists;
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

void ArtistView::use_clear_button_changed()
{
	bool b = GetSetting(Set::Lib_UseViewClearButton);
	use_clear_button(b);
}

void ArtistView::album_artists_triggered(bool b)
{
	Q_UNUSED(b)
	SetSetting(Set::Lib_ShowAlbumArtists, m->album_artist_action->isChecked());
}

void ArtistView::run_merge_operation(const Library::MergeData& mergedata)
{
	Tagging::UserOperations* uto = new Tagging::UserOperations(mergedata.library_id(), this);

	connect(uto, &Tagging::UserOperations::sig_finished, uto, &Tagging::UserOperations::deleteLater);

	uto->merge_artists(mergedata.source_ids(), mergedata.target_id());
}


void ArtistView::album_artists_changed()
{
	m->album_artist_action->setChecked(GetSetting(Set::Lib_ShowAlbumArtists));
}
