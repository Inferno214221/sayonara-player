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
#include <QHeaderView>
#include <QScrollBar>
#include <QTimer>
#include <QLabel>

namespace
{
	template<typename Callback>
	void createShortcut(const QKeySequence& ks, Playlist::View* view, Callback callback)
	{
		auto* shortcut = new QShortcut(ks, view);
		shortcut->setContext(Qt::WidgetShortcut);
		view->connect(shortcut, &QShortcut::activated, view, callback);
	}

	int minimumSelectedItem(Playlist::View* view)
	{
		const auto selected = view->selectedItems();
		const auto it = std::min_element(selected.cbegin(), selected.cend());

		return (it == selected.cend()) ? -1 : *it;
	}

	int resizeSection(int column, int size, QHeaderView* header)
	{
		if(header && (header->sectionSize(column) != size))
		{
			header->resizeSection(column, size);
		}

		return header->sectionSize(column);
	}

	int calcDragDropLine(const QPoint& pos, Playlist::View* view)
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

	int resizeCoverSection(int coverWidth, QHeaderView* horizontalHeader)
	{
		return (GetSetting(Set::PL_ShowCovers))
		       ? resizeSection(+Playlist::Model::ColumnName::Cover, coverWidth, horizontalHeader)
		       : 0;
	}

	int resizeNumberSection(const QFontMetrics& fontMetrics, int maxRows, QHeaderView* horizontalHeader)
	{
		if(GetSetting(Set::PL_ShowNumbers))
		{
			const auto width = Gui::Util::textWidth(fontMetrics, QString::number(maxRows * 100));
			return resizeSection(+Playlist::Model::ColumnName::TrackNumber, width, horizontalHeader);
		}

		return 0;
	}

	int resizeTimeSection(const QFontMetrics& fontMetrics, QHeaderView* horizontalHeader)
	{
		const auto widthTime = Gui::Util::textWidth(fontMetrics, "1888:88");
		return resizeSection(+Playlist::Model::ColumnName::Time, widthTime, horizontalHeader);
	}
}

namespace Playlist
{
	struct View::Private
	{
		DynamicPlaybackChecker* dynamicPlaybackChecker;
		ContextMenu* contextMenu = nullptr;
		Model* model;
		Gui::ProgressBar* progressbar;
		QLabel* currentFileLabel;

		Private(PlaylistCreator* playlistCreator, PlaylistPtr playlist, DynamicPlaybackChecker* dynamicPlaybackChecker,
		        View* view) :
			dynamicPlaybackChecker(dynamicPlaybackChecker),
			model(new Model(playlistCreator, playlist, view)),
			progressbar(new Gui::ProgressBar(view)),
			currentFileLabel(new QLabel(view))
		{
			view->setObjectName(QString("playlist_view%1").arg(playlist->index()));
			view->setSearchableModel(this->model);
			view->setItemDelegate(new Delegate(view));

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
			view->verticalHeader()->setMinimumSectionSize(10);
			view->horizontalHeader()->hide();
			view->horizontalHeader()->setMinimumSectionSize(10);

			this->progressbar->hide();
			this->currentFileLabel->hide();
		}
	};

	View::View(PlaylistCreator* playlistCreator, PlaylistPtr playlist, DynamicPlaybackChecker* dynamicPlaybackChecker,
	           QWidget* parent) :
		SearchableTableView(parent),
		InfoDialogContainer(),
		Gui::Dragable(this)
	{
		m = Pimpl::make<Private>(playlistCreator, playlist, dynamicPlaybackChecker, this);

		initShortcuts();

		ListenSetting(Set::PL_ShowRating, View::showRatingChanged);
		ListenSetting(Set::PL_ShowNumbers, View::columnsChanged);
		ListenSettingNoCall(Set::PL_ShowCovers, View::columnsChanged);

		connect(m->model, &Model::sigDataReady, this, &View::refresh);
		connect(m->model, &Model::sigCurrentTrackChanged, this, &View::currentTrackChanged);
		connect(m->model, &Model::sigBusyChanged, this, &View::playlistBusyChanged);
		connect(m->model, &Model::sigCurrentScannedFileChanged, this, &View::currentScannedFileChanged);

		QTimer::singleShot(100, this, &View::jumpToCurrentTrack);
	}

	View::~View() = default;

