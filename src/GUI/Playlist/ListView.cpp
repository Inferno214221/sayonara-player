/* PlaylistView.cpp */

/* Copyright (C) 2011-2017 Lucio Carreras
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


/*
 * PlaylistView.cpp
 *
 *  Created on: Jun 26, 2011
 *      Author: Lucio Carreras
 */

#include "ListView.h"
#include "Model.h"
#include "Delegate.h"
#include "PlaylistContextMenu.h"

#include "GUI/Utils/PreferenceAction.h"
#include "GUI/Utils/ContextMenu/LibraryContextMenu.h"
#include "GUI/Utils/Widgets/ProgressBar.h"
#include "GUI/Utils/CustomMimeData.h"
#include "GUI/Utils/MimeDataUtils.h"

#include "Utils/Parser/StreamParser.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Set.h"
#include "Utils/Settings/Settings.h"
#include "Components/Playlist/AbstractPlaylist.h"

#include <QShortcut>
#include <QDropEvent>
#include <QHeaderView>
#include <QScrollBar>

#include <algorithm>

using namespace Gui;

struct PlaylistView::Private
{
	int						playlist_index;

	PlaylistContextMenu*	context_menu=nullptr;
	PlaylistItemModel*		model=nullptr;
	ProgressBar*			progressbar=nullptr;

	Private(PlaylistPtr pl, PlaylistView* parent) :
		playlist_index(pl->index()),
		model(new PlaylistItemModel(pl, parent))
	{}
};

PlaylistView::PlaylistView(PlaylistPtr pl, QWidget* parent) :
	SearchableTableView(parent),
	InfoDialogContainer(),
	Dragable(this)
{
	m = Pimpl::make<Private>(pl, this);

	this->setObjectName("playlist_view" + QString::number(pl->index()));
	this->set_model(m->model);
	this->setItemDelegate(new PlaylistItemDelegate(this));

	init_view();

	connect(m->model, &PlaylistItemModel::sig_data_ready, this, &PlaylistView::refresh);
	connect(pl.get(), &Playlist::Base::sig_current_track_changed, this, &PlaylistView::goto_row);

	Set::listen<Set::PL_ShowNumbers>(this, &PlaylistView::sl_columns_changed);
	Set::listen<Set::PL_ShowCovers>(this, &PlaylistView::sl_columns_changed);
	Set::listen<Set::PL_ShowNumbers>(this, &PlaylistView::sl_columns_changed);
	Set::listen<Set::PL_ShowRating>(this, &PlaylistView::refresh);

	new QShortcut(QKeySequence(Qt::Key_Backspace), this, SLOT(clear()), nullptr, Qt::WidgetShortcut);
	new QShortcut(QKeySequence(QKeySequence::Delete), this, SLOT(remove_selected_rows()), nullptr, Qt::WidgetShortcut);
	new QShortcut(QKeySequence(Qt::ControlModifier + Qt::Key_Up), this, SLOT(move_selected_rows_up()), nullptr, Qt::WidgetShortcut);
	new QShortcut(QKeySequence(Qt::ControlModifier + Qt::Key_Down), this, SLOT(move_selected_rows_down()), nullptr, Qt::WidgetShortcut);
	new QShortcut(QKeySequence(Qt::Key_Return), this, SLOT(play_selected_track()), nullptr, Qt::WidgetShortcut);
	new QShortcut(QKeySequence(Qt::Key_Enter), this, SLOT(play_selected_track()), nullptr, Qt::WidgetShortcut);

	this->goto_row(pl->current_track_index());
}

PlaylistView::~PlaylistView() {}

