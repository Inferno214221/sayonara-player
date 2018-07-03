/* LibraryView.cpp */

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
 *  Created on: Jun 26, 2011
 *      Author: Lucio Carreras
 */

#include "ItemView.h"
#include "HeaderView.h"
#include "Components/Covers/CoverLocation.h"
#include "GUI/Library/Utils/ColumnHeader.h"

#include "Utils/globals.h"
#include "Utils/Settings/Settings.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Set.h"
#include "Utils/Logger/Logger.h"

#include "GUI/Utils/ContextMenu/LibraryContextMenu.h"
#include "GUI/Utils/SearchableWidget/MiniSearcher.h"
#include "GUI/Utils/CustomMimeData.h"
#include "GUI/Utils/PreferenceAction.h"

#include <QHeaderView>
#include <QDropEvent>
#include <QMouseEvent>
#include <QPushButton>
#include <QScrollBar>
#include <QDrag>
#include <QBoxLayout>

using namespace Library;

struct Library::ItemView::Private
{
	QPushButton*		btn_clear_selection=nullptr;
	QAction*			merge_action=nullptr;
	QMenu*				merge_menu=nullptr;
	LibraryContextMenu*	context_menu=nullptr;

	MD::Interpretation	type;
	bool				cur_filling;
	bool				use_clear_button;

	Private() :
		type(MD::Interpretation::None),
		cur_filling(false),
		use_clear_button(false)
	{}
};

ItemView::ItemView(QWidget* parent) :
	WidgetTemplate<SearchableTableView>(parent),
	InfoDialogContainer(),
	Dragable(this)
{
	_model = nullptr;
	m = Pimpl::make<Private>();

	this->setAcceptDrops(true);
	this->setSelectionBehavior(QAbstractItemView::SelectRows);
	this->setAlternatingRowColors(true);
	this->setDragEnabled(true);

	QHeaderView* vertical_header = this->verticalHeader();
	if(vertical_header) {
		vertical_header->setResizeContentsPrecision(2);
	}

	clearSelection();
}

ItemView::~ItemView() {}

void ItemView::set_item_model(ItemModel* model)
{
	SearchableTableView::setModel(model);
	_model = model;
}

void ItemView::selectionChanged(const QItemSelection& selected, const QItemSelection& deselected )
{
	show_clear_button(!selected.empty());

	if(m->cur_filling) {
		return;
	}

	SearchableTableView::selectionChanged(selected, deselected);
	IndexSet indexes = selected_items();

	if(m->context_menu){
		m->context_menu->show_action(LibraryContextMenu::EntryClearSelection, !selected.isEmpty());
	}

	selection_changed(indexes);
}


// selections end


// Right click stuff
void ItemView::init_context_menu()
{
	m->context_menu = new LibraryContextMenu(this);

	m->merge_menu = new QMenu(tr("Merge"), m->context_menu);
	m->merge_action = m->context_menu->addMenu(m->merge_menu);
	m->merge_action->setVisible(false);

	connect(m->context_menu, &LibraryContextMenu::sig_edit_clicked, this, [=](){
		show_edit();
	});

	connect(m->context_menu, &LibraryContextMenu::sig_info_clicked, this, [=](){
		show_info();
	});

	connect(m->context_menu, &LibraryContextMenu::sig_lyrics_clicked, this, [=](){
		show_lyrics();
	});

	connect(m->context_menu, &LibraryContextMenu::sig_clear_selection_clicked, this, [=](){
		this->clear_selection();
	});
	connect(m->context_menu, &LibraryContextMenu::sig_delete_clicked, this, &ItemView::delete_clicked);
	connect(m->context_menu, &LibraryContextMenu::sig_play_clicked, this, &ItemView::play_clicked);
	connect(m->context_menu, &LibraryContextMenu::sig_play_next_clicked, this, &ItemView::play_next_clicked);
	connect(m->context_menu, &LibraryContextMenu::sig_play_new_tab_clicked, this, &ItemView::play_new_tab_clicked);
	connect(m->context_menu, &LibraryContextMenu::sig_append_clicked, this, &ItemView::append_clicked);
	connect(m->context_menu, &LibraryContextMenu::sig_refresh_clicked, this, &ItemView::refresh_clicked);

	m->context_menu->add_preference_action(new LibraryPreferenceAction(m->context_menu));
}

LibraryContextMenu* ItemView::context_menu() const
{
	return m->context_menu;
}

void ItemView::show_context_menu_actions(int entries)
{
	if(!m->context_menu) {
		init_context_menu();
	}

	m->context_menu->show_actions(entries);
	m->context_menu->show_action(LibraryContextMenu::EntryClearSelection, !selected_items().isEmpty());
}

