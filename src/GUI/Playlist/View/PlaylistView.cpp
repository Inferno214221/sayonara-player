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

#include "PlaylistView.h"
#include "GUI/Playlist/BookmarksMenu.h"
#include "GUI/Playlist/Model/PlaylistItemModel.h"
#include "GUI/Playlist/Delegate/PlaylistItemDelegate.h"

#include "GUI/Utils/ContextMenu/LibraryContextMenu.h"
#include "GUI/Utils/CustomMimeData.h"
#include "GUI/Utils/Widgets/ProgressBar.h"
#include "GUI/Utils/MimeDataUtils.h"

#include "Utils/Set.h"
#include "Utils/Parser/StreamParser.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Logger/Logger.h"

#include "Components/Playlist/AbstractPlaylist.h"
#include "Components/Playlist/PlaylistHandler.h"
#include "Components/PlayManager/PlayManager.h"
#include "Components/Tagging/Editor.h"

#include <QScrollBar>
#include <QShortcut>
#include <QDropEvent>
#include <algorithm>

using namespace Gui;

struct PlaylistView::Private
{
    LibraryContextMenu*		rc_menu=nullptr;

    PlaylistItemModel*		model=nullptr;
    PlaylistItemDelegate*	delegate=nullptr;

	Gui::ProgressBar*		progress=nullptr;
    BookmarksMenu*			bookmarks_menu=nullptr;
    QAction*				bookmarks_action=nullptr;

    int						async_drop_index;

    Private(PlaylistPtr pl, PlaylistView* parent) :
        model(new PlaylistItemModel(pl, parent)),
        delegate(new PlaylistItemDelegate(parent)),
        async_drop_index(-1)
    {}
};

PlaylistView::PlaylistView(PlaylistPtr pl, QWidget* parent) :
    SearchableListView(parent),
    InfoDialogContainer(),
    Dragable(this)
{
    m = Pimpl::make<Private>(pl, this);

    this->setModel(m->model);
    this->setSearchModel(m->model);
    this->setItemDelegate(m->delegate);

    this->setDragEnabled(true);
    this->setDropIndicatorShown(true);
    this->setDragDropMode(QAbstractItemView::DragDrop);
    this->setAcceptDrops(true);
    this->setSelectionMode(QAbstractItemView::ExtendedSelection);
    this->setAlternatingRowColors(true);
    this->setMovement(QListView::Free);
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    new QShortcut(QKeySequence(Qt::Key_Backspace), this, SLOT(clear()), nullptr, Qt::WidgetShortcut);
}

PlaylistView::~PlaylistView() {}

void PlaylistView::init_rc_menu()
{
    if(m->rc_menu){
        return;
    }

    m->rc_menu = new LibraryContextMenu(this);
    m->bookmarks_menu = new BookmarksMenu(this);
    m->bookmarks_action = m->rc_menu->addMenu(m->bookmarks_menu);

	connect(m->rc_menu, &LibraryContextMenu::sig_info_clicked, [=](){
        show_info();
    });

	connect(m->rc_menu, &LibraryContextMenu::sig_edit_clicked, [=](){
        show_edit();
    });

	connect(m->rc_menu, &LibraryContextMenu::sig_lyrics_clicked, [=](){
        show_lyrics();
    });

    connect(m->rc_menu, &LibraryContextMenu::sig_delete_clicked, this, &PlaylistView::delete_cur_selected_tracks);
    connect(m->rc_menu, &LibraryContextMenu::sig_remove_clicked, this, &PlaylistView::remove_cur_selected_rows);
    connect(m->rc_menu, &LibraryContextMenu::sig_clear_clicked, this, &PlaylistView::clear);
    connect(m->rc_menu, &LibraryContextMenu::sig_rating_changed, this, &PlaylistView::rating_changed);

	connect(m->bookmarks_menu, &BookmarksMenu::sig_bookmark_pressed, [](uint32_t time){
        PlayManager::instance()->seek_abs_ms(time * 1000);
    });
}

