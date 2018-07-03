/* TrackView.cpp */

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

#include "TrackView.h"

#include "GUI/Library/TrackModel.h"
#include "GUI/Library/RatingDelegate.h"
#include "GUI/Library/Utils/ColumnHeader.h"
#include "GUI/Library/Utils/ColumnIndex.h"

#include "Components/Library/AbstractLibrary.h"

#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Library/Sortorder.h"
#include "Utils/Library/Sorting.h"

using namespace Library;

struct TrackView::Private
{
	AbstractLibrary* library = nullptr;
};

TrackView::TrackView(QWidget* parent) :
	Library::TableView(parent)
{
	m = Pimpl::make<Private>();
}

TrackView::~TrackView() {}

void TrackView::init_view(AbstractLibrary* library)
{
	m->library = library;

	TrackModel* track_model = new TrackModel(this, library);
	RatingDelegate* track_delegate = new RatingDelegate(this, (int) ColumnIndex::Track::Rating, true);

	this->set_item_model(track_model);
	this->set_search_model(track_model);
	this->setItemDelegate(track_delegate);
	this->set_metadata_interpretation(MD::Interpretation::Tracks);

	connect(this, &ItemView::doubleClicked, this, &TrackView::double_clicked);
	connect(library, &AbstractLibrary::sig_all_tracks_loaded, this, &TrackView::tracks_ready);
}


ColumnHeaderList TrackView::column_headers() const
{
	ColumnHeaderList track_columns;

	ColumnHeader* t_h0 = new ColumnHeader(ColumnHeader::Sharp, true, SortOrder::TrackNumAsc, SortOrder::TrackNumDesc, 25);
	ColumnHeader* t_h1 = new ColumnHeader(ColumnHeader::Title, false, SortOrder::TrackTitleAsc, SortOrder::TrackTitleDesc, 0.4, 200);
	ColumnHeader* t_h2 = new ColumnHeader(ColumnHeader::Artist, true, SortOrder::TrackArtistAsc, SortOrder::TrackArtistDesc, 0.3, 160);
	ColumnHeader* t_h3 = new ColumnHeader(ColumnHeader::Album, true, SortOrder::TrackAlbumAsc, SortOrder::TrackAlbumDesc, 0.3, 160);
	ColumnHeader* t_h4 = new ColumnHeader(ColumnHeader::Year, true, SortOrder::TrackYearAsc, SortOrder::TrackYearDesc, 50);
	ColumnHeader* t_h5 = new ColumnHeader(ColumnHeader::DurationShort, true, SortOrder::TrackLenghtAsc, SortOrder::TrackLengthDesc, 50);
	ColumnHeader* t_h6 = new ColumnHeader(ColumnHeader::Bitrate, true, SortOrder::TrackBitrateAsc, SortOrder::TrackBitrateDesc, 75);
	ColumnHeader* t_h7 = new ColumnHeader(ColumnHeader::Filesize, true, SortOrder::TrackSizeAsc, SortOrder::TrackSizeDesc, 75);
	ColumnHeader* t_h8 = new ColumnHeader(ColumnHeader::Rating, true, SortOrder::TrackRatingAsc, SortOrder::TrackRatingDesc, 80);

	track_columns  << t_h0  << t_h1  << t_h2  << t_h3  <<t_h4  << t_h5  << t_h6 << t_h7 << t_h8;

	return track_columns;
}

BoolList TrackView::visible_columns() const
{
	return _settings->get<Set::Lib_ColsTitle>();
}

void TrackView::save_visible_columns(const BoolList& lst)
{
	_settings->set<Set::Lib_ColsTitle>(lst);
}

SortOrder TrackView::sortorder() const
{
	Sortings so = _settings->get<Set::Lib_Sorting>();
	return so.so_tracks;
}

void TrackView::save_sortorder(SortOrder s)
{
	m->library->change_track_sortorder(s);
}

void TrackView::selection_changed(const IndexSet& lst)
{
	TableView::selection_changed(lst);
	m->library->selected_tracks_changed(lst);
}

void TrackView::double_clicked(const QModelIndex& idx)
{
	Q_UNUSED(idx)

	m->library->prepare_current_tracks_for_playlist(false);
}

void TrackView::middle_clicked()
{
	TableView::middle_clicked();
	m->library->prepare_current_tracks_for_playlist(true);
}

void TrackView::play_clicked()
{
	double_clicked(QModelIndex());
}

void TrackView::play_new_tab_clicked()
{
	TableView::play_new_tab_clicked();
	m->library->prepare_current_tracks_for_playlist(true);
}


void TrackView::play_next_clicked()
{
	TableView::play_next_clicked();
	m->library->play_next_current_tracks();
}

void TrackView::append_clicked()
{
	TableView::append_clicked();
	m->library->append_current_tracks();
}

void TrackView::refresh_clicked()
{
	TableView::refresh_clicked();
	m->library->refresh_tracks();
}

void TrackView::tracks_ready()
{
	const MetaDataList& v_md = m->library->tracks();

	this->fill<MetaDataList, TrackModel>(v_md);
}