void ItemView::add_context_action(QAction *action)
{
	if(!m->context_menu) {
		init_context_menu();
	}

	m->context_menu->addAction(action);
}

void ItemView::remove_context_action(QAction *action)
{
	if(!m->context_menu) {
		init_context_menu();
	}

	m->context_menu->removeAction(action);
}

QMimeData* ItemView::dragable_mimedata() const
{
	return _model->custom_mimedata();
}

QPixmap ItemView::drag_pixmap() const
{
	Cover::Location cl = _model->cover(
				selected_items()
	);

	QString cover_path = cl.preferred_path();
	if(cl.valid()){
		return QPixmap(cover_path);
	}

	return QPixmap();
}

void ItemView::set_selection_type(SelectionViewInterface::SelectionType type)
{
	SelectionViewInterface::set_selection_type(type);

	if(type == SelectionViewInterface::SelectionType::Rows){
		setSelectionBehavior(QAbstractItemView::SelectRows);
	}

	else {
		setSelectionBehavior(QAbstractItemView::SelectColumns);
		this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	}
}


void ItemView::show_clear_button(bool visible)
{
	if(!m->use_clear_button)
	{
		return;
	}

	if(!m->btn_clear_selection)
	{
		m->btn_clear_selection = new QPushButton(this);
		m->btn_clear_selection->setText(tr("Clear selection"));

		connect(m->btn_clear_selection, &QPushButton::clicked, this, [=](){
			this->clearSelection();
			m->btn_clear_selection->hide();
		});
	}

	const int h = 22;

	int y = this->height() - h - 1;
	int w = this->width() - 2;

	if(this->verticalScrollBar() && this->verticalScrollBar()->isVisible())
	{
		w -= this->verticalScrollBar()->width();
	}

	if(this->horizontalScrollBar() && this->horizontalScrollBar()->isVisible())
	{
		y -= this->horizontalScrollBar()->height();
	}

	m->btn_clear_selection->setVisible(visible);
	m->btn_clear_selection->setGeometry(1, y, w, h);

	int mini_searcher_padding = (visible) ? h : 0;
	SearchableTableView::set_mini_searcher_padding(mini_searcher_padding);

}

void ItemView::use_clear_button(bool yesno)
{
	m->use_clear_button = yesno;
	if(m->btn_clear_selection)
	{
		if(!yesno){
			m->btn_clear_selection->hide();
		}
		else{
			m->btn_clear_selection->setVisible(this->selected_items().count() > 0);
		}
	}
}

bool ItemView::is_valid_drag_position(const QPoint &p) const
{
	QModelIndex idx = this->indexAt(p);
	return (idx.isValid() && (this->model()->flags(idx) & Qt::ItemFlag::ItemIsSelectable));
}

void ItemView::context_menu_show(const QPoint& p)
{
	m->context_menu->exec(p);
}


void ItemView::set_metadata_interpretation(MD::Interpretation type)
{
	m->type = type;
}

MetaDataList ItemView::info_dialog_data() const
{
	return _model->mimedata_tracks();
}



MD::Interpretation ItemView::metadata_interpretation() const
{
	return m->type;
}

void ItemView::merge_action_triggered()
{
	QAction* action = dynamic_cast<QAction*>(sender());
	int id = action->data().toInt();

	IndexSet selected_items = this->selected_items();
	SP::Set<Id> ids;

	for(auto idx : selected_items)
	{
		ids.insert( _model->id_by_row(idx) );
	}

	emit sig_merge(ids, id);
}

void ItemView::play_clicked()
{
	emit sig_play_clicked();
}

void ItemView::play_new_tab_clicked()
{
	emit sig_play_new_tab_clicked();
}

void ItemView::play_next_clicked()
{
	emit sig_play_next_clicked();
}

void ItemView::delete_clicked()
{
	emit sig_delete_clicked();
}

void ItemView::middle_clicked()
{
	emit sig_middle_button_clicked();
}

void ItemView::append_clicked()
{
	emit sig_append_clicked();
}

void ItemView::refresh_clicked()
{
	emit sig_refresh_clicked();
}

void ItemView::selection_changed(const IndexSet& indexes)
{
	emit sig_sel_changed(indexes);
}


void ItemView::resize_rows_to_contents()
{
	if(!_model || _model->rowCount() == 0) {
		return;
	}

	QHeaderView* header = this->verticalHeader();
	if(header) {
		header->resizeSections(QHeaderView::ResizeToContents);
	}
}

void ItemView::resize_rows_to_contents(int first_row, int count)
{
	if(!_model || _model->rowCount() == 0) {
		return;
	}

	QHeaderView* header = this->verticalHeader();
	if(header) {
		for(int i=first_row; i<first_row + count; i++)
		{
			this->resizeRowToContents(i);
		}
	}
}