int PlaylistView::row_count() const
{
    return m->model->rowCount();
}

void PlaylistView::goto_row(int row)
{
    row = std::min(row, m->model->rowCount() - 1);
    row = std::max(row, 0);

    this->scrollTo(
        m->model->index(row, 0),
        SearchableListView::EnsureVisible
    );
}

void PlaylistView::clear()
{
    clear_selection();
    m->model->clear();
}

void PlaylistView::selectionChanged ( const QItemSelection& selected, const QItemSelection & deselected )
{
    SearchableListView::selectionChanged(selected, deselected);

    if(!selected.isEmpty())
    {
        goto_row(selected.indexes().first().row());
    }
}

void PlaylistView::scroll_up()
{
    QPoint p(5, 5);

    QModelIndex idx = this->indexAt(p);

    if(idx.row() > 0) {
        goto_row(idx.row() - 1);
    }
}


void PlaylistView::scroll_down()
{
    QPoint p(5, this->y() + this->height() - 5);

    QModelIndex idx = this->indexAt(p);

    if(idx.isValid() && (idx.row() < row_count() - 1)) {
        goto_row(idx.row() + 1);
    }
}


void PlaylistView::remove_cur_selected_rows()
{
    int min_row = get_min_selected_item();

    m->model->remove_rows(get_selected_items());
    clear_selection();

    if(row_count() > 0)
    {
        min_row = std::min(min_row, row_count() - 1);
        select_row(min_row);
    }
}

void PlaylistView::delete_cur_selected_tracks()
{
    IndexSet selections = get_selected_items();
    emit sig_delete_tracks(selections);
}


// remove the black line under the titles
void PlaylistView::clear_drag_drop_lines(int row)
{
    m->delegate->set_drag_index(-1);
    this->update(m->model->index(row));
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
    const QMimeData* mimedata = event->mimeData();

    int row = m->delegate->drag_index();
    clear_drag_drop_lines(row);

    if(!mimedata) {
        return;
    }

	bool inner_drag_drop = Util::MimeData::is_inner_drag_drop(mimedata);
    if(inner_drag_drop)
    {
        bool copy = (event->keyboardModifiers() & Qt::ControlModifier);
        handle_inner_drag_drop(row, copy);
        return;
    }

	MetaDataList v_md = Util::MimeData::get_metadata(mimedata);
    if(!v_md.isEmpty())
    {
        PlaylistHandler* plh = PlaylistHandler::instance();
        plh->insert_tracks(v_md, row+1, plh->get_current_idx());
    }

	QStringList playlists = Util::MimeData::get_playlists(mimedata);
    if(!playlists.isEmpty())
    {
        this->setEnabled(false);
        if(!m->progress) {
			m->progress = new Gui::ProgressBar(this);
        }

        m->progress->show();
        m->async_drop_index = row;

		QString cover_url = Util::MimeData::cover_url(mimedata);

        StreamParser* stream_parser = new StreamParser();
        stream_parser->set_cover_url(cover_url);

        connect(stream_parser, &StreamParser::sig_finished, this, &PlaylistView::async_drop_finished);
        stream_parser->parse_streams(playlists);
    }
}


void PlaylistView::async_drop_finished(bool success)
{
    this->setEnabled(true);
    m->progress->hide();

    PlaylistHandler* plh = PlaylistHandler::instance();
    StreamParser* stream_parser = dynamic_cast<StreamParser*>(sender());

    if(success){
        MetaDataList v_md = stream_parser->get_metadata();
        plh->insert_tracks(v_md, m->async_drop_index+1, plh->get_current_idx());
    }

    sender()->deleteLater();
}


