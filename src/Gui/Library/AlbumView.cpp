/* LibraryViewAlbum.cpp */

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

#include "AlbumView.h"
#include "ItemModel.h"

#include "Gui/Library/AlbumModel.h"
#include "Gui/Library/RatingDelegate.h"
#include "Gui/Library/Utils/DiscPopupMenu.h"
#include "Gui/Library/Header/ColumnIndex.h"
#include "Gui/Library/Header/ColumnHeader.h"
#include "Gui/Utils/ContextMenu/LibraryContextMenu.h"

#include "Components/Tagging/UserTaggingOperations.h"
#include "Components/Library/AbstractLibrary.h"

#include "Utils/Library/MergeData.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Utils.h"
#include "Utils/Set.h"

#include <QHeaderView>
#include <QVBoxLayout>


using namespace Library;

struct AlbumView::Private
{
	AbstractLibrary*		library=nullptr;
	DiscPopupMenu*			discmenu=nullptr;
	QPoint					discmenu_point;
};

AlbumView::AlbumView(QWidget* parent) :
	TableView(parent)
{
	m = Pimpl::make<Private>();

	connect(this, &QTableView::clicked, this, &AlbumView::index_clicked);
}

AlbumView::~AlbumView() = default;

void AlbumView::init_view(AbstractLibrary* library)
{
	m->library = library;

	AlbumModel* album_model = new AlbumModel(this, m->library);
	RatingDelegate* album_delegate = new RatingDelegate(this, static_cast<int>(ColumnIndex::Album::Rating), true);

	this->set_item_model(album_model);
	this->setItemDelegate(album_delegate);

	connect(m->library, &AbstractLibrary::sig_all_albums_loaded, this, &AlbumView::fill);

	ListenSetting(Set::Lib_UseViewClearButton, AlbumView::use_clear_button_changed);
}

AbstractLibrary* AlbumView::library() const
{
	return m->library;
}

ColumnHeaderList AlbumView::column_headers() const
{
	ColumnHeaderList columns;

	columns << std::make_shared<ColumnHeader>(ColumnHeader::Sharp, true, SortOrder::NoSorting, SortOrder::NoSorting, 20);
	columns << std::make_shared<ColumnHeader>(ColumnHeader::Album, false, SortOrder::AlbumNameAsc, SortOrder::AlbumNameDesc, 160, true);
	columns << std::make_shared<ColumnHeader>(ColumnHeader::Duration, true, SortOrder::AlbumDurationAsc, SortOrder::AlbumDurationDesc, 90);
	columns << std::make_shared<ColumnHeader>(ColumnHeader::NumTracks, true, SortOrder::AlbumTracksAsc, SortOrder::AlbumTracksDesc, 80);
	columns << std::make_shared<ColumnHeader>(ColumnHeader::Year, true, SortOrder::AlbumYearAsc, SortOrder::AlbumYearDesc, 50);
	columns << std::make_shared<ColumnHeader>(ColumnHeader::Rating, true, SortOrder::AlbumRatingAsc, SortOrder::AlbumRatingDesc, 85);

	return columns;
}

IntList AlbumView::column_header_sizes() const
{
	return GetSetting(Set::Lib_ColSizeAlbum);
}


void Library::AlbumView::save_column_header_sizes(const IntList& sizes)
{
	SetSetting(Set::Lib_ColSizeAlbum, sizes);
}

BoolList AlbumView::visible_columns() const
{
	return GetSetting(Set::Lib_ColsAlbum);
}

void AlbumView::save_visible_columns(const BoolList& lst)
{
	SetSetting(Set::Lib_ColsAlbum, lst);
}

SortOrder AlbumView::sortorder() const
{
	Sortings so = GetSetting(Set::Lib_Sorting);
	return so.so_albums;
}

void AlbumView::save_sortorder(SortOrder s)
{
	m->library->change_album_sortorder(s);
}

void AlbumView::show_context_menu(const QPoint& p)
{
	delete_discmenu();
	TableView::show_context_menu(p);
}


