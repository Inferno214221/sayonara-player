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

namespace Playlist
{
	namespace
	{
		template<typename Callback>
		void createShortcut(const QKeySequence& ks, View* view, Callback callback)
		{
			auto* shortcut = new QShortcut(ks, view);
			shortcut->setContext(Qt::WidgetShortcut);
			view->connect(shortcut, &QShortcut::activated, view, callback);
		}

		int minimumSelectedItem(View* view)
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

		int calcDragDropLine(const QPoint& pos, View* view)
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

		int resizeCoverSection(const int coverWidth, QHeaderView* horizontalHeader)
		{
			return (GetSetting(Set::PL_ShowCovers))
			       ? resizeSection(static_cast<int>(Model::ColumnName::Cover), coverWidth, horizontalHeader)
			       : 0;
		}

		int resizeNumberSection(const QFontMetrics& fontMetrics, const int maxRows, QHeaderView* horizontalHeader)
		{
			if(GetSetting(Set::PL_ShowNumbers))
			{
				const auto width = Gui::Util::textWidth(fontMetrics, QString::number(maxRows * 100));
				return resizeSection(static_cast<int>(Model::ColumnName::TrackNumber), width, horizontalHeader);
			}

			return 0;
		}

		int resizeTimeSection(const QFontMetrics& fontMetrics, QHeaderView* horizontalHeader)
		{
			const auto widthTime = Gui::Util::textWidth(fontMetrics, "1888:88");
			return resizeSection(static_cast<int>(Model::ColumnName::Time), widthTime, horizontalHeader);
		}

		void setupContextMenuItems(ContextMenu* contextMenu, Model* model, const Util::Set<int>& selectedItems)
		{
			auto entryMask = ContextMenu::Entries {ContextMenu::EntryNone};
			if(model->rowCount() > 0)
			{
				entryMask |= (ContextMenu::EntryClear |
				              ContextMenu::EntryRefresh |
				              ContextMenu::EntryReverse |
				              ContextMenu::EntryRandomize |
				              ContextMenu::EntryJumpToNextAlbum);

				if(!selectedItems.isEmpty())
				{
					entryMask |= (ContextMenu::EntryPlay |
					              ContextMenu::EntryInfo |
					              ContextMenu::EntryRemove);

					if(selectedItems.size() == 1)
					{
						const auto& selectedRow = *selectedItems.begin();
						const auto& track = model->metadata(selectedRow);

						entryMask |= contextMenu->setTrack(track, (selectedRow == model->currentTrack()));
					}

					if(model->hasLocalMedia(selectedItems))
					{
						entryMask |= (ContextMenu::EntryEdit |
						              ContextMenu::EntryDelete);
					}
				}

				if(model->currentTrack() >= 0)
				{
					entryMask |= ContextMenu::EntryCurrentTrack;
				}
			}

			contextMenu->showActions(entryMask);
		}

		void initView(View* view, Model* model, Delegate* delegate, const int playlistIndex)
		{
			view->setObjectName(QString("playlist_view%1").arg(playlistIndex));
			view->setSearchableModel(model);
			view->setItemDelegate(delegate);
			view->setTabKeyNavigation(false);
			view->setSelectionMode(QAbstractItemView::ExtendedSelection);
			view->setAlternatingRowColors(true);
			view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
			view->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
			view->setSelectionBehavior(QAbstractItemView::SelectRows);
			view->setShowGrid(false);
			view->setAutoScroll(true);
			view->setAutoScrollMargin(50); // NOLINT(readability-magic-numbers)
			view->setDragEnabled(true);
			view->setDragDropMode(QAbstractItemView::DragDrop);
			view->setDragDropOverwriteMode(false);
			view->setAcceptDrops(true);
			view->setDropIndicatorShown(true);

			view->verticalHeader()->hide();
			view->verticalHeader()->setMinimumSectionSize(10); // NOLINT(readability-magic-numbers)
			view->horizontalHeader()->hide();
			view->horizontalHeader()->setMinimumSectionSize(10); // NOLINT(readability-magic-numbers)
		}
	} // namespace

