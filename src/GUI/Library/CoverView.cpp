/* CoverView.cpp */

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



#include "CoverView.h"
#include "CoverModel.h"
#include "CoverDelegate.h"
#include "CoverViewContextMenu.h"

#include "Components/Library/LocalLibrary.h"
#include "Components/Tagging/UserTaggingOperations.h"

#include "GUI/Utils/ContextMenu/LibraryContextMenu.h"

#include "Utils/Library/Sorting.h"
#include "Utils/Library/Sortorder.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Language.h"
#include "Utils/Utils.h"
#include "Utils/Logger/Logger.h"

#include <QHeaderView>
#include <QTimer>
#include <QWheelEvent>
#include <QShortcut>
#include <QKeySequence>

#include <mutex>

using Library::CoverView;
using Library::CoverModel;
using AtomicBool=std::atomic<bool>;
using AtomicInt=std::atomic<int>;

struct CoverView::Private
{
	LocalLibrary*	library=nullptr;
	CoverModel*		model=nullptr;

	std::atomic_flag zoom_locked = ATOMIC_FLAG_INIT;

	Private() : zoom_locked(false) {}
};

CoverView::CoverView(QWidget* parent) :
	Library::ItemView(parent)
{
	m = Pimpl::make<Private>();

	connect(this, &ItemView::doubleClicked, this, &CoverView::play_clicked);
}

CoverView::~CoverView() {}

void CoverView::init(LocalLibrary* library)
{
	m->library = library;
	m->model = new Library::CoverModel(this, library);

	ItemView::set_selection_type( SelectionViewInterface::SelectionType::Items );
	ItemView::set_metadata_interpretation(MD::Interpretation::Albums);
	ItemView::set_item_model(m->model);

	this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	this->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
	this->setSelectionBehavior(QAbstractItemView::SelectItems);
	this->setItemDelegate(new Library::CoverDelegate(this));
	this->setShowGrid(false);
	this->setAlternatingRowColors(false);

	connect(m->library, &LocalLibrary::sig_all_albums_loaded, this, &CoverView::reload);

	if(this->horizontalHeader()){
		this->horizontalHeader()->hide();
	}

	if(this->verticalHeader()){
		this->verticalHeader()->hide();
	}

	new QShortcut(QKeySequence(QKeySequence::Refresh), this, SLOT(reload()), nullptr, Qt::WidgetShortcut);
	new QShortcut(QKeySequence("F7"), this, SLOT(clear_cache()));
}

AbstractLibrary* CoverView::library() const
{
	return m->library;
}

QStringList CoverView::zoom_actions()
{
	return QStringList{"50", "75", "100", "125", "150", "175", "200"};
}


void CoverView::change_zoom(int zoom)
{
	bool force_reload = (zoom < 0);

	if(row_count() == 0){
		return;
	}

	if(force_reload){
		zoom = m->model->zoom();
	}

	else
	{
		if(zoom == m->model->zoom()){
			return;
		}
	}

	zoom = std::min(zoom, 200);
	zoom = std::max(zoom, 50);

	if(!force_reload)
	{
		if( zoom == m->model->zoom() )
		{
			return;
		}
	}

	SetSetting(Set::Lib_CoverZoom, zoom);

	if(m->zoom_locked.test_and_set()){
		return;
	}

	m->model->set_zoom(zoom, this->size());
	resize_sections();
	m->zoom_locked.clear();
}

void CoverView::resize_sections()
{
	if(this->is_empty()){
		return;
	}

	this->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
	this->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
}