	void View::initShortcuts()
	{
		createShortcut(QKeySequence(Qt::Key_Backspace), this, &View::clear);
		createShortcut(QKeySequence(QKeySequence::Delete), this, &View::removeSelectedRows);
		createShortcut(QKeySequence(Qt::ControlModifier + Qt::Key_Up), this, &View::moveSelectedRowsUp);
		createShortcut(QKeySequence(Qt::ControlModifier + Qt::Key_Down), this, &View::moveSelectedRowsDown);
		createShortcut(QKeySequence(Qt::ControlModifier + Qt::Key_J), this, &View::jumpToCurrentTrack);
		createShortcut(QKeySequence(Qt::ControlModifier + Qt::Key_G), this, &View::findTrackTriggered);
		createShortcut(QKeySequence(Qt::Key_Return), this, &View::playSelectedTrack);
		createShortcut(QKeySequence(Qt::Key_Enter), this, &View::playSelectedTrack);
	}

	void View::initContextMenu()
	{
		m->contextMenu = new ContextMenu(m->dynamicPlaybackChecker, this);
		m->contextMenu->addPreferenceAction(new Gui::PlaylistPreferenceAction(m->contextMenu));

		connect(m->contextMenu, &ContextMenu::sigRefreshClicked, m->model, &Model::refreshData);
		connect(m->contextMenu, &ContextMenu::sigReverseTriggered, m->model, &Model::reverseTracks);
		connect(m->contextMenu, &ContextMenu::sigEditClicked, this, [&]() { showEdit(); });
		connect(m->contextMenu, &ContextMenu::sigInfoClicked, this, [&]() { showInfo(); });
		connect(m->contextMenu, &ContextMenu::sigLyricsClicked, this, [&]() { showLyrics(); });
		connect(m->contextMenu, &ContextMenu::sigDeleteClicked, this, &View::deleteSelectedTracks);
		connect(m->contextMenu, &ContextMenu::sigRemoveClicked, this, &View::removeSelectedRows);
		connect(m->contextMenu, &ContextMenu::sigClearClicked, this, &View::clear);
		connect(m->contextMenu, &ContextMenu::sigRatingChanged, this, &View::ratingChanged);
		connect(m->contextMenu, &ContextMenu::sigJumpToCurrentTrack, this, &View::jumpToCurrentTrack);
		connect(m->contextMenu, &ContextMenu::sigBookmarkPressed, this, &View::bookmarkTriggered);
		connect(m->contextMenu, &ContextMenu::sigFindTrackTriggered, this, &View::findTrackTriggered);
	}

	void View::gotoRow(int row)
	{
		if(Util::between(row, rowCount()))
		{
			const auto range = mapIndexToModelIndexes(row);
			this->scrollTo(range.first);
		}
	}

	void View::handleDrop(QDropEvent* event)
	{
		m->model->setDragIndex(-1);

		const auto* mimeData = event->mimeData();
		const auto dragDropLine = calcDragDropLine(event->pos(), this);

		if(Gui::MimeData::isInnerDragDrop(mimeData, m->model->playlistIndex()))
		{
			if(const auto selectedRows = selectedItems(); !selectedRows.contains(dragDropLine))
			{
				const auto newSelection = (event->keyboardModifiers() & Qt::ControlModifier)
				                          ? m->model->copyTracks(selectedRows, dragDropLine + 1)
				                          : m->model->moveTracks(selectedRows, dragDropLine + 1);

				this->selectRows(newSelection, 0);
			}

			return;
		}

		if(auto* asyncDropHandler = Gui::MimeData::asyncDropHandler(mimeData); asyncDropHandler)
		{
			m->model->setBusy(true);

			asyncDropHandler->setTargetIndex(dragDropLine + 1);
			connect(asyncDropHandler, &Gui::AsyncDropHandler::sigFinished, this, &View::asyncDropFinished);
			asyncDropHandler->start();
		}

		else if(Gui::MimeData::hasMetadata(mimeData))
		{
			const auto tracks = Gui::MimeData::metadata(mimeData);
			m->model->insertTracks(tracks, dragDropLine + 1);
		}
	}

	void View::asyncDropFinished()
	{
		auto* asyncDropHandler = static_cast<Gui::AsyncDropHandler*>(sender());
		m->model->setBusy(false);

		if(const auto tracks = asyncDropHandler->tracks(); !tracks.isEmpty())
		{
			m->model->insertTracks(tracks, asyncDropHandler->targetIndex());
		}

		asyncDropHandler->deleteLater();
	}

	void View::ratingChanged(Rating rating)
	{
		if(const auto selections = selectedItems(); !selections.isEmpty())
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
		m->model->changeTrack(minimumSelectedItem(this));
	}

	void View::jumpToCurrentTrack()
	{
		gotoRow(m->model->currentTrack());
		selectRow(m->model->currentTrack());
	}