	struct View::Private
	{
		DynamicPlaybackChecker* dynamicPlaybackChecker;
		ContextMenu* contextMenu = nullptr;
		Model* model;
		Gui::ProgressBar* progressbar;
		QLabel* currentFileLabel;

		Private(PlaylistCreator* playlistCreator, const PlaylistPtr& playlist,
		        DynamicPlaybackChecker* dynamicPlaybackChecker, View* view) :
			dynamicPlaybackChecker(dynamicPlaybackChecker),
			model(new Model(playlistCreator, playlist, view)),
			progressbar(new Gui::ProgressBar(view)),
			currentFileLabel(new QLabel(view)) {}
	};

	View::View(PlaylistCreator* playlistCreator, const PlaylistPtr& playlist,
	           DynamicPlaybackChecker* dynamicPlaybackChecker, QWidget* parent) :
		SearchableTableView(parent),
		Gui::Dragable(this)
	{
		m = Pimpl::make<Private>(playlistCreator, playlist, dynamicPlaybackChecker, this);

		initView(this, m->model, new Delegate(this), playlist->index());

		m->progressbar->hide();
		m->currentFileLabel->hide();

		initContextMenu();

		ListenSetting(Set::PL_ShowRating, View::showRatingChanged);
		ListenSetting(Set::PL_ShowNumbers, View::columnsChanged);
		ListenSettingNoCall(Set::PL_ShowCovers, View::columnsChanged);

		connect(m->model, &Model::sigDataReady, this, &View::refresh);
		connect(m->model, &Model::sigCurrentTrackChanged, this, &View::currentTrackChanged);
		connect(m->model, &Model::sigBusyChanged, this, &View::playlistBusyChanged);
		connect(m->model, &Model::sigCurrentScannedFileChanged, this, &View::currentScannedFileChanged);

		connect(selectionModel(), &QItemSelectionModel::selectionChanged, this, [&]() {
			setupContextMenuItems(m->contextMenu, m->model, selectedItems());
		});

		createShortcut(QKeySequence(Qt::ControlModifier + Qt::Key_Up), this, &View::moveSelectedRowsUp);
		createShortcut(QKeySequence(Qt::ControlModifier + Qt::Key_Down), this, &View::moveSelectedRowsDown);

		QTimer::singleShot(100, this, &View::jumpToCurrentTrack); // NOLINT(readability-magic-numbers)
	}

	View::~View() = default;

	void View::initContextMenu()
	{
		m->contextMenu = new ContextMenu(m->dynamicPlaybackChecker, this);
		m->contextMenu->addPreferenceAction(new Gui::PlaylistPreferenceAction(m->contextMenu));

		connect(m->contextMenu, &ContextMenu::sigPlayClicked, this, &View::playSelectedTrack);
		connect(m->contextMenu, &ContextMenu::sigRefreshClicked, m->model, &Model::refreshData);
		connect(m->contextMenu, &ContextMenu::sigEditClicked, this, [&]() { showEdit(); });
		connect(m->contextMenu, &ContextMenu::sigInfoClicked, this, [&]() { showInfo(); });
		connect(m->contextMenu, &ContextMenu::sigLyricsClicked, this, [&]() { showLyrics(); });
		connect(m->contextMenu, &ContextMenu::sigDeleteClicked, this, &View::deleteSelectedTracks);
		connect(m->contextMenu, &ContextMenu::sigRemoveClicked, this, &View::removeSelectedRows);
		connect(m->contextMenu, &ContextMenu::sigClearClicked, this, &View::clear);
		connect(m->contextMenu, &ContextMenu::sigBookmarkTriggered, this, &View::bookmarkTriggered);
		connect(m->contextMenu->action(ContextMenu::EntryRating), &QAction::triggered, this, &View::ratingChanged);
		connect(m->contextMenu->action(ContextMenu::EntryReverse), &QAction::triggered,
		        m->model, &Model::reverseTracks);
		connect(m->contextMenu->action(ContextMenu::EntryRandomize), &QAction::triggered,
		        m->model, &Model::randomizeTracks);
		connect(m->contextMenu->action(ContextMenu::EntryCurrentTrack), &QAction::triggered,
		        this, &View::jumpToCurrentTrack);
		connect(m->contextMenu->action(ContextMenu::EntryFindInLibrary), &QAction::triggered, this, [&]() {
			m->model->findTrack(currentIndex().row());
		});
		connect(m->contextMenu->action(ContextMenu::EntryJumpToNextAlbum), &QAction::triggered,
		        m->model, &Model::jumpToNextAlbum);
	}