void AlbumView::index_clicked(const QModelIndex& idx)
{
	if(idx.column() == static_cast<int>(ColumnIndex::Album::MultiDisc))
	{
		QModelIndexList selections = this->selectionModel()->selectedRows();
		if(selections.size() == 1){
			init_discmenu(idx);
			show_discmenu();
		}
	}
}


/* where to show the popup */
void AlbumView::calc_discmenu_point(QModelIndex idx)
{
	QHeaderView* v_header = this->verticalHeader();

	m->discmenu_point = QCursor::pos();

	QRect box = this->geometry();
	box.moveTopLeft(this->parentWidget()->mapToGlobal(box.topLeft()));

	if(!box.contains(m->discmenu_point))
	{
		m->discmenu_point.setX(box.x() + (box.width() * 2) / 3);
		m->discmenu_point.setY(box.y());

		QPoint dmp_tmp = parentWidget()->pos();
		dmp_tmp.setY(dmp_tmp.y() - v_header->sizeHint().height());

		while(idx.row() != indexAt(dmp_tmp).row())
		{
			  dmp_tmp.setY(dmp_tmp.y() + 10);
			  m->discmenu_point.setY(m->discmenu_point.y() + 10);
		}
	}
}

void AlbumView::init_discmenu(QModelIndex idx)
{
	int row = idx.row();
	delete_discmenu();

	if( !idx.isValid() ||
		(row >= model()->rowCount()) ||
		(row < 0) )
	{
		return;
	}

	const Album& album = m->library->albums().at(static_cast<size_t>(row));
	if(album.discnumbers.size() < 2) {
		return;
	}

	calc_discmenu_point(idx);

	m->discmenu = new DiscPopupMenu(this, album.discnumbers);

	connect(m->discmenu, &DiscPopupMenu::sig_disc_pressed, this, &AlbumView::sig_disc_pressed);
}


void AlbumView::delete_discmenu()
{
	if(!m->discmenu) {
		return;
	}

	m->discmenu->hide();
	m->discmenu->close();

	disconnect(m->discmenu, &DiscPopupMenu::sig_disc_pressed, this, &AlbumView::sig_disc_pressed);

	m->discmenu->deleteLater();
	m->discmenu = nullptr;
}


void AlbumView::show_discmenu()
{
	if(!m->discmenu) return;

	m->discmenu->popup(m->discmenu_point);
}


void AlbumView::play_clicked()
{
	TableView::play_clicked();
	m->library->prepare_fetched_tracks_for_playlist(false);
}

void AlbumView::play_new_tab_clicked()
{
	TableView::play_new_tab_clicked();
	m->library->prepare_fetched_tracks_for_playlist(true);
}

void AlbumView::play_next_clicked()
{
	TableView::play_next_clicked();
	m->library->play_next_fetched_tracks();
}

void AlbumView::append_clicked()
{
	TableView::append_clicked();
	m->library->append_fetched_tracks();
}

void AlbumView::selection_changed(const IndexSet& indexes)
{
	TableView::selection_changed(indexes);
	m->library->selected_albums_changed(indexes);
}

void AlbumView::refresh_clicked()
{
	TableView::refresh_clicked();
	m->library->refresh_albums();
}

void AlbumView::run_merge_operation(const MergeData& mergedata)
{
	Tagging::UserOperations* uto = new Tagging::UserOperations(mergedata.library_id(), this);

	connect(uto, &Tagging::UserOperations::sig_finished, uto, &Tagging::UserOperations::deleteLater);

	uto->merge_albums(mergedata.source_ids(), mergedata.target_id());
}

bool AlbumView::is_mergeable() const
{
	return true;
}

MD::Interpretation AlbumView::metadata_interpretation() const
{
	return MD::Interpretation::Albums;
}

void AlbumView::use_clear_button_changed()
{
	bool b = GetSetting(Set::Lib_UseViewClearButton);
	use_clear_button(b);
}

