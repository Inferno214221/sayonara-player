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
#include "ItemModel.h"

#include "HeaderView.h"
#include "Components/Library/AbstractLibrary.h"
#include "Components/Covers/CoverLocation.h"
#include "GUI/Library/Utils/ColumnHeader.h"

#include "Utils/globals.h"
#include "Utils/Settings/Settings.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Set.h"
#include "Utils/FileUtils.h"
#include "Utils/Logger/Logger.h"

#include "GUI/Utils/ContextMenu/LibraryContextMenu.h"
#include "GUI/Utils/SearchableWidget/MiniSearcher.h"
#include "GUI/Utils/CustomMimeData.h"
#include "GUI/Utils/PreferenceAction.h"

#include <QShortcut>
#include <QKeySequence>
#include <QDrag>
#include <QDropEvent>
#include <QMouseEvent>
#include <QPushButton>
#include <QScrollBar>
#include <QBoxLayout>
#include <QHeaderView>
#include <QItemSelectionModel>

using namespace Library;

struct Library::ItemView::Private
{
	ItemModel*			model=nullptr;
	QPushButton*		btn_clear_selection=nullptr;
	QAction*			merge_action=nullptr;
	QAction*			album_artist_action=nullptr;
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
	SearchableTableView(parent),
	InfoDialogContainer(),
	Dragable(this)
{
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

	new QShortcut(QKeySequence(Qt::Key_Return), this, SLOT(play_clicked()), nullptr, Qt::WidgetShortcut);
	new QShortcut(QKeySequence(Qt::Key_Enter), this, SLOT(play_clicked()), nullptr, Qt::WidgetShortcut);
	new QShortcut(QKeySequence(Qt::AltModifier + Qt::Key_Return), this, SLOT(play_next_clicked()), nullptr, Qt::WidgetShortcut);
	new QShortcut(QKeySequence(Qt::AltModifier + Qt::Key_Enter), this, SLOT(play_next_clicked()), nullptr, Qt::WidgetShortcut);
	new QShortcut(QKeySequence(Qt::ShiftModifier + Qt::Key_Return), this, SLOT(append_clicked()), nullptr, Qt::WidgetShortcut);
	new QShortcut(QKeySequence(Qt::ShiftModifier + Qt::Key_Enter), this, SLOT(append_clicked()), nullptr, Qt::WidgetShortcut);
	new QShortcut(QKeySequence(Qt::Key_Backspace), this, SLOT(clearSelection()), nullptr, Qt::WidgetShortcut);
}

ItemView::~ItemView() {}

AbstractLibrary* ItemView::library() const { return nullptr; }

ItemModel* ItemView::item_model() const
{
	return m->model;
}

void ItemView::set_item_model(ItemModel* model)
{
	m->model = model;

	SearchableTableView::set_model(model);

	QItemSelectionModel* sm = this->selectionModel();
	connect(sm, &QItemSelectionModel::selectionChanged, this, &ItemView::selected_items_changed);
}

LibraryContextMenu::Entries ItemView::context_menu_entries() const
{
	LibraryContextMenu::Entries entries = (
			LibraryContextMenu::EntryPlay |
			LibraryContextMenu::EntryPlayNewTab |
			LibraryContextMenu::EntryInfo |
			LibraryContextMenu::EntryEdit |
			LibraryContextMenu::EntryDelete |
			LibraryContextMenu::EntryPlayNext |
			LibraryContextMenu::EntryAppend |
			LibraryContextMenu::EntryCoverView);

	return entries;
}

void ItemView::selected_items_changed(const QItemSelection& selected, const QItemSelection& deselected )
{
	Q_UNUSED(deselected)
	show_clear_button(!selected.empty());

	if(m->cur_filling) {
		return;
	}

	if(m->context_menu){
		m->context_menu->show_action(LibraryContextMenu::EntryClearSelection, !selected.isEmpty());
	}

	selection_changed(selected_items());
}

void ItemView::init_context_menu()
{
	init_context_menu_custom_type(nullptr);
}

// Right click stuff
void ItemView::init_context_menu_custom_type(LibraryContextMenu* menu)
{
	if(m->context_menu){
		return;
	}

	if(!menu){
		m->context_menu = new LibraryContextMenu(this);
	}

	else {
		m->context_menu = menu;
	}

	m->merge_menu = new QMenu(tr("Merge"), m->context_menu);
	m->merge_action = m->context_menu->addMenu(m->merge_menu);
	m->merge_action->setVisible(false);

	QAction* action_clear = m->context_menu->get_action(LibraryContextMenu::EntryClearSelection);
	m->context_menu->insertAction(action_clear, m->merge_action);

	connect(m->context_menu, &LibraryContextMenu::sig_edit_clicked, this, [=](){ show_edit(); });
	connect(m->context_menu, &LibraryContextMenu::sig_info_clicked, this, [=](){ show_info(); });
	connect(m->context_menu, &LibraryContextMenu::sig_lyrics_clicked, this, [=](){ show_lyrics(); });
	connect(m->context_menu, &LibraryContextMenu::sig_clear_selection_clicked, this, [=](){ clear_selection(); });
	connect(m->context_menu, &LibraryContextMenu::sig_delete_clicked, this, &ItemView::delete_clicked);
	connect(m->context_menu, &LibraryContextMenu::sig_play_clicked, this, &ItemView::play_clicked);
	connect(m->context_menu, &LibraryContextMenu::sig_play_next_clicked, this, &ItemView::play_next_clicked);
	connect(m->context_menu, &LibraryContextMenu::sig_play_new_tab_clicked, this, &ItemView::play_new_tab_clicked);
	connect(m->context_menu, &LibraryContextMenu::sig_append_clicked, this, &ItemView::append_clicked);
	connect(m->context_menu, &LibraryContextMenu::sig_refresh_clicked, this, &ItemView::refresh_clicked);

	this->show_context_menu_actions(context_menu_entries());
	m->context_menu->add_preference_action(new LibraryPreferenceAction(m->context_menu));
}

LibraryContextMenu* ItemView::context_menu() const
{
	return m->context_menu;
}

void ItemView::show_context_menu(const QPoint& p)
{
	m->context_menu->exec(p);
}

void ItemView::show_context_menu_actions(LibraryContextMenu::Entries entries)
{
	m->context_menu->show_actions(entries);
	m->context_menu->show_action(LibraryContextMenu::EntryClearSelection, !selected_items().isEmpty());
}

QMimeData* ItemView::dragable_mimedata() const
{
	return item_model()->custom_mimedata();
}

QPixmap ItemView::drag_pixmap() const
{
	Cover::Location cl = item_model()->cover(selected_items());

	QString cover_path = cl.preferred_path();
	return QPixmap(cover_path);
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


void ItemView::set_metadata_interpretation(MD::Interpretation type)
{
	m->type = type;
}

MD::Interpretation ItemView::metadata_interpretation() const
{
	return m->type;
}

MetaDataList ItemView::info_dialog_data() const
{
	return item_model()->mimedata_tracks();
}

bool ItemView::MergeData::is_valid() const
{
	return ((target_id >= 0) && (source_ids.count() >= 2) && !(source_ids.contains(-1)));
}

ItemView::MergeData ItemView::calc_mergedata() const
{
	ItemView::MergeData ret;
	QAction* action = static_cast<QAction*>(sender());
	ret.target_id = action->data().toInt();

	IndexSet selected_items = this->selected_items();

	ItemModel* model = item_model();
	for(auto idx : selected_items)
	{
		ret.source_ids.insert( model->id_by_index(idx) );
	}

	return ret;
}

void ItemView::merge_action_triggered()
{
	ItemView::MergeData mergedata = calc_mergedata();

	if(mergedata.is_valid()){
		run_merge_operation(mergedata);
	}
}

void ItemView::run_merge_operation(const ItemView::MergeData& md) { Q_UNUSED(md) }

void ItemView::play_clicked() { emit sig_play_clicked(); }
void ItemView::play_new_tab_clicked() { emit sig_play_new_tab_clicked(); }
void ItemView::play_next_clicked() { emit sig_play_next_clicked(); }
void ItemView::delete_clicked() { emit sig_delete_clicked(); }
void ItemView::append_clicked() { emit sig_append_clicked(); }
void ItemView::refresh_clicked() { emit sig_refresh_clicked(); }

void ItemView::fill()
{
	IndexSet selections = m->model->selected_indexes();

	int old_size, new_size;
	m->model->refresh_data(&old_size, &new_size);

	select_items(selections);

	if(new_size > old_size) {
		resize_rows_to_contents(old_size, new_size - old_size);
	}
}


void ItemView::selection_changed(const IndexSet& indexes)
{
	emit sig_sel_changed(indexes);
}


void ItemView::import_requested(const QStringList& files)
{
	AbstractLibrary* lib = this->library();
	if(lib){
		lib->import_files(files);
	}
}


void ItemView::resize_rows_to_contents()
{
	if(!item_model() || is_empty()) {
		return;
	}

	QHeaderView* header = this->verticalHeader();
	if(header) {
		header->resizeSections(QHeaderView::ResizeToContents);
	}
}


void ItemView::resize_rows_to_contents(int first_row, int count)
{
	if(!item_model() || is_empty()) {
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


void ItemView::mousePressEvent(QMouseEvent* event)
{
	if(is_empty())
	{
		return;
	}

	if(event->button() == Qt::LeftButton){
		this->drag_pressed(event->pos());
	}

	SearchableTableView::mousePressEvent(event);

	if(event->button() == Qt::MidButton)
	{
		if(!this->selected_items().isEmpty()){
			play_new_tab_clicked();
		}
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

	bool is_mergeable =
			(m->type == MD::Interpretation::Artists ||
			 m->type == MD::Interpretation::Albums);

	if(is_mergeable)
	{
		size_t n_selections = selections.size();

		if(n_selections > 1)
		{
			m->merge_menu->clear();

			ItemModel* model = item_model();
			for(int i : selections)
			{
				QString name = item_model()->searchable_string(i);
				name.replace("&", "&&");

				QAction* action = new QAction(name, m->merge_menu);

				int id = model->id_by_index(i);
				action->setData(id);
				connect(action, &QAction::triggered, this, &ItemView::merge_action_triggered);

				m->merge_menu->addAction(action);
			}

			m->merge_action->setVisible(n_selections > 1);
		}
	}

	show_context_menu(pos);

	QTableView::contextMenuEvent(event);
}

void ItemView::dragEnterEvent(QDragEnterEvent *event) {	event->accept(); }
void ItemView::dragMoveEvent(QDragMoveEvent *event) { event->accept(); }
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

		if(::Util::File::exists(path)) {
			filelist << path;
		}
	}

	import_requested(filelist);
}


void ItemView::changeEvent(QEvent* event)
{
	SearchableTableView::changeEvent(event);

	if(event->type() == QEvent::FontChange)
	{
		resize_rows_to_contents();
	}
}


void ItemView::keyPressEvent(QKeyEvent* event)
{
	event->setAccepted(false);
	SearchableTableView::keyPressEvent(event);
}


void ItemView::resizeEvent(QResizeEvent *event)
{
	SearchableTableView::resizeEvent(event);

	if(m->btn_clear_selection){
		show_clear_button(m->btn_clear_selection->isVisible());
	}
}