void PlaylistView::init_view()
{
	setSelectionMode(QAbstractItemView::ExtendedSelection);
	setAlternatingRowColors(true);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	setSelectionBehavior(QAbstractItemView::SelectRows);
	setShowGrid(false);
	setAutoScroll(true);
	setAutoScrollMargin(50);

	verticalHeader()->hide();
	verticalHeader()->setMinimumSectionSize(1);
	horizontalHeader()->hide();
	horizontalHeader()->setMinimumSectionSize(0);

	setDragEnabled(true);
	setDragDropMode(QAbstractItemView::DragDrop);
	setDragDropOverwriteMode(false);
	setAcceptDrops(true);
	setDropIndicatorShown(true);

	setEditTriggers(QAbstractItemView::SelectedClicked);
}


void PlaylistView::init_context_menu()
{
	if(m->context_menu){
		return;
	}

	m->context_menu = new PlaylistContextMenu(this);

	connect(m->context_menu, &LibraryContextMenu::sig_edit_clicked, this, [=](){ show_edit(); });
	connect(m->context_menu, &LibraryContextMenu::sig_info_clicked, this, [=](){ show_info(); });
	connect(m->context_menu, &LibraryContextMenu::sig_lyrics_clicked, this, [=](){ show_lyrics(); });
	connect(m->context_menu, &PlaylistContextMenu::sig_delete_clicked, this, &PlaylistView::delete_selected_tracks);
	connect(m->context_menu, &PlaylistContextMenu::sig_remove_clicked, this, &PlaylistView::remove_selected_rows);
	connect(m->context_menu, &PlaylistContextMenu::sig_clear_clicked, this, &PlaylistView::clear);
	connect(m->context_menu, &PlaylistContextMenu::sig_rating_changed, this, &PlaylistView::rating_changed);
	connect(m->context_menu, &PlaylistContextMenu::sig_bookmark_pressed, this, [=](Seconds timestamp){
		IndexSet idxs = this->selected_items();
		if(idxs.size() > 0){
			emit sig_bookmark_pressed(idxs.first(), timestamp);
		}
	});

	m->context_menu->add_preference_action(new PlaylistPreferenceAction(m->context_menu));
}


void PlaylistView::goto_row(int row)
{
	row = std::min(row, m->model->rowCount() - 1);
	row = std::max(row, 0);

	ModelIndexRange range = model_indexrange_by_index(row);
	this->scrollTo(range.first);
}

int PlaylistView::calc_drag_drop_line(QPoint pos)
{
	if(pos.y() < 0) {
		return -1;
	}

	int row = this->indexAt(pos).row();

	if(row < 0) {
		row = row_count() - 1;
	}

	return row;
}

void PlaylistView::handle_drop(QDropEvent* event)
{
	int row = calc_drag_drop_line(event->pos());
	m->model->set_drag_index(-1);

	const QMimeData* mimedata = event->mimeData();
	if(!mimedata) {
		return;
	}

	bool is_inner_drag_drop = MimeData::is_inner_drag_drop(mimedata, m->playlist_index);
	if(is_inner_drag_drop)
	{
		bool copy = (event->keyboardModifiers() & Qt::ControlModifier);
		handle_inner_drag_drop(row, copy);
		return;
	}

	MetaDataList v_md = MimeData::metadata(mimedata);
	if(!v_md.isEmpty())
	{
		m->model->insert_tracks(v_md, row+1);
	}

	QStringList playlists = MimeData::playlists(mimedata);
	if(!playlists.isEmpty())
	{
		this->setEnabled(false);
		if(!m->progressbar) {
			m->progressbar = new ProgressBar(this);
		}

		m->progressbar->show();

		QString cover_url = MimeData::cover_url(mimedata);

		StreamParser* stream_parser = new StreamParser();
		stream_parser->set_cover_url(cover_url);

		connect(stream_parser, &StreamParser::sig_finished, this, [=](bool success){
			async_drop_finished(success, row);
		});

		stream_parser->parse_streams(playlists);
	}
}


void PlaylistView::async_drop_finished(bool success, int async_drop_index)
{
	this->setEnabled(true);
	m->progressbar->hide();

	StreamParser* stream_parser = dynamic_cast<StreamParser*>(sender());

	if(success){
		MetaDataList v_md = stream_parser->metadata();
		m->model->insert_tracks(v_md, async_drop_index+1);
	}

	stream_parser->deleteLater();
}