// mouse events
void ItemView::mousePressEvent(QMouseEvent* event)
{
	if(_model->rowCount() == 0)
	{
		return;
	}

	QPoint pos_org = event->pos();

	if(event->button() == Qt::LeftButton){
		this->drag_pressed(pos_org);
	}

	SearchableTableView::mousePressEvent(event);

	if(event->button() == Qt::MidButton) {
		// item has to be marked as selected first,
		// so the signal is emmited after calling
		// SearchableTableView::mousePressEvent(event);
		middle_clicked();
	}
}


void ItemView::mouseMoveEvent(QMouseEvent* event)
{
	QDrag* drag = this->drag_moving(event->pos());
	if(drag)
	{
		connect(drag, &QDrag::destroyed, this, [=]() {
			this->drag_released(Dragable::ReleaseReason::Destroyed);
		});
	}
}
// mouse events end

// keyboard events

void ItemView::keyPressEvent(QKeyEvent* event)
{
	int key = event->key();

	Qt::KeyboardModifiers  modifiers = event->modifiers();

	bool shift_pressed = (modifiers & Qt::ShiftModifier);
	bool alt_pressed = (modifiers & Qt::AltModifier);
	bool ctrl_pressed = (modifiers & Qt::ControlModifier);

	IndexSet selections = selected_items();

	switch(key)
	{
		case Qt::Key_Return:
		case Qt::Key_Enter:
			if(selections.isEmpty() || ctrl_pressed){
				return;
			}

			// standard enter
			if(!shift_pressed && !alt_pressed)
			{
				if(!selections.isEmpty()){
					int first_idx = selections.first();
					emit doubleClicked( _model->index(first_idx, 0));
				}
			}

			// enter with shift
			else if(shift_pressed && !alt_pressed) {
				append_clicked();
			}

			else if(alt_pressed) {
				play_next_clicked();
			}

			return;

		case Qt::Key_Backspace:
			this->clearSelection();
			return;

		default:
			event->setAccepted(false);
			break;
	}

	SearchableTableView::keyPressEvent(event);
}

void ItemView::contextMenuEvent(QContextMenuEvent* event)
{
	if(!m->context_menu)
	{
		init_context_menu();
	}

	IndexSet selections = selected_items();

	QPoint pos = event->globalPos();

	if(m->type == MD::Interpretation::Tracks && selections.size() == 1)
	{
		m->context_menu->show_action(LibraryContextMenu::EntryLyrics, true);
	}
	else {
		m->context_menu->show_action(LibraryContextMenu::EntryLyrics, false);
	}

	bool is_right_type =
			(m->type == MD::Interpretation::Artists ||
			 m->type == MD::Interpretation::Albums);

	if(is_right_type)
	{
		size_t n_selections = selections.size();

		if(n_selections > 1)
		{
			m->merge_menu->clear();

			for(int i : selections)
			{
				int id = _model->id_by_row(i);
				if(id < 0){
					n_selections--;
					if(n_selections <= 1) {
						break;
					}

					continue;
				}

				QString name = _model->searchable_string(i);
				name.replace("&", "&&");

				QAction* action = new QAction(name, m->merge_menu);
				action->setData(id);
				m->merge_menu->addAction(action);
				connect(action, &QAction::triggered, this, &ItemView::merge_action_triggered);
			}

			m->merge_action->setVisible(n_selections > 1);
		}
	}

	context_menu_show(pos);

	QTableView::contextMenuEvent(event);
}
// keyboard end


void ItemView::dragEnterEvent(QDragEnterEvent *event)
{
	event->accept();
}

void ItemView::dragMoveEvent(QDragMoveEvent *event)
{
	event->accept();
}

void ItemView::dropEvent(QDropEvent *event)
{
	event->accept();

	const QMimeData* mimedata = event->mimeData();
	if(!mimedata) {
		return;
	}

	QString text;

	if(mimedata->hasText()){
		text = mimedata->text();
	}

	// extern drops
	if( !mimedata->hasUrls() || text.compare("tracks", Qt::CaseInsensitive) == 0) {
		return;
	}

	QStringList filelist;
	const QList<QUrl> urls = mimedata->urls();
	for(const QUrl& url : urls)
	{
		QString path = url.path();

		if(QFile::exists(path)) {
			filelist << path;
		}
	}

	emit sig_import_files(filelist);
}

void ItemView::changeEvent(QEvent* event)
{
	SearchableTableView::changeEvent(event);
	QEvent::Type type = event->type();

	if(type == QEvent::FontChange)
	{
		resize_rows_to_contents();
	}
}

void ItemView::resizeEvent(QResizeEvent *event)
{
	SearchableTableView::resizeEvent(event);

	if(m->btn_clear_selection){
		show_clear_button(m->btn_clear_selection->isVisible());
	}
}