void PlaylistView::handle_inner_drag_drop(int row, bool copy)
{
    IndexSet cur_selected_rows, new_selected_rows;
    int n_lines_before_tgt = 0;

    cur_selected_rows = get_selected_items();

    if( cur_selected_rows.contains(row) ) {
        return;
    }

    if(copy)
    {
        m->model->copy_rows(cur_selected_rows, row + 1);
        emit sig_time_changed();
    }

    else
    {
        m->model->move_rows(cur_selected_rows, row + 1);
        n_lines_before_tgt = std::count_if(cur_selected_rows.begin(), cur_selected_rows.end(), [&row](int sel){
            return sel < row;
        });
    }

    for(int i=row; i<row + cur_selected_rows.count(); i++)
    {
        new_selected_rows.insert(i - n_lines_before_tgt + 1);
    }

    this->select_rows( new_selected_rows );
}


void PlaylistView::fill(PlaylistPtr pl)
{
    int cur_track = pl->current_track_index();
    this->scrollTo(m->model->index(cur_track), SearchableListView::EnsureVisible);
}


void PlaylistView::rating_changed(int rating)
{
    IndexSet selections = get_selected_items();
    if(selections.isEmpty()){
        return;
    }

    int row = selections.first();
    MetaData md( m->model->metadata(row) );
    MetaDataList v_md_old{ md };

    Tagging::Editor* te = new Tagging::Editor(v_md_old);

    md.rating = rating;
    te->update_track(0, md);
    te->commit();

    connect(te, &QThread::finished, te, &Tagging::Editor::deleteLater);
}


MD::Interpretation PlaylistView::metadata_interpretation() const
{
    return MD::Interpretation::Tracks;
}


MetaDataList PlaylistView::info_dialog_data() const
{
    IndexSet selected_rows = get_selected_items();

    return m->model->metadata(selected_rows);
}


int PlaylistView::get_index_by_model_index(const QModelIndex& idx) const
{
    return idx.row();
}

QModelIndex PlaylistView::get_model_index_by_index(int idx) const
{
    return m->model->index(idx);
}


void PlaylistView::contextMenuEvent(QContextMenuEvent* e)
{
    if(!m->rc_menu){
        init_rc_menu();
    }

    QPoint pos = e->globalPos();
    QModelIndex idx = indexAt(e->pos());

    LibraryContexMenuEntries entry_mask = 0;

    if(this->row_count() > 0)
    {
        entry_mask = (LibraryContextMenu::EntryClear);
    }

    IndexSet selections = get_selected_items();
    if(selections.size() > 0)
    {
        entry_mask |=
                (LibraryContextMenu::EntryInfo |
                 LibraryContextMenu::EntryRemove);
    }

    if(selections.size() == 1)
    {
        entry_mask |= (LibraryContextMenu::EntryLyrics);
    }

    if(m->model->has_local_media(selections) )
    {
        entry_mask |= (LibraryContextMenu::EntryEdit);

        if(selections.size() == 1)
        {
            MetaData md = m->model->metadata(selections.first());
            m->rc_menu->set_rating( md.rating );
            entry_mask |= LibraryContextMenu::EntryRating;
        }

        if(selections.size() > 0)
        {
            entry_mask |= LibraryContextMenu::EntryDelete;
        }
    }

    m->bookmarks_action->setVisible(
        (idx.row() == m->model->current_track()) &&
        (idx.row() >= 0) &&
        m->bookmarks_menu->has_bookmarks()
    );

    if((entry_mask > 0) || m->bookmarks_action->isVisible()){
        m->rc_menu->show_actions(entry_mask);
        m->rc_menu->exec(pos);
    }

    SearchableListView::contextMenuEvent(e);
}


void PlaylistView::mousePressEvent(QMouseEvent* event)
{
    SearchableListView::mousePressEvent(event);

    if(event->buttons() & Qt::LeftButton){
        this->drag_pressed(event->pos());
    }
}


