/* PlaylistView.cpp */

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

#include "PlaylistView.h"
#include "PlaylistModel.h"
#include "PlaylistDelegate.h"
#include "PlaylistContextMenu.h"

#include "Gui/Utils/GuiUtils.h"
#include "Gui/Utils/PreferenceAction.h"
#include "Gui/Utils/ContextMenu/LibraryContextMenu.h"
#include "Gui/Utils/Widgets/ProgressBar.h"
#include "Gui/Utils/MimeData/CustomMimeData.h"
#include "Gui/Utils/MimeData/MimeDataUtils.h"
#include "Gui/Utils/MimeData/DragDropAsyncHandler.h"

#include "Utils/Language/Language.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Message/Message.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Set.h"
#include "Utils/Settings/Settings.h"

#include "Components/Playlist/Playlist.h"
#include "Interfaces/PlaylistInterface.h"

#include <QShortcut>
#include <QDropEvent>
#include <QHeaderView>
#include <QScrollBar>
#include <QDrag>
#include <QTimer>
#include <QLabel>

#include <algorithm>

using namespace Gui;

namespace Pl = ::Playlist;
using Pl::View;

struct View::Private
{
	View* view;
	DynamicPlaybackChecker* dynamicPlaybackChecker;
	Pl::ContextMenu* contextMenu = nullptr;
	Pl::Model* model;
	ProgressBar* progressbar;
	QLabel* currentFileLabel;

	Private(PlaylistCreator* playlistCreator, PlaylistPtr playlist, DynamicPlaybackChecker* dynamicPlaybackChecker,
	        View* parent) :
		view(parent),
		dynamicPlaybackChecker(dynamicPlaybackChecker),
		model(new Pl::Model(playlistCreator, playlist, parent)),
		progressbar(new ProgressBar(parent)),
		currentFileLabel(new QLabel(parent))
	{
		view->setObjectName(QString("playlist_view%1").arg(playlist->index()));
		view->setSearchableModel(this->model);
		view->setItemDelegate(new Pl::Delegate(view));

		view->setTabKeyNavigation(false);
		view->setSelectionMode(QAbstractItemView::ExtendedSelection);
		view->setAlternatingRowColors(true);
		view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		view->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
		view->setSelectionBehavior(QAbstractItemView::SelectRows);
		view->setShowGrid(false);
		view->setAutoScroll(true);
		view->setAutoScrollMargin(50);
		view->setDragEnabled(true);
		view->setDragDropMode(QAbstractItemView::DragDrop);
		view->setDragDropOverwriteMode(false);
		view->setAcceptDrops(true);
		view->setDropIndicatorShown(true);

		view->verticalHeader()->hide();
		view->verticalHeader()->setMinimumSectionSize(1);
		view->horizontalHeader()->hide();
		view->horizontalHeader()->setMinimumSectionSize(0);
		view->horizontalHeader()->setMinimumSectionSize(10);

		this->progressbar->hide();
		this->currentFileLabel->hide();
	}

	int minimumSelectedItem()
	{
		const auto selected = view->selectedItems();
		auto it = std::min_element(selected.begin(), selected.end());

		return (it == selected.end()) ? -1 : *it;
	}

	void resizeSection(int column, int size)
	{
		if(view->horizontalHeader()->sectionSize(column) != size)
		{
			view->horizontalHeader()->resizeSection(column, size);
		}
	}

	int calcDragDropLine(QPoint pos)
	{
		const auto offset = (view->rowCount() > 0)
		                    ? view->rowHeight(0) / 2
		                    : view->fontMetrics().height() / 2;

		if(pos.y() < offset)
		{
			return -1;
		}

		const auto row = view->indexAt(pos).row();
		return (row >= 0) ? row : view->rowCount() - 1;
	}
};

