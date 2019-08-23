/* PlaylistView.cpp */

/* Copyright (C) 2011-2019 Lucio Carreras
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

#include "PlaylistView.h"
#include "PlaylistModel.h"
#include "PlaylistDelegate.h"
#include "PlaylistContextMenu.h"

#include "Gui/Utils/PreferenceAction.h"
#include "Gui/Utils/ContextMenu/LibraryContextMenu.h"
#include "Gui/Utils/Widgets/ProgressBar.h"
#include "Gui/Utils/CustomMimeData.h"
#include "Gui/Utils/MimeDataUtils.h"

#include "Utils/Parser/StreamParser.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Set.h"
#include "Utils/Settings/Settings.h"
#include "Utils/FileUtils.h"

#include "Components/Playlist/PlaylistHandler.h"
#include "Components/Playlist/Playlist.h"

#include <QShortcut>
#include <QDropEvent>
#include <QHeaderView>
#include <QScrollBar>
#include <QDrag>
#include <QTimer>
#include <QLabel>

#include <algorithm>

using namespace Gui;

namespace Pl=::Playlist;
using Pl::View;

struct View::Private
{
	PlaylistPtr				playlist;
	Pl::ContextMenu*		context_menu=nullptr;
	Pl::Model*				model=nullptr;
	ProgressBar*			progressbar=nullptr;
	QLabel*					current_file_label=nullptr;

	Private(PlaylistPtr pl, View* parent) :
		playlist(pl),
		model(new Pl::Model(pl, parent))
	{}
};

View::View(PlaylistPtr pl, QWidget* parent) :
	SearchableTableView(parent),
	InfoDialogContainer(),
	Gui::Dragable(this)
{
	m = Pimpl::make<Private>(pl, this);

	auto* playlist_handler = Pl::Handler::instance();

	this->setObjectName("playlist_view" + QString::number(pl->index()));
	this->set_model(m->model);
	this->setItemDelegate(new Pl::Delegate(this));
	this->horizontalHeader()->setMinimumSectionSize(10);

	init_view();

	ListenSetting(Set::PL_ShowNumbers, View::sl_columns_changed);
	ListenSetting(Set::PL_ShowCovers, View::sl_columns_changed);
	ListenSetting(Set::PL_ShowNumbers, View::sl_columns_changed);
	ListenSetting(Set::PL_ShowRating, View::sl_show_rating_changed);

	new QShortcut(QKeySequence(Qt::Key_Backspace), this, SLOT(clear()), nullptr, Qt::WidgetShortcut);
	new QShortcut(QKeySequence(QKeySequence::Delete), this, SLOT(remove_selected_rows()), nullptr, Qt::WidgetShortcut);
	new QShortcut(QKeySequence(Qt::ControlModifier + Qt::Key_Up), this, SLOT(move_selected_rows_up()), nullptr, Qt::WidgetShortcut);
	new QShortcut(QKeySequence(Qt::ControlModifier + Qt::Key_Down), this, SLOT(move_selected_rows_down()), nullptr, Qt::WidgetShortcut);
	new QShortcut(QKeySequence(Qt::Key_Return), this, SLOT(play_selected_track()), nullptr, Qt::WidgetShortcut);
	new QShortcut(QKeySequence(Qt::Key_Enter), this, SLOT(play_selected_track()), nullptr, Qt::WidgetShortcut);

	connect(m->model, &Pl::Model::sig_data_ready, this, &View::refresh);
	connect(playlist_handler, &Pl::Handler::sig_current_track_changed, this, &View::current_track_changed);
	connect(pl.get(), &Playlist::sig_busy_changed, this, &View::playlist_busy_changed);
	connect(pl.get(), &Playlist::sig_current_scanned_file_changed, this, &View::current_scanned_file_changed);

	QTimer::singleShot(100, this, [=](){
		this->goto_to_current_track();
	});
}

View::~View() = default;

void View::init_view()
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
}


void View::init_context_menu()
{
	using Pl::ContextMenu;

	if(m->context_menu){
		return;
	}

	m->context_menu = new ContextMenu(this);

	connect(m->context_menu, &ContextMenu::sig_refresh_clicked, m->model, &Pl::Model::refresh_data);
	connect(m->context_menu, &ContextMenu::sig_edit_clicked, this, [=](){ show_edit(); });
	connect(m->context_menu, &ContextMenu::sig_info_clicked, this, [=](){ show_info(); });
	connect(m->context_menu, &ContextMenu::sig_lyrics_clicked, this, [=](){ show_lyrics(); });
	connect(m->context_menu, &ContextMenu::sig_delete_clicked, this, &View::delete_selected_tracks);
	connect(m->context_menu, &ContextMenu::sig_remove_clicked, this, &View::remove_selected_rows);
	connect(m->context_menu, &ContextMenu::sig_clear_clicked, this, &View::clear);
	connect(m->context_menu, &ContextMenu::sig_rating_changed, this, &View::rating_changed);
	connect(m->context_menu, &ContextMenu::sig_jump_to_current_track, this, &View::goto_to_current_track);
	connect(m->context_menu, &ContextMenu::sig_bookmark_pressed, this, &View::bookmark_triggered);
	connect(m->context_menu, &ContextMenu::sig_find_track_triggered, this, &View::find_track_triggered);

	m->context_menu->add_preference_action(new PlaylistPreferenceAction(m->context_menu));
}


void View::goto_row(int row)
{
	row = std::min(row, m->model->rowCount() - 1);
	row = std::max(row, 0);

	ModelIndexRange range = model_indexrange_by_index(row);
	this->scrollTo(range.first);
}

int View::calc_drag_drop_line(QPoint pos)
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

void View::handle_drop(QDropEvent* event)
{
	int row = calc_drag_drop_line(event->pos());
	m->model->set_drag_index(-1);

	const QMimeData* mimedata = event->mimeData();
	if(!mimedata) {
		return;
	}

	bool is_inner_drag_drop = MimeData::is_inner_drag_drop(mimedata, m->playlist->index());
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

	const QList<QUrl> urls = mimedata->urls();
	if(!urls.isEmpty())
	{
		bool www = Util::File::is_www(urls.first().toString());
		if(www)
		{
			QStringList files;
			for(const QUrl& url : urls)
			{
				files << url.toString();
			}

			m->playlist->set_busy(true);

			QString cover_url = MimeData::cover_url(mimedata);

			StreamParser* stream_parser = new StreamParser();
			stream_parser->set_cover_url(cover_url);

			connect(stream_parser, &StreamParser::sig_finished, this, [=](bool success){
				async_drop_finished(success, row);
			});

			stream_parser->parse_streams(files);
		}

		else if(v_md.isEmpty())
		{
			QStringList paths;
			for(const QUrl& url : urls)
			{
				if(url.isLocalFile()){
					paths << url.toLocalFile();
				}
			}

			Handler::instance()->insert_tracks(paths, row+1, m->playlist->index());
		}
	}
}


void View::async_drop_finished(bool success, int async_drop_index)
{
	m->playlist->set_busy(false);

	StreamParser* stream_parser = dynamic_cast<StreamParser*>(sender());

	if(success){
		MetaDataList v_md = stream_parser->metadata();
		m->model->insert_tracks(v_md, async_drop_index+1);
	}

	stream_parser->deleteLater();
}


void View::handle_inner_drag_drop(int row, bool copy)
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


void View::rating_changed(Rating rating)
{
	IndexSet selections = selected_items();
	if(selections.isEmpty()){
		return;
	}

	m->model->change_rating(selected_items(), rating);
}


void View::move_selected_rows_up()
{
	IndexSet selections = selected_items();
	IndexSet new_selections = m->model->move_rows_up(selections);
	select_rows(new_selections);
}

void View::move_selected_rows_down()
{
	IndexSet selections = selected_items();
	IndexSet new_selections = m->model->move_rows_down(selections);
	select_rows(new_selections);
}

void View::play_selected_track()
{
	int min_row = min_selected_item();
	emit sig_double_clicked(min_row);
}

void View::goto_to_current_track()
{
	goto_row(m->model->current_track());
}

void View::find_track_triggered()
{
	int row = this->currentIndex().row();
	if(row >= 0){
		m->playlist->find_track(row);
	}
}

void View::bookmark_triggered(Seconds timestamp)
{
	int row = this->currentIndex().row();
	if(row >= 0){
		emit sig_bookmark_pressed(row, timestamp);
	}
}

void View::remove_selected_rows()
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

void View::delete_selected_tracks()
{
	IndexSet selections = selected_items();
	emit sig_delete_tracks(selections);
}


void View::clear()
{
	clear_selection();
	m->model->clear();
}


MD::Interpretation View::metadata_interpretation() const
{
	return MD::Interpretation::Tracks;
}

MetaDataList View::info_dialog_data() const
{
	return m->model->metadata(selected_items());
}

void View::contextMenuEvent(QContextMenuEvent* e)
{
	if(!m->context_menu){
		init_context_menu();
	}

	QPoint pos = e->globalPos();
	QModelIndex idx = indexAt(e->pos());

	Pl::ContextMenu::Entries entry_mask = 0;
	if(row_count() > 0)	{
		entry_mask = (Pl::ContextMenu::EntryClear | Pl::ContextMenu::EntryRefresh);
	}

	IndexSet selections = selected_items();
	if(selections.size() > 0)
	{
		entry_mask |= Pl::ContextMenu::EntryInfo;
		entry_mask |= Pl::ContextMenu::EntryRemove;
	}

	if(selections.size() == 1)
	{
		entry_mask |= (Pl::ContextMenu::EntryLyrics);
	}

	if(m->model->has_local_media(selections) )
	{
		entry_mask |= Pl::ContextMenu::EntryEdit;
		entry_mask |= Pl::ContextMenu::EntryRating;
		entry_mask |= Pl::ContextMenu::EntryDelete;

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
			entry_mask |= Pl::ContextMenu::EntryBookmarks;
			entry_mask |= Pl::ContextMenu::EntryFindInLibrary;
		}
	}

	if(m->model->current_track() >= 0){
		entry_mask |= Pl::ContextMenu::EntryCurrentTrack;
	}

	m->context_menu->show_actions(entry_mask);
	m->context_menu->exec(pos);

	SearchableTableView::contextMenuEvent(e);
}

void View::mousePressEvent(QMouseEvent* event)
{
	SearchableTableView::mousePressEvent(event);

	if(event->buttons() & Qt::LeftButton){
		drag_pressed(event->pos());
	}

	else if(event->button() & Qt::MiddleButton)
	{
		find_track_triggered();
	}
}

void View::mouseMoveEvent(QMouseEvent* event)
{
	QDrag* drag = drag_moving(event->pos());
	if(drag)
	{
		connect(drag, &QDrag::destroyed, this, [=](){
			drag_released(Dragable::ReleaseReason::Destroyed);
		});
	}
}

QMimeData* View::dragable_mimedata() const
{
	QModelIndexList indexes = selectedIndexes();
	return m->model->mimeData(indexes);
}

void View::mouseDoubleClickEvent(QMouseEvent* event)
{
	SearchableTableView::mouseDoubleClickEvent(event);

	QModelIndex idx = this->indexAt(event->pos());

	if( (idx.flags() & Qt::ItemIsEnabled) &&
		(idx.flags() & Qt::ItemIsSelectable))
	{
		emit sig_double_clicked(idx.row());
	}
}

void View::keyPressEvent(QKeyEvent* event)
{
	event->setAccepted(false);
	SearchableTableView::keyPressEvent(event);
}

void View::dragEnterEvent(QDragEnterEvent* event)
{
	if(this->acceptDrops()){
		event->accept();
	}
}

void View::dragMoveEvent(QDragMoveEvent* event)
{
	QTableView::dragMoveEvent(event);		// needed for autoscroll
	if(this->acceptDrops()){				// needed for dragMove
		event->accept();
	}

	int row = calc_drag_drop_line(event->pos());
	m->model->set_drag_index(row);
}

void View::dragLeaveEvent(QDragLeaveEvent* event)
{
	event->accept();
	m->model->set_drag_index(-1);
}

void View::dropEventFromOutside(QDropEvent* event)
{
	dropEvent(event);
}

void View::playlist_busy_changed(bool b)
{
	this->setDisabled(b);

	if(b)
	{
		if(!m->progressbar) {
			m->progressbar = new ProgressBar(this);
		}

		m->progressbar->show();

		// when the list view is disabled, the focus would automatically
		// jump to the parent widget, which may result in the
		// forward/backward button of the playlist
		m->progressbar->setFocus();

		this->setDragDropMode(QAbstractItemView::NoDragDrop);
		this->setAcceptDrops(false);
	}

	else
	{
		if(m->progressbar) {
			m->progressbar->hide();
		}

		if(m->current_file_label){
			m->current_file_label->hide();
		}

		this->setDragDropMode(QAbstractItemView::DragDrop);
		this->setAcceptDrops(true);
		this->setFocus();
	}
}

void View::current_scanned_file_changed(const QString& current_file)
{
	if(!m->current_file_label)
	{
		m->current_file_label = new QLabel(this);
	}

	int offset_bottom = 3;
	offset_bottom += this->fontMetrics().height();
	if(m->progressbar) {
		offset_bottom += m->progressbar->height() + 2;
	}

	m->current_file_label->setText(current_file);
	m->current_file_label->setGeometry(0, this->height() - offset_bottom, this->width(), this->fontMetrics().height() + 4);
	m->current_file_label->show();
}

void View::dropEvent(QDropEvent* event)
{
	if(!this->acceptDrops()){
		event->ignore();
		return;
	}

	event->accept();
	handle_drop(event);
}


int View::index_by_model_index(const QModelIndex& idx) const
{
	return idx.row();
}

ModelIndexRange View::model_indexrange_by_index(int idx) const
{
	return ModelIndexRange(m->model->index(idx, 0),
						   m->model->index(idx, m->model->columnCount() - 1));
}

bool View::viewportEvent(QEvent* event)
{
	bool success = SearchableTableView::viewportEvent(event);

	if(event->type() == QEvent::Resize) {
		refresh();
	}

	return success;
}


void View::skin_changed()
{
	SearchableTableView::skin_changed();
	refresh();
}

void View::sl_columns_changed()
{
	bool show_numbers = GetSetting(Set::PL_ShowNumbers);
	bool show_covers = GetSetting(Set::PL_ShowCovers);

	horizontalHeader()->setSectionHidden(Pl::Model::ColumnName::TrackNumber, !show_numbers);
	horizontalHeader()->setSectionHidden(Pl::Model::ColumnName::Cover, !show_covers);

	refresh();
}

void View::sl_show_rating_changed()
{
	bool show_rating = GetSetting(Set::PL_ShowRating);
	if(show_rating)
	{
		this->setEditTriggers(QAbstractItemView::SelectedClicked);
	}

	else {
		this->setEditTriggers(QAbstractItemView::NoEditTriggers);
	}

	refresh();
}

void View::refresh()
{
	using CN=Pl::Model::ColumnName;

	QFontMetrics fm = this->fontMetrics();
	int h = std::max(fm.height() + 4, 20);

	bool show_rating = GetSetting(Set::PL_ShowRating);
	if(show_rating){
		h += fm.height();
	}

	for(int i=0; i<m->model->rowCount(); i++)
	{
		if(h != rowHeight(i))
		{
			verticalHeader()->resizeSection(i, h);
		}
	}

	QHeaderView* hh = this->horizontalHeader();
	int viewport_width = viewport()->width();
	int w_time = fm.width("1888:88");

	if(GetSetting(Set::PL_ShowCovers))
	{
		int w_cov = h;
		viewport_width -= w_cov;

		if(hh->sectionSize(CN::Cover != w_cov)) {
			hh->resizeSection(CN::Cover, w_cov);
		}
	}

	if(GetSetting(Set::PL_ShowNumbers))
	{
		int w_tn = fm.width(QString::number(m->model->rowCount() * 100));
		viewport_width -= w_tn;

		if(hh->sectionSize(CN::TrackNumber) != w_tn) {
			hh->resizeSection(CN::TrackNumber, w_tn);
		}
	}

	if(hh->sectionSize(CN::Time) != w_time) {
		hh->resizeSection(CN::Time, w_time);
	}

	if(hh->sectionSize(CN::Description) != viewport_width - w_time) {
		hh->resizeSection(CN::Description, viewport_width - w_time);
	}

	m->model->set_row_height(h);
}

void View::current_track_changed(int track_index, int playlist_index)
{
	if(m->playlist->index() == playlist_index){
		goto_row(track_index);
	}
}
