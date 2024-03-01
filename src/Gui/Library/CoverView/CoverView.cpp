/* CoverView.cpp */

/* Copyright (C) 2011-2024 Michael Lugmair (Lucio Carreras)
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
#include "CoverViewSortorderInfo.h"

#include "Components/Library/LocalLibrary.h"
#include "Components/Tagging/UserTaggingOperations.h"
#include "Gui/Utils/ContextMenu/LibraryContextMenu.h"
#include "Gui/Utils/GuiUtils.h"
#include "Utils/Language/Language.h"
#include "Utils/Library/MergeData.h"
#include "Utils/Library/Sorting.h"
#include "Utils/Library/Sortorder.h"
#include "Utils/Logger/Logger.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Set.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Tagging/TagReader.h"
#include "Utils/Tagging/TagWriter.h"
#include "Utils/Utils.h"

#include <QHeaderView>
#include <QTimer>
#include <QWheelEvent>
#include <QShortcut>
#include <QKeySequence>
#include <QScrollBar>

#include <mutex>

namespace
{
	int ensureZoomInValidRange(int zoom, const QList<int>& zoomFactors)
	{
		return std::min(std::max(zoom, zoomFactors.first()), zoomFactors.last());
	}

	void hideHeader(QHeaderView* header)
	{
		if(header)
		{
			header->hide();
		}
	}

	void setScrollspeed(QScrollBar* scrollbar, const int step)
	{
		if(scrollbar)
		{
			scrollbar->setSingleStep(step);
		}
	}
}

namespace Library
{
	struct CoverView::Private
	{
		LocalLibrary* library {nullptr};
		CoverModel* model {nullptr};
		std::atomic_flag zoomLocked {false};
	};

	CoverView::CoverView(QWidget* parent) :
		Library::ItemView(parent),
		m {Pimpl::make<Private>()} {}

	CoverView::~CoverView() = default;

	void CoverView::init(LocalLibrary* library)
	{
		ItemView::init(PlayActionEventHandler::create(library->playlistInteractor(), library));

		m->library = library;
		m->model = new Library::CoverModel(this, library);

		setModel(m->model);
		setItemDelegate(new Library::CoverDelegate(this));
		setShowGrid(false);
		setAlternatingRowColors(false);
		setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectItems);
		hideHeader(horizontalHeader());
		hideHeader(verticalHeader());
		setScrollspeed(verticalScrollBar(), GetSetting(Set::Lib_CoverScrollspeed));

		connect(m->library, &LocalLibrary::sigAllAlbumsLoaded, this, &CoverView::reload);

		new QShortcut(QKeySequence(QKeySequence::Refresh), this, SLOT(reload()), nullptr, Qt::WidgetShortcut);
		new QShortcut(QKeySequence("F7"), this, SLOT(clearCache()));

		ListenSetting(Set::Lib_CoverScrollspeed, CoverView::scrollspeedChanged);
	}

	AbstractLibrary* CoverView::library() const { return m->library; }

	QList<int> CoverView::zoomFactors()
	{
		static const auto list = QList<int> {50, 75, 100, 125, 150, 175, 200, 225, 250};
		return list;
	}

	int CoverView::zoom() const { return m->model->zoom(); }

	void CoverView::changeZoom(int zoom)
	{
		if(m->model->rowCount() == 0)
		{
			return;
		}

		const auto currentZoom = this->zoom();
		const auto forceReload = (zoom < 0);
		if(forceReload)
		{
			zoom = currentZoom;
		}

		zoom = ensureZoomInValidRange(zoom, CoverView::zoomFactors());

		const auto isZoomEqual = (zoom == currentZoom) || (zoom == m->model->zoom());
		if(!forceReload && isZoomEqual)
		{
			return;
		}

		if(const auto isZoomForbidden = m->zoomLocked.test_and_set(); isZoomForbidden)
		{
			return;
		}

		m->model->setZoom(zoom, size());
		resizeSections();
		m->zoomLocked.clear();
	}

	void CoverView::resizeSections()
	{
		if(m->model->rowCount() > 0)
		{
			verticalHeader()->setDefaultSectionSize(m->model->itemSize().height());
			horizontalHeader()->setDefaultSectionSize(m->model->itemSize().width());
		}
	}

	QList<CoverViewSortorderInfo> CoverView::sortingActions()
	{
		return {
			{Lang::Name,      true,  AlbumSortorder::NameAsc},
			{Lang::Name,      false, AlbumSortorder::NameDesc},
			{Lang::Artist,    true,  AlbumSortorder::AlbumArtistAsc},
			{Lang::Artist,    false, AlbumSortorder::AlbumArtistDesc},
			{Lang::Created,   true,  AlbumSortorder::CreationDateAsc},
			{Lang::Created,   false, AlbumSortorder::CreationDateDesc},
			{Lang::Year,      true,  AlbumSortorder::YearAsc},
			{Lang::Year,      false, AlbumSortorder::YearDesc},
			{Lang::Duration,  true,  AlbumSortorder::DurationAsc},
			{Lang::Duration,  false, AlbumSortorder::DurationDesc},
			{Lang::NumTracks, true,  AlbumSortorder::TracksAsc},
			{Lang::NumTracks, false, AlbumSortorder::TracksDesc},
		};
	}

	void CoverView::changeSortorder(const Library::AlbumSortorder so)
	{
		m->library->changeAlbumSortorder(so);
	}

	void CoverView::initContextMenu()
	{
		if(!contextMenu())
		{
			auto* contextMenu = new CoverViewContextMenu(this);
			ItemView::initCustomContextMenu(contextMenu);

			connect(contextMenu, &CoverViewContextMenu::sigZoomChanged, this, &CoverView::changeZoom);
			connect(contextMenu, &CoverViewContextMenu::sigSortingChanged, this, &CoverView::changeSortorder);
		}
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
		clearSelection();
	}

	QStyleOptionViewItem CoverView::viewOptions() const
	{
		auto option = ItemView::viewOptions();

		option.decorationAlignment = Qt::AlignHCenter;
		option.displayAlignment = Qt::AlignHCenter;
		option.decorationPosition = QStyleOptionViewItem::Top;
		option.textElideMode = Qt::ElideRight;

		return option;
	}

	int CoverView::sizeHintForColumn(const int /*column*/) const { return m->model->itemSize().width(); }

	bool CoverView::isMergeable() const { return true; }

	MD::Interpretation CoverView::metadataInterpretation() const { return MD::Interpretation::Albums; }

	int CoverView::mapModelIndexToIndex(const QModelIndex& idx) const
	{
		return idx.row() * m->model->columnCount() + idx.column();
	}

	ModelIndexRange CoverView::mapIndexToModelIndexes(const int idx) const
	{
		const auto row = idx / m->model->columnCount();
		const auto col = idx % m->model->columnCount();

		return {
			m->model->index(row, col),
			m->model->index(row, col)
		};
	}

	SelectionViewInterface::SelectionType CoverView::selectionType() const
	{
		return SelectionViewInterface::SelectionType::Items;
	}

	void CoverView::refreshView() { m->library->refreshAlbums(); }

	void CoverView::runMergeOperation(const MergeData& mergedata)
	{
		auto* taggingOperation =
			new Tagging::UserOperations(Tagging::TagReader::create(),
			                            Tagging::TagWriter::create(),
			                            mergedata.libraryId(),
			                            this);

		connect(taggingOperation, &Tagging::UserOperations::sigFinished,
		        taggingOperation, &Tagging::UserOperations::deleteLater);

		taggingOperation->mergeAlbums(mergedata.sourceIds(), mergedata.targetId());
	}

	void CoverView::scrollspeedChanged()
	{
		setScrollspeed(verticalScrollBar(), GetSetting(Set::Lib_CoverScrollspeed));
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

	ItemModel* CoverView::itemModel() const { return m->model; }

	PlayActionEventHandler::TrackSet CoverView::trackSet() const { return PlayActionEventHandler::TrackSet::All; }

	void CoverView::triggerSelectionChange(const IndexSet& indexes)
	{
		m->library->selectedAlbumsChanged(indexes);
	}
}