void PlaylistView::mouseMoveEvent(QMouseEvent* event)
{
    QDrag* drag = this->drag_moving(event->pos());
    if(drag)
    {
		connect(drag, &QDrag::destroyed, [=](){
            this->drag_released(Dragable::ReleaseReason::Destroyed);
        });
    }
}

QMimeData* PlaylistView::get_mimedata() const
{
    return m->model->custom_mimedata(this->selectedIndexes());
}

void PlaylistView::mouseDoubleClickEvent(QMouseEvent* event)
{
    SearchableListView::mouseDoubleClickEvent(event);

    QModelIndex idx = this->indexAt(event->pos());

    if(idx.isValid()){
        m->model->set_current_track(idx.row());
    }

    if( (idx.flags() & Qt::ItemIsEnabled) &&
        (idx.flags() & Qt::ItemIsSelectable))
    {
        emit sig_double_clicked(idx.row());
    }
}

void PlaylistView::keyPressEvent(QKeyEvent* event)
{
    int key = event->key();
    Qt::KeyboardModifiers modifiers = event->modifiers();

    bool ctrl_pressed = (modifiers & Qt::ControlModifier);

    if((key == Qt::Key_Up || key == Qt::Key_Down))
    {
        IndexSet selections = get_selected_items();
        IndexSet new_selections;

        // move items
        if( ctrl_pressed && !selections.isEmpty() )
        {
            if(key == Qt::Key_Up)
            {
                new_selections = m->model->move_rows_up(selections);
            }

            else // key = Qt::Key_Down
            {
                new_selections = m->model->move_rows_down(selections);
            }
        }

        else if(selections.isEmpty())
        {
            if(key == Qt::Key_Up) {
                new_selections.insert(row_count() - 1);
            }

            else {
                new_selections.insert(0);
            }
        }

        else {
            SearchableListView::keyPressEvent(event);
            return;
        }

        if(!new_selections.empty()){
            select_rows(new_selections);
        }

        return;
    }

    else if(event->matches(QKeySequence::SelectAll))
    {
        select_all();
        return;
    }

    else if(event->matches(QKeySequence::Delete))
    {
        remove_cur_selected_rows();
        return;
    }

    SearchableListView::keyPressEvent(event);

    if(!event->isAccepted() ) {
        return;
    }

    int new_row = -1;
    int min_row = get_min_selected_item();

    switch(key)
    {
        case Qt::Key_End:
            new_row = row_count() - 1;
            break;

        case Qt::Key_Home:
            new_row = 0;
            break;

        case Qt::Key_Left:
            if(ctrl_pressed) {
                emit sig_left_tab_clicked();
            }

            break;

        case Qt::Key_Right:
            if(ctrl_pressed) {
                emit sig_right_tab_clicked();
            }

            break;

        case Qt::Key_Return:
        case Qt::Key_Enter:

            if(min_row >= 0)
            {
                m->model->set_current_track(min_row);
                emit sig_double_clicked(min_row);
            }

            break;

        default:
            break;
    }

    if(new_row >= 0) {
        goto_row(new_row);
    }
}



void PlaylistView::dragEnterEvent(QDragEnterEvent* event)
{
    event->accept();
}

void PlaylistView::dragMoveEvent(QDragMoveEvent* event)
{
    event->accept();

    int first_row = this->indexAt(QPoint(5, 5)).row();
    int last_row = this->indexAt(QPoint(5, this->height())).row() - 1;
    int row = calc_drag_drop_line(event->pos() );

    bool is_old = m->delegate->is_drag_index(row);
    if(!is_old)
    {
        clear_drag_drop_lines(m->delegate->drag_index());
        m->delegate->set_drag_index(row);
        this->update(m->model->index(row));
    }

    if(row == first_row){
        scroll_up();
    }
    if(row == last_row){
        scroll_down();
    }
}

void PlaylistView::dragLeaveEvent(QDragLeaveEvent* event)
{
    event->accept();
    clear_drag_drop_lines(m->delegate->drag_index());
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