View::View(PlaylistCreator* playlistCreator, PlaylistPtr playlist, DynamicPlaybackChecker* dynamicPlaybackChecker,
           QWidget* parent) :
	SearchableTableView(parent),
	InfoDialogContainer(),
	Gui::Dragable(this)
{
	m = Pimpl::make<Private>(playlistCreator, playlist, dynamicPlaybackChecker, this);

	ListenSetting(Set::PL_ShowNumbers, View::columnsChanged);
	ListenSetting(Set::PL_ShowCovers, View::columnsChanged);
	ListenSetting(Set::PL_ShowNumbers, View::columnsChanged);
	ListenSetting(Set::PL_ShowRating, View::showRatingChanged);

	new QShortcut(QKeySequence(Qt::Key_Backspace),
	              this, SLOT(clear()), nullptr, Qt::WidgetShortcut);
	new QShortcut(QKeySequence(QKeySequence::Delete),
	              this, SLOT(removeSelectedRows()), nullptr, Qt::WidgetShortcut);
	new QShortcut(QKeySequence(Qt::ControlModifier + Qt::Key_Up),
	              this, SLOT(moveSelectedRowsUp()), nullptr, Qt::WidgetShortcut);
	new QShortcut(QKeySequence(Qt::ControlModifier + Qt::Key_Down),
	              this, SLOT(moveSelectedRowsDown()), nullptr, Qt::WidgetShortcut);
	new QShortcut(QKeySequence(Qt::Key_Return),
	              this, SLOT(playSelectedTrack()), nullptr, Qt::WidgetShortcut);
	new QShortcut(QKeySequence(Qt::Key_Enter),
	              this, SLOT(playSelectedTrack()), nullptr, Qt::WidgetShortcut);

	connect(m->model, &Pl::Model::sigDataReady, this, &View::refresh);

	connect(playlist.get(), &Playlist::sigBusyChanged, this, &View::playlistBusyChanged);
	connect(playlist.get(),
	        &Playlist::sigCurrentScannedFileChanged,
	        this,
	        &View::currentScannedFileChanged);

	QTimer::singleShot(100, this, [=]() {
		this->gotoToCurrentTrack();
	});
}

View::~View() = default;

void View::initContextMenu()
{
	using Pl::ContextMenu;

	m->contextMenu = new ContextMenu(m->dynamicPlaybackChecker, this);
	m->contextMenu->addPreferenceAction(new PlaylistPreferenceAction(m->contextMenu));

	connect(m->contextMenu, &ContextMenu::sigRefreshClicked, m->model, &Pl::Model::refreshData);
	connect(m->contextMenu, &ContextMenu::sigEditClicked, this, [&]() { showEdit(); });
	connect(m->contextMenu, &ContextMenu::sigInfoClicked, this, [&]() { showInfo(); });
	connect(m->contextMenu, &ContextMenu::sigLyricsClicked, this, [&]() { showLyrics(); });
	connect(m->contextMenu, &ContextMenu::sigDeleteClicked, this, &View::deleteSelectedTracks);
	connect(m->contextMenu, &ContextMenu::sigRemoveClicked, this, &View::removeSelectedRows);
	connect(m->contextMenu, &ContextMenu::sigClearClicked, this, &View::clear);
	connect(m->contextMenu, &ContextMenu::sigRatingChanged, this, &View::ratingChanged);
	connect(m->contextMenu, &ContextMenu::sigJumpToCurrentTrack, this, &View::gotoToCurrentTrack);
	connect(m->contextMenu, &ContextMenu::sigBookmarkPressed, this, &View::bookmarkTriggered);
	connect(m->contextMenu, &ContextMenu::sigFindTrackTriggered, this, &View::findTrackTriggered);
	connect(m->contextMenu, &ContextMenu::sigReverseTriggered, this, [&]() {
		m->model->reverseTracks();
	});
}

void View::gotoRow(int row)
{
	row = std::min(row, rowCount() - 1);
	row = std::max(row, 0);

	const auto range = mapIndexToModelIndexes(row);
	this->scrollTo(range.first);
}

void View::handleDrop(QDropEvent* event)
{
	m->model->setDragIndex(-1);

	const auto* mimedata = event->mimeData();
	const auto dragDropLine = m->calcDragDropLine(event->pos());
	const auto isInnerDragDrop = MimeData::isInnerDragDrop(mimedata, m->model->playlistIndex());
	if(isInnerDragDrop)
	{
		const auto selectedRows = selectedItems();
		if(!selectedRows.contains(dragDropLine))
		{
			const auto newSelection = (event->keyboardModifiers() & Qt::ControlModifier)
			                          ? m->model->copyTracks(selectedRows, dragDropLine + 1)
			                          : m->model->moveTracks(selectedRows, dragDropLine + 1);

			this->selectRows(newSelection, 0);
		}

		return;
	}

	auto* asyncDropHandler = MimeData::asyncDropHandler(mimedata);
	if(asyncDropHandler)
	{
		m->model->setBusy(true);

		asyncDropHandler->setTargetIndex(dragDropLine + 1);
		connect(asyncDropHandler, &Gui::AsyncDropHandler::sigFinished,
		        this, &View::asyncDropFinished);
		asyncDropHandler->start();
	}

	else if(MimeData::hasMetadata(mimedata))
	{
		const auto tracks = MimeData::metadata(mimedata);
		m->model->insertTracks(tracks, dragDropLine + 1);
	}
}