QList<ActionPair> CoverView::sorting_actions()
{
	using namespace Library;

	QList<ActionPair> ret
	{
		ActionPair(Lang::Name, Lang::Ascending, SortOrder::AlbumNameAsc),
		ActionPair(Lang::Name, Lang::Descending, SortOrder::AlbumNameDesc),
		ActionPair(Lang::Year, Lang::Ascending, SortOrder::AlbumYearAsc),
		ActionPair(Lang::Year, Lang::Descending, SortOrder::AlbumYearDesc),
		ActionPair(Lang::Artist, Lang::Ascending, SortOrder::ArtistNameAsc),
		ActionPair(Lang::Artist, Lang::Descending, SortOrder::ArtistNameDesc),
		ActionPair(Lang::NumTracks, Lang::Ascending, SortOrder::AlbumTracksAsc),
		ActionPair(Lang::NumTracks, Lang::Descending, SortOrder::AlbumTracksDesc),
		ActionPair(Lang::Duration, Lang::Ascending, SortOrder::AlbumDurationAsc),
		ActionPair(Lang::Duration, Lang::Descending, SortOrder::AlbumDurationDesc)
	};

	return ret;
}


void CoverView::change_sortorder(Library::SortOrder so)
{
	m->library->change_album_sortorder(so);
}


void CoverView::init_context_menu()
{
	if(context_menu()){
		return;
	}

	CoverViewContextMenu* cm = new CoverViewContextMenu(this);
	ItemView::init_context_menu_custom_type(cm);

	connect(cm, &CoverViewContextMenu::sig_zoom_changed, this, &CoverView::change_zoom);
	connect(cm, &CoverViewContextMenu::sig_sorting_changed, this, &CoverView::change_sortorder);
}

void CoverView::reload()
{
	m->model->reload();
}

void CoverView::clear_cache()
{
	sp_log(Log::Debug, this) << "Clear cache";
	m->model->clear();
}

void CoverView::fill()
{
	this->clearSelection();
}

void CoverView::language_changed() {}

QStyleOptionViewItem CoverView::viewOptions() const
{
	QStyleOptionViewItem option = ItemView::viewOptions();
	option.decorationAlignment = Qt::AlignHCenter;
	option.displayAlignment = Qt::AlignHCenter;
	option.decorationPosition = QStyleOptionViewItem::Top;
	option.textElideMode = Qt::ElideRight;

	return option;
}


void CoverView::wheelEvent(QWheelEvent* e)
{
	if( (e->modifiers() & Qt::ControlModifier) && (e->delta() != 0) )
	{
		int d = (e->delta() > 0) ? 10 : -10;

		change_zoom(m->model->zoom() + d);
	}

	else
	{
		ItemView::wheelEvent(e);
	}
}

void CoverView::resizeEvent(QResizeEvent* e)
{
	ItemView::resizeEvent(e);
	change_zoom();
}

int CoverView::sizeHintForColumn(int c) const
{
	Q_UNUSED(c)
	return m->model->item_size().width();
}

void CoverView::hideEvent(QHideEvent* e)
{
	if(m->model){
		m->model->clear();
	}

	ItemView::hideEvent(e);
}

int CoverView::index_by_model_index(const QModelIndex& idx) const
{
	return idx.row() * model()->columnCount() + idx.column();
}

ModelIndexRange CoverView::model_indexrange_by_index(int idx) const
{
	int row = idx / model()->columnCount();
	int col = idx % model()->columnCount();

	return ModelIndexRange(model()->index(row, col), model()->index(row, col));
}

void CoverView::play_clicked()
{
	m->library->prepare_fetched_tracks_for_playlist(false);
}

void CoverView::play_new_tab_clicked()
{
	m->library->prepare_fetched_tracks_for_playlist(true);
}

void CoverView::play_next_clicked()
{
	m->library->play_next_fetched_tracks();
}

void CoverView::append_clicked()
{
	m->library->append_fetched_tracks();
}

void CoverView::selection_changed(const IndexSet& indexes)
{
	m->library->selected_albums_changed(indexes);
}

void CoverView::refresh_clicked()
{
	m->library->refresh_albums();
}

void CoverView::run_merge_operation(const Library::ItemView::MergeData& mergedata)
{
	Tagging::UserOperations* uto = new Tagging::UserOperations(mergedata.library_id(), this);

	connect(uto, &Tagging::UserOperations::sig_finished, uto, &Tagging::UserOperations::deleteLater);

	uto->merge_albums(mergedata.source_ids(), mergedata.target_id());
}