void PlaylistView::handle_inner_drag_drop(int row, bool copy)
{
	IndexSet new_selected_rows;
	IndexSet cur_selected_rows = selected_items();
	if( cur_selected_rows.contains(row) ) {
		return;
	}

	if(copy)
	{
		new_selected_rows = m->model->copy_rows(cur_selected_rows, row + 1);
	}

	else
	{
		new_selected_rows = m->model->move_rows(cur_selected_rows, row + 1);
	}

	this->select_rows(new_selected_rows, 0);
}


void PlaylistView::rating_changed(Rating rating)
{
	IndexSet selections = selected_items();
	if(selections.isEmpty()){
		return;
	}

	m->model->change_rating(selected_items(), rating);
}


void PlaylistView::move_selected_rows_up()
{
	IndexSet selections = selected_items();
	IndexSet new_selections = m->model->move_rows_up(selections);
	select_rows(new_selections);
}

void PlaylistView::move_selected_rows_down()
{
	IndexSet selections = selected_items();
	IndexSet new_selections = m->model->move_rows_down(selections);
	select_rows(new_selections);
}

void PlaylistView::play_selected_track()
{
	int min_row = min_selected_item();
	emit sig_double_clicked(min_row);
}

void PlaylistView::remove_selected_rows()
{
	int min_row = min_selected_item();

	m->model->remove_rows(selected_items());
	clear_selection();

	if(row_count() > 0)
	{
		min_row = std::min(min_row, row_count() - 1);
		select_row(min_row);
	}
}

void PlaylistView::delete_selected_tracks()
{
	IndexSet selections = selected_items();
	emit sig_delete_tracks(selections);
}

void PlaylistView::clear()
{
	clear_selection();
	m->model->clear();
}

MD::Interpretation PlaylistView::metadata_interpretation() const
{
	return MD::Interpretation::Tracks;
}

MetaDataList PlaylistView::info_dialog_data() const
{
	return m->model->metadata(selected_items());
}

void PlaylistView::contextMenuEvent(QContextMenuEvent* e)
{
	if(!m->context_menu){
		init_context_menu();
	}

	QPoint pos = e->globalPos();
	QModelIndex idx = indexAt(e->pos());

	LibraryContextMenu::Entries entry_mask = 0;
	if(row_count() > 0)	{
		entry_mask = (LibraryContextMenu::EntryClear);
	}

	IndexSet selections = selected_items();
	if(selections.size() > 0)
	{
		entry_mask |= LibraryContextMenu::EntryInfo;
		entry_mask |= LibraryContextMenu::EntryRemove;
	}

	if(selections.size() == 1)
	{
		entry_mask |= (LibraryContextMenu::EntryLyrics);
	}

	if(m->model->has_local_media(selections) )
	{
		entry_mask |= LibraryContextMenu::EntryEdit;
		entry_mask |= PlaylistContextMenu::EntryRating;
		entry_mask |= LibraryContextMenu::EntryDelete;


		if(selections.size() == 1)
		{
			MetaData md = m->model->metadata(selections.first());
			m->context_menu->set_rating( md.rating );
		}
	}

	if(idx.row() >= 0)
	{
		MetaData md = m->model->metadata(idx.row());
		m->context_menu->set_metadata(md);

		if(md.id >= 0)
		{
			entry_mask |= PlaylistContextMenu::EntryBookmarks;
		}
	}

	m->context_menu->show_actions(entry_mask);
	m->context_menu->exec(pos);

	SearchableTableView::contextMenuEvent(e);
}

void PlaylistView::mousePressEvent(QMouseEvent* event)
{
	SearchableTableView::mousePressEvent(event);

	if(event->buttons() & Qt::LeftButton){
		drag_pressed(event->pos());
	}
}