void View::asyncDropFinished()
{
	auto* asyncDropHandler = static_cast<Gui::AsyncDropHandler*>(sender());

	// busy playlists do not accept playlists modifications, so we have
	// to disable busy status before inserting the tracks
	m->model->setBusy(false);

	const auto tracks = asyncDropHandler->tracks();
	m->model->insertTracks(tracks, asyncDropHandler->targetIndex());
	asyncDropHandler->deleteLater();
}

void View::ratingChanged(Rating rating)
{
	const auto selections = selectedItems();
	if(!selections.isEmpty())
	{
		m->model->changeRating(selections, rating);
	}
}

void View::moveSelectedRowsUp()
{
	const auto newSelections = m->model->moveTracksUp(selectedItems());
	selectRows(newSelections);
}

void View::moveSelectedRowsDown()
{
	const auto newSelections = m->model->moveTracksDown(selectedItems());
	selectRows(newSelections);
}

void View::playSelectedTrack()
{
	emit sigDoubleClicked(m->minimumSelectedItem());
}

void View::gotoToCurrentTrack()
{
	gotoRow(m->model->currentTrack());
}

void View::findTrackTriggered()
{
	const auto row = this->currentIndex().row();
	if(row >= 0)
	{
		m->model->findTrack(row);
	}
}

void View::bookmarkTriggered(Seconds timestamp)
{
	const auto row = this->currentIndex().row();
	if(row >= 0)
	{
		emit sigBookmarkPressed(row, timestamp);
	}
}

void View::removeSelectedRows()
{
	const auto minRow = m->minimumSelectedItem();

	m->model->removeTracks(selectedItems());
	clearSelection();

	if(rowCount() > 0)
	{
		const auto newRow = std::min(minRow, rowCount() - 1);
		selectRow(newRow);
	}
}

void View::deleteSelectedTracks()
{
	const auto indexes = selectedItems();

	if(!indexes.isEmpty())
	{
		const auto text = tr("You are about to delete %n file(s)", "", indexes.count()) +
		                  "!\n" +
		                  Lang::get(Lang::Continue).question();

		const auto answer = Message::question_yn(text);
		if(answer == Message::Answer::Yes)
		{
			m->model->deleteTracks(indexes);
		}
	}
}

void View::clear()
{
	clearSelection();
	m->model->clear();
}

MD::Interpretation View::metadataInterpretation() const
{
	return MD::Interpretation::Tracks;
}

MetaDataList View::infoDialogData() const
{
	return m->model->metadata(selectedItems());
}

void View::contextMenuEvent(QContextMenuEvent* e)
{
	if(!m->contextMenu)
	{
		initContextMenu();
	}

	const auto pos = e->globalPos();
	const auto modelIndex = indexAt(e->pos());

	Pl::ContextMenu::Entries entryMask = 0;
	if(rowCount() > 0)
	{
		entryMask = (Pl::ContextMenu::EntryClear | Pl::ContextMenu::EntryRefresh |
		             Pl::ContextMenu::EntryReverse);
	}

	const auto selections = selectedItems();
	if(selections.size() > 0)
	{
		entryMask |= Pl::ContextMenu::EntryInfo;
		entryMask |= Pl::ContextMenu::EntryRemove;
	}

	if(selections.size() == 1)
	{
		entryMask |= (Pl::ContextMenu::EntryLyrics);
	}

	if(m->model->hasLocalMedia(selections))
	{
		entryMask |= Pl::ContextMenu::EntryEdit;
		entryMask |= Pl::ContextMenu::EntryRating;
		entryMask |= Pl::ContextMenu::EntryDelete;

		if(selections.size() == 1)
		{
			const auto track = m->model->metadata(selections.first());
			m->contextMenu->setRating(track.rating());
		}
	}

	if(modelIndex.row() >= 0)
	{
		const auto isCurrentTrack = (m->model->currentTrack() == modelIndex.row());
		const auto track = m->model->metadata(modelIndex.row());
		const auto isLibraryTrack = (track.id() >= 0);

		m->contextMenu->setTrack(track, (isCurrentTrack && isLibraryTrack));

		if(isLibraryTrack)
		{
			entryMask |= Pl::ContextMenu::EntryBookmarks;
			entryMask |= Pl::ContextMenu::EntryFindInLibrary;
		}
	}

	if(m->model->currentTrack() >= 0)
	{
		entryMask |= Pl::ContextMenu::EntryCurrentTrack;
	}

	m->contextMenu->showActions(entryMask);
	m->contextMenu->exec(pos);

	SearchableTableView::contextMenuEvent(e);
}

void View::mousePressEvent(QMouseEvent* event)
{
	SearchableTableView::mousePressEvent(event);
	if(event->button() & Qt::MiddleButton)
	{
		findTrackTriggered();
	}
}