	void View::gotoRow(const int row)
	{
		if(Util::between(row, rowCount()))
		{
			const auto range = mapIndexToModelIndexes(row);
			scrollTo(range.first);
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

				selectRows(newSelection, 0);
			}
		}

		else if(auto* asyncDropHandler = Gui::MimeData::asyncDropHandler(mimeData); asyncDropHandler)
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
		auto* asyncDropHandler = dynamic_cast<Gui::AsyncDropHandler*>(sender());
		m->model->setBusy(false);

		if(const auto tracks = asyncDropHandler->tracks(); !tracks.isEmpty())
		{
			m->model->insertTracks(tracks, asyncDropHandler->targetIndex());
		}

		asyncDropHandler->deleteLater();
	}

	void View::ratingChanged()
	{
		auto* action = dynamic_cast<QAction*>(sender());
		if(const auto selections = selectedItems(); !selections.isEmpty())
		{
			const auto rating = action->property("rating").value<Rating>();
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
		if(minimumSelectedItem(this) >= 0)
		{
			m->model->changeTrack(minimumSelectedItem(this));
		}
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
			const auto text = tr("You are about to delete %n file(s)", "", indexes.count()) + "!\n" +
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

	MD::Interpretation View::metadataInterpretation() const { return MD::Interpretation::Tracks; }

	MetaDataList View::infoDialogData() const { return m->model->metadata(selectedItems()); }

	QWidget* View::getParentWidget() { return this; }

	void View::contextMenuEvent(QContextMenuEvent* e)
	{
		m->contextMenu->clearTrack();

		setupContextMenuItems(m->contextMenu, m->model, selectedItems());

		m->contextMenu->exec(e->globalPos());

		SearchableTableView::contextMenuEvent(e);
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

		setDisabled(isBusy);
		setAcceptDrops(!isBusy);
		setDragDropMode(isBusy ? QAbstractItemView::NoDragDrop : QAbstractItemView::DragDrop);
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
		if(acceptDrops())
		{
			handleDrop(event);
		}
	}

	int View::mapModelIndexToIndex(const QModelIndex& idx) const { return idx.row(); }

	ModelIndexRange View::mapIndexToModelIndexes(const int index) const
	{
		auto minimumColumn = 0;
		auto maximumColumn = m->model->columnCount() - 1;
		while(isColumnHidden(minimumColumn))
		{
			minimumColumn++;
		}
		while(isColumnHidden(maximumColumn))
		{
			maximumColumn--;
		}

		return {m->model->index(index, minimumColumn), m->model->index(index, maximumColumn)};
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

		setEditTriggers(editTrigger);

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

		resizeSection(static_cast<int>(Model::ColumnName::Description), viewportWidth, horizontalHeader());

		setIconSize(QSize(viewRowHeight - 2, viewRowHeight - 2));
		setupContextMenuItems(m->contextMenu, m->model, selectedItems());
	}

	void View::searchDone()
	{
		if(GetSetting(Set::PL_PlayTrackAfterSearch))
		{
			playSelectedTrack();
		}
	}
} // Playlist