void PlaylistView::mouseMoveEvent(QMouseEvent* event)
{
	QDrag* drag = drag_moving(event->pos());
	if(drag)
	{
		connect(drag, &QDrag::destroyed, this, [=](){
			drag_released(Dragable::ReleaseReason::Destroyed);
		});
	}
}

QMimeData* PlaylistView::dragable_mimedata() const
{
	return m->model->mimeData( selectedIndexes() );
}

void PlaylistView::mouseDoubleClickEvent(QMouseEvent* event)
{
	SearchableTableView::mouseDoubleClickEvent(event);

	QModelIndex idx = this->indexAt(event->pos());

	if( (idx.flags() & Qt::ItemIsEnabled) &&
		(idx.flags() & Qt::ItemIsSelectable))
	{
		emit sig_double_clicked(idx.row());
	}
}

void PlaylistView::keyPressEvent(QKeyEvent* event)
{
	event->setAccepted(false);
	SearchableTableView::keyPressEvent(event);
}

void PlaylistView::dragEnterEvent(QDragEnterEvent* event)
{
	event->accept();
}

void PlaylistView::dragMoveEvent(QDragMoveEvent* event)
{
	QTableView::dragMoveEvent(event);		// needed for autoscroll
	event->accept();						// needed for dragMove

	int row = calc_drag_drop_line(event->pos());
	m->model->set_drag_index(row);
}

void PlaylistView::dragLeaveEvent(QDragLeaveEvent* event)
{
	event->accept();
	m->model->set_drag_index(-1);
}

void PlaylistView::dropEventFromOutside(QDropEvent* event)
{
	dropEvent(event);
}

void PlaylistView::dropEvent(QDropEvent* event)
{
	event->accept();
	handle_drop(event);
}


int PlaylistView::index_by_model_index(const QModelIndex& idx) const
{
	return idx.row();
}

ModelIndexRange PlaylistView::model_indexrange_by_index(int idx) const
{
	return ModelIndexRange(m->model->index(idx, 0),
						   m->model->index(idx, m->model->columnCount() - 1));
}

bool PlaylistView::viewportEvent(QEvent* event)
{
	bool success = SearchableTableView::viewportEvent(event);

	if(event->type() == QEvent::Resize)
	{
		refresh();
	}

	return success;
}

void PlaylistView::skin_changed()
{
	refresh();
}

void PlaylistView::sl_columns_changed()
{
	bool show_numbers = _settings->get<Set::PL_ShowNumbers>();
	bool show_covers = _settings->get<Set::PL_ShowCovers>();

	horizontalHeader()->setSectionHidden(PlaylistItemModel::ColumnName::TrackNumber, !show_numbers);
	horizontalHeader()->setSectionHidden(PlaylistItemModel::ColumnName::Cover, !show_covers);

	refresh();
}

void PlaylistView::refresh()
{
	bool show_rating = _settings->get<Set::PL_ShowRating>();
	QFontMetrics fm(this->font());

	int h = std::max(fm.height() + 4, 20);
	if(show_rating){
		h += fm.height();
	}

	for(int i=0; i<m->model->rowCount(); i++) {
		verticalHeader()->resizeSection(i, h);
	}

	QHeaderView* hh = this->horizontalHeader();
	int viewport_width = viewport()->width();
	int w_time = fm.width("1888:88");

	if(_settings->get<Set::PL_ShowCovers>())
	{
		int w_cov = 30;
		viewport_width -= w_cov;
		hh->resizeSection(PlaylistItemModel::ColumnName::Cover, w_cov);
	}

	if(_settings->get<Set::PL_ShowNumbers>())
	{
		int w_tn = fm.width(QString::number(m->model->rowCount() * 100));
		viewport_width -= w_tn;
		hh->resizeSection(PlaylistItemModel::ColumnName::TrackNumber, w_tn);
	}

	hh->resizeSection(PlaylistItemModel::ColumnName::Time, w_time);
	hh->resizeSection(PlaylistItemModel::ColumnName::Description, viewport_width - (w_time));
}