void View::mouseDoubleClickEvent(QMouseEvent* event)
{
	SearchableTableView::mouseDoubleClickEvent(event);

	const auto modelIndex = this->indexAt(event->pos());

	if((modelIndex.flags() & Qt::ItemIsEnabled) &&
	   (modelIndex.flags() & Qt::ItemIsSelectable))
	{
		emit sigDoubleClicked(modelIndex.row());
	}
}

void View::keyPressEvent(QKeyEvent* event)
{
	event->setAccepted(false);
	SearchableTableView::keyPressEvent(event);
}

void View::dragEnterEvent(QDragEnterEvent* event)
{
	event->setAccepted(this->acceptDrops());
}

void View::dragMoveEvent(QDragMoveEvent* event)
{
	QTableView::dragMoveEvent(event);  // needed for autoscroll
	event->setAccepted(this->acceptDrops());

	const auto row = m->calcDragDropLine(event->pos());
	m->model->setDragIndex(row);
}

void View::dragLeaveEvent(QDragLeaveEvent* event)
{
	event->accept();
	m->model->setDragIndex(-1);
}

void View::dropEventFromOutside(QDropEvent* event)
{
	dropEvent(event);
}

void View::playlistBusyChanged(bool b)
{
	this->setDisabled(b);

	if(b)
	{
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
		m->progressbar->hide();
		m->currentFileLabel->hide();

		this->setDragDropMode(QAbstractItemView::DragDrop);
		this->setAcceptDrops(true);
		this->setFocus();
	}
}

void View::currentScannedFileChanged(const QString& currentFile)
{
	auto offsetBottom = this->fontMetrics().height() + 3;
	if(m->progressbar->isVisible())
	{
		offsetBottom += m->progressbar->height() + 2;
	}

	m->currentFileLabel->setText(currentFile);
	m->currentFileLabel->setGeometry(0,
	                                 this->height() - offsetBottom,
	                                 this->width(),
	                                 this->fontMetrics().height() + 4);
	m->currentFileLabel->show();
}

void View::dropEvent(QDropEvent* event)
{
	event->setAccepted(this->acceptDrops());
	if(this->acceptDrops())
	{
		handleDrop(event);
	}
}

int View::mapModelIndexToIndex(const QModelIndex& idx) const
{
	return idx.row();
}

ModelIndexRange View::mapIndexToModelIndexes(int idx) const
{
	return ModelIndexRange(m->model->index(idx, 0),
	                       m->model->index(idx, m->model->columnCount() - 1));
}

bool View::viewportEvent(QEvent* event)
{
	const auto success = SearchableTableView::viewportEvent(event);
	if(event->type() == QEvent::Resize)
	{
		refresh();
	}
	return success;
}

void View::skinChanged()
{
	SearchableTableView::skinChanged();
	refresh();
}

void View::columnsChanged()
{
	const auto showNumber = GetSetting(Set::PL_ShowNumbers);
	const auto showCovers = GetSetting(Set::PL_ShowCovers);

	horizontalHeader()->setSectionHidden(Pl::Model::ColumnName::TrackNumber, !showNumber);
	horizontalHeader()->setSectionHidden(Pl::Model::ColumnName::Cover, !showCovers);

	refresh();
}

void View::showRatingChanged()
{
	const auto editTrigger = (GetSetting(Set::PL_ShowRating))
	                         ? QAbstractItemView::SelectedClicked
	                         : QAbstractItemView::NoEditTriggers;

	this->setEditTriggers(editTrigger);

	refresh();
}

void View::refresh()
{
	const auto fm = this->fontMetrics();
	auto viewRowHeight = std::max(fm.height() + 4, Gui::Util::viewRowHeight());

	if(GetSetting(Set::PL_ShowRating))
	{
		viewRowHeight += fm.height();
	}

	for(int i = 0; i < rowCount(); i++)
	{
		verticalHeader()->resizeSection(i, viewRowHeight);
	}

	auto viewportWidth = viewport()->width();
	if(GetSetting(Set::PL_ShowCovers))
	{
		const auto widthCover = viewRowHeight;
		viewportWidth -= widthCover;

		m->resizeSection(+Pl::Model::ColumnName::Cover, widthCover);
	}

	if(GetSetting(Set::PL_ShowNumbers))
	{
		const auto widthTrackNumber = Gui::Util::textWidth(fm, QString::number(rowCount() * 100));
		viewportWidth -= widthTrackNumber;

		m->resizeSection(+Pl::Model::ColumnName::TrackNumber, widthTrackNumber);
	}

	const auto widthTime = Gui::Util::textWidth(fm, "1888:88");
	m->resizeSection(+Pl::Model::ColumnName::Time, widthTime);
	m->resizeSection(+Pl::Model::ColumnName::Description, viewportWidth - widthTime);

	m->model->setRowHeight(viewRowHeight);
}
