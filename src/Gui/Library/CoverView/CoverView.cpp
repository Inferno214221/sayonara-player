/* CoverView.cpp */

/* Copyright (C) 2011-2020 Michael Lugmair (Lucio Carreras)
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

#include "Gui/Library/Header/ActionPair.h"
#include "Gui/Utils/ContextMenu/LibraryContextMenu.h"

#include "Utils/Library/Sorting.h"
#include "Utils/Library/Sortorder.h"
#include "Utils/Library/MergeData.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Language/Language.h"
#include "Utils/Utils.h"
#include "Utils/Set.h"
#include "Utils/Logger/Logger.h"

#include <QHeaderView>
#include <QTimer>
#include <QWheelEvent>
#include <QShortcut>
#include <QKeySequence>

#include <mutex>

namespace Library
{
	struct CoverView::Private
	{
		LocalLibrary* library = nullptr;
		CoverModel* model = nullptr;

		std::atomic_flag zoomLocked {0};
	};

	CoverView::CoverView(QWidget* parent) :
		Library::ItemView(parent)
	{
		m = Pimpl::make<Private>();
	}

	CoverView::~CoverView() = default;

	void CoverView::init(LocalLibrary* library)
	{
		m->library = library;
		m->model = new Library::CoverModel(this, library);

		ItemView::setItemModel(m->model);

		this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		this->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
		this->setSelectionBehavior(QAbstractItemView::SelectItems);
		this->setItemDelegate(new Library::CoverDelegate(this));
		this->setShowGrid(false);
		this->setAlternatingRowColors(false);

		connect(m->library, &LocalLibrary::sigAllAlbumsLoaded, this, &CoverView::reload);

		if(this->horizontalHeader())
		{
			this->horizontalHeader()->hide();
		}

		if(this->verticalHeader())
		{
			this->verticalHeader()->hide();
		}

		new QShortcut(QKeySequence(QKeySequence::Refresh), this, SLOT(reload()), nullptr, Qt::WidgetShortcut);
		new QShortcut(QKeySequence("F7"), this, SLOT(clearCache()));
	}

	AbstractLibrary* CoverView::library() const
	{
		return m->library;
	}

QStringList CoverView::zoomActions()
{
	return QStringList{"50", "75", "100", "125", "150", "175", "200"};
}

void CoverView::changeZoom(int zoom)
{
	bool forceReload = (zoom < 0);

	if(itemModel()->rowCount() == 0){
		return;
	}

	if(forceReload){
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

	if(!forceReload)
	{
		if( zoom == m->model->zoom() )
		{
			return;
		}
	}

	SetSetting(Set::Lib_CoverZoom, zoom);

	if(m->zoomLocked.test_and_set()){
		return;
	}

	m->model->setZoom(zoom, this->size());
	resizeSections();
	m->zoomLocked.clear();
}

	void CoverView::resizeSections()
	{
		if(itemModel()->rowCount() == 0)
		{
			return;
		}

		verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
		horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
	}

	QList<ActionPair> CoverView::sortingActions()
	{
		static const auto result = QList<ActionPair>
			{
				ActionPair(Lang::Name, true, SortOrder::AlbumNameAsc),
				ActionPair(Lang::Name, false, SortOrder::AlbumNameDesc),
				ActionPair(Lang::Year, true, SortOrder::AlbumYearAsc),
				ActionPair(Lang::Year, false, SortOrder::AlbumYearDesc),
				ActionPair(Lang::Artist, true, SortOrder::ArtistNameAsc),
				ActionPair(Lang::Artist, false, SortOrder::ArtistNameDesc),
				ActionPair(Lang::NumTracks, true, SortOrder::AlbumTracksAsc),
				ActionPair(Lang::NumTracks, false, SortOrder::AlbumTracksDesc),
				ActionPair(Lang::Duration, true, SortOrder::AlbumDurationAsc),
				ActionPair(Lang::Duration, false, SortOrder::AlbumDurationDesc)
			};

		return result;
	}

	void CoverView::changeSortorder(Library::SortOrder so)
	{
		m->library->changeAlbumSortorder(so);
	}

	void CoverView::initContextMenu()
	{
		if(contextMenu())
		{
			return;
		}

		auto* contextMenu = new CoverViewContextMenu(this);
		ItemView::initCustomContextMenu(contextMenu);

		connect(contextMenu, &CoverViewContextMenu::sigZoomChanged, this, &CoverView::changeZoom);
		connect(contextMenu, &CoverViewContextMenu::sigSortingChanged, this, &CoverView::changeSortorder);
	}

	void CoverView::reload()
	{
		m->model->reload();
	}

	void CoverView::clearCache()
	{
		spLog(Log::Debug, this) << "Clear cache";
		m->model->clear();
	}

	void CoverView::fill()
	{
		this->clearSelection();
	}

	void CoverView::languageChanged() {}

	QStyleOptionViewItem CoverView::viewOptions() const
	{
		auto option = ItemView::viewOptions();

		option.decorationAlignment = Qt::AlignHCenter;
		option.displayAlignment = Qt::AlignHCenter;
		option.decorationPosition = QStyleOptionViewItem::Top;
		option.textElideMode = Qt::ElideRight;

		return option;
	}

	int CoverView::sizeHintForColumn([[maybe_unused]] int column) const
	{
		return m->model->itemSize().width();
	}

	bool CoverView::isMergeable() const
	{
		return true;
	}

	MD::Interpretation CoverView::metadataInterpretation() const
	{
		return MD::Interpretation::Albums;
	}

	int CoverView::mapModelIndexToIndex(const QModelIndex& idx) const
	{
		return idx.row() * model()->columnCount() + idx.column();
	}

	ModelIndexRange CoverView::mapIndexToModelIndexes(int idx) const
	{
		const auto row = idx / model()->columnCount();
		const auto col = idx % model()->columnCount();

		return ModelIndexRange(model()->index(row, col), model()->index(row, col));
	}

	SelectionViewInterface::SelectionType CoverView::selectionType() const
	{
		return SelectionViewInterface::SelectionType::Items;
	}

	void CoverView::playClicked()
	{
		m->library->prepareFetchedTracksForPlaylist(false);
	}

	void CoverView::playNewTabClicked()
	{
		m->library->prepareFetchedTracksForPlaylist(true);
	}

	void CoverView::playNextClicked()
	{
		m->library->playNextFetchedTracks();
	}

	void CoverView::appendClicked()
	{
		m->library->appendFetchedTracks();
	}

	void CoverView::selectedItemsChanged(const IndexSet& indexes)
	{
		m->library->selectedAlbumsChanged(indexes);
	}

	void CoverView::refreshClicked() {}

	void CoverView::runMergeOperation(const MergeData& mergedata)
	{
		auto* taggingOperation = new Tagging::UserOperations(mergedata.libraryId(), this);

		connect(taggingOperation, &Tagging::UserOperations::sigFinished,
		        taggingOperation, &Tagging::UserOperations::deleteLater);

		taggingOperation->mergeAlbums(mergedata.sourceIds(), mergedata.targetId());
	}

	void CoverView::wheelEvent(QWheelEvent* e)
	{
		const auto delta = e->angleDelta().y();
		if((e->modifiers() & Qt::ControlModifier) && (delta != 0))
		{
			const auto d = (delta > 0) ? 10 : -10;
			changeZoom(m->model->zoom() + d);
		}

		else
		{
			ItemView::wheelEvent(e);
		}
	}

	void CoverView::resizeEvent(QResizeEvent* e)
	{
		ItemView::resizeEvent(e);
		changeZoom();
	}

	void CoverView::hideEvent(QHideEvent* e)
	{
		if(m->model)
		{
			m->model->clear();
		}

		ItemView::hideEvent(e);
	}
}