	void View::currentTrackChanged(int index)
	{
		if(GetSetting(Set::PL_JumpToCurrentTrack) && (index >= 0))
		{
			gotoRow(index);
		}
	}

	void View::findTrackTriggered()
	{
		if(const auto row = currentIndex().row(); row >= 0)
		{
			m->model->findTrack(row);
		}
	}

	void View::bookmarkTriggered(Seconds timestamp)
	{
		m->model->changeTrack(currentIndex().row(), timestamp);
	}

	void View::removeSelectedRows()
	{
		const auto minRow = minimumSelectedItem(this);

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
		if(const auto indexes = selectedItems(); !indexes.isEmpty())
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

	QWidget* View::getParentWidget()
	{
		return this;
	}

	void View::contextMenuEvent(QContextMenuEvent* e)
	{
		if(!m->contextMenu)
		{
			initContextMenu();
		}

		m->contextMenu->clearTrack();

		ContextMenu::Entries entryMask = ContextMenu::EntryNone;
		if(rowCount() > 0)
		{
			entryMask |= (ContextMenu::EntryClear |
			              ContextMenu::EntryRefresh |
			              ContextMenu::EntryReverse);

			if(const auto selections = selectedItems(); !selections.isEmpty())
			{
				entryMask |= (ContextMenu::EntryInfo |
				              ContextMenu::EntryRemove);

				if(selections.size() == 1)
				{
					const auto& selectedRow = *selections.begin();
					const auto& track = m->model->metadata(selectedRow);

					entryMask |= m->contextMenu->setTrack(track, (selectedRow == m->model->currentTrack()));
				}

				if(m->model->hasLocalMedia(selections))
				{
					entryMask |= (ContextMenu::EntryEdit |
					              ContextMenu::EntryDelete);
				}
			}

			if(m->model->currentTrack() >= 0)
			{
				entryMask |= ContextMenu::EntryCurrentTrack;
			}
		}

		m->contextMenu->showActions(entryMask);
		m->contextMenu->exec(e->globalPos());

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
			m->model->changeTrack(modelIndex.row());
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

		const auto row = calcDragDropLine(event->pos(), this);
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

	void View::playlistBusyChanged(bool isBusy)
	{
		m->progressbar->setVisible(isBusy);
		m->currentFileLabel->setVisible(isBusy);

		auto* focusObject = isBusy
		                    ? static_cast<QWidget*>(m->progressbar)
		                    : static_cast<QWidget*>(this);

		focusObject->setDisabled(false);
		focusObject->setFocus();
		this->setDisabled(isBusy);

		this->setAcceptDrops(!isBusy);
		this->setDragDropMode(isBusy ? QAbstractItemView::NoDragDrop : QAbstractItemView::DragDrop);
	}

	void View::currentScannedFileChanged(const QString& currentFile)
	{
		const auto baseHeight = Gui::Util::viewRowHeight(fontMetrics());
		const auto offsetBottom = (m->progressbar->isVisible())
		                          ? baseHeight + m->progressbar->height() + 2
		                          : baseHeight;

		const auto geometry = QRect(0, height() - offsetBottom, width(), baseHeight);

		m->currentFileLabel->setGeometry(geometry);
		m->currentFileLabel->setText(currentFile);
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

		horizontalHeader()->setSectionHidden(Model::ColumnName::TrackNumber, !showNumber);
		horizontalHeader()->setSectionHidden(Model::ColumnName::Cover, !showCovers);

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
		const auto fm = fontMetrics();
		const auto baseRowHeight = Gui::Util::viewRowHeight(fm);
		const auto viewRowHeight = GetSetting(Set::PL_ShowRating)
		                           ? baseRowHeight + fm.height()
		                           : baseRowHeight;

		for(auto row = 0; row < rowCount(); row++)
		{
			resizeSection(row, viewRowHeight, verticalHeader());
		}

		auto viewportWidth = viewport()->width();
		viewportWidth -= resizeNumberSection(fm, rowCount(), horizontalHeader());
		viewportWidth -= resizeCoverSection(viewRowHeight, horizontalHeader());
		viewportWidth -= resizeTimeSection(fm, horizontalHeader());

		resizeSection(+Model::ColumnName::Description, viewportWidth, horizontalHeader());

		this->setIconSize(QSize(viewRowHeight - 2, viewRowHeight - 2));
	}

	void View::searchDone()
	{
		if(GetSetting(Set::PL_PlayTrackAfterSearch) && (minimumSelectedItem(this) >= 0))
		{
			playSelectedTrack();
		}

	}
}
