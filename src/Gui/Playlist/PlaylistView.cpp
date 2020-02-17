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


/*
 * PlaylistView.cpp
 *
 *  Created on: Jun 26, 2011
 *      Author: Michael Lugmair (Lucio Carreras)
 */

#include "PlaylistView.h"
#include "PlaylistModel.h"
#include "PlaylistDelegate.h"
#include "PlaylistContextMenu.h"

#include "Gui/Utils/GuiUtils.h"
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
namespace FileUtils=::Util::File;
using Pl::View;

struct View::Private
{
	View*					view=nullptr;
	PlaylistPtr				playlist;
	Pl::ContextMenu*		contextMenu=nullptr;
	Pl::Model*				model=nullptr;
	ProgressBar*			progressbar=nullptr;
	QLabel*					currentFileLabel=nullptr;

	Private(PlaylistPtr pl, View* parent) :
		view(parent),
		playlist(pl),
		model(new Pl::Model(pl, parent))
	{}

	int minimumSelectedItem()
	{
		const IndexSet selected = view->selectedItems();
		auto it = std::min_element(selected.begin(), selected.end());
		if(it == selected.end()){
			return -1;
		}

		return *it;
	}
};

View::View(PlaylistPtr pl, QWidget* parent) :
	SearchableTableView(parent),
	InfoDialogContainer(),
	Gui::Dragable(this)
{
	m = Pimpl::make<Private>(pl, this);

	auto* playlist_handler = Pl::Handler::instance();

	this->setObjectName("playlist_view" + QString::number(pl->index()));
	this->setSearchableModel(m->model);
	this->setItemDelegate(new Pl::Delegate(this));
	this->horizontalHeader()->setMinimumSectionSize(10);

	initView();

	ListenSetting(Set::PL_ShowNumbers, View::columnsChanged);
	ListenSetting(Set::PL_ShowCovers, View::columnsChanged);
	ListenSetting(Set::PL_ShowNumbers, View::columnsChanged);
	ListenSetting(Set::PL_ShowRating, View::showRatingChanged);

	new QShortcut(QKeySequence(Qt::Key_Backspace), this, SLOT(clear()), nullptr, Qt::WidgetShortcut);
	new QShortcut(QKeySequence(QKeySequence::Delete), this, SLOT(removeSelectedRows()), nullptr, Qt::WidgetShortcut);
	new QShortcut(QKeySequence(Qt::ControlModifier + Qt::Key_Up), this, SLOT(moveSelectedRowsUp()), nullptr, Qt::WidgetShortcut);
	new QShortcut(QKeySequence(Qt::ControlModifier + Qt::Key_Down), this, SLOT(moveSelectedRowsDown()), nullptr, Qt::WidgetShortcut);
	new QShortcut(QKeySequence(Qt::Key_Return), this, SLOT(playSelectedTrack()), nullptr, Qt::WidgetShortcut);
	new QShortcut(QKeySequence(Qt::Key_Enter), this, SLOT(playSelectedTrack()), nullptr, Qt::WidgetShortcut);

	connect(m->model, &Pl::Model::sigDataReady, this, &View::refresh);
	connect(playlist_handler, &Pl::Handler::sigCurrentTrackChanged, this, &View::currentTrackChanged);
	connect(pl.get(), &Playlist::sigBusyChanged, this, &View::playlistBusyChanged);
	connect(pl.get(), &Playlist::sigCurrentScannedFileChanged, this, &View::currentScannedFileChanged);

	QTimer::singleShot(100, this, [=](){
		this->gotoToCurrentTrack();
	});
}

View::~View() = default;

void View::initView()
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


void View::initContextMenu()
{
	using Pl::ContextMenu;

	if(m->contextMenu){
		return;
	}

	m->contextMenu = new ContextMenu(this);

	connect(m->contextMenu, &ContextMenu::sigRefreshClicked, m->model, &Pl::Model::refreshData);
	connect(m->contextMenu, &ContextMenu::sigEditClicked, this, [=](){ showEdit(); });
	connect(m->contextMenu, &ContextMenu::sigInfoClicked, this, [=](){ showInfo(); });
	connect(m->contextMenu, &ContextMenu::sigLyricsClicked, this, [=](){ showLyrics(); });
	connect(m->contextMenu, &ContextMenu::sigDeleteClicked, this, &View::deleteSelectedTracks);
	connect(m->contextMenu, &ContextMenu::sigRemoveClicked, this, &View::removeSelectedRows);
	connect(m->contextMenu, &ContextMenu::sigClearClicked, this, &View::clear);
	connect(m->contextMenu, &ContextMenu::sigRatingChanged, this, &View::ratingChanged);
	connect(m->contextMenu, &ContextMenu::sigJumpToCurrentTrack, this, &View::gotoToCurrentTrack);
	connect(m->contextMenu, &ContextMenu::sigBookmarkPressed, this, &View::bookmarkTriggered);
	connect(m->contextMenu, &ContextMenu::sigFindTrackTriggered, this, &View::findTrackTriggered);
	connect(m->contextMenu, &ContextMenu::sigReverseTriggered, this, &View::reverseTriggered);

	m->contextMenu->addPreferenceAction(new PlaylistPreferenceAction(m->contextMenu));
}


void View::gotoRow(int row)
{
	row = std::min(row, rowCount() - 1);
	row = std::max(row, 0);

	ModelIndexRange range = mapIndexToModelIndexes(row);
	this->scrollTo(range.first);
}

int View::calcDragDropLine(QPoint pos)
{
	int offset = (rowCount() > 0) ? this->rowHeight(0) / 2 : 10;
	if(pos.y() < offset) {
		return -1;
	}

	int row = this->indexAt(pos).row();
	if(row < 0) {
		row = rowCount() - 1;
	}

	return row;
}

void View::handleDrop(QDropEvent* event)
{
	int row = calcDragDropLine(event->pos());

	m->model->setDragIndex(-1);

	const QMimeData* mimedata = event->mimeData();
	if(!mimedata) {
		return;
	}

	bool is_inner_drag_drop = MimeData::isInnerDragDrop(mimedata, m->playlist->index());
	if(is_inner_drag_drop)
	{
		bool copy = (event->keyboardModifiers() & Qt::ControlModifier);
		handleInnerDragDrop(row, copy);
		return;
	}

	const MetaDataList v_md = MimeData::metadata(mimedata);
	if(!v_md.isEmpty())
	{
		m->model->insertTracks(v_md, row + 1);
	}

	const QList<QUrl> urls = mimedata->urls();
	if(!urls.isEmpty())
	{
		QStringList files;
		bool www = FileUtils::isWWW(urls.first().toString());
		if(www)
		{
			m->playlist->setBusy(true);

			for(const QUrl& url : urls)
			{
				files << url.toString();
			}

			auto* streamParser = new StreamParser();
			streamParser->setCoverUrl(MimeData::coverUrl(mimedata));

			connect(streamParser, &StreamParser::sigFinished, this, [=](bool success){
				asyncDropFinished(success, row);
			});

			streamParser->parse(files);
		}

		else if(v_md.isEmpty())
		{
			for(const QUrl& url : urls)
			{
				if(url.isLocalFile()){
					files << url.toLocalFile();
				}
			}

			Handler::instance()->insertTracks(files, row + 1, m->playlist->index());
		}
	}
}


void View::asyncDropFinished(bool success, int async_drop_index)
{
	m->playlist->setBusy(false);

	auto* stream_parser = dynamic_cast<StreamParser*>(sender());
	if(success)
	{
		MetaDataList v_md = stream_parser->tracks();
		m->model->insertTracks(v_md, async_drop_index+1);
	}

	stream_parser->deleteLater();
}


void View::handleInnerDragDrop(int row, bool copy)
{
	IndexSet newSelectedRows;
	IndexSet curSelectedRows = selectedItems();
	if( curSelectedRows.contains(row) ) {
		return;
	}

	if(copy)
	{
		newSelectedRows = m->model->copyTracks(curSelectedRows, row + 1);
	}

	else
	{
		newSelectedRows = m->model->moveTracks(curSelectedRows, row + 1);
	}

	this->selectRows(newSelectedRows, 0);
}


void View::ratingChanged(Rating rating)
{
	IndexSet selections = selectedItems();
	if(selections.isEmpty()){
		return;
	}

	m->model->changeRating(selectedItems(), rating);
}


void View::moveSelectedRowsUp()
{
	IndexSet selections = selectedItems();
	IndexSet newSelectionss = m->model->moveTracksUp(selections);
	selectRows(newSelectionss);
}

void View::moveSelectedRowsDown()
{
	IndexSet selections = selectedItems();
	IndexSet newSelectionss = m->model->moveTracksDown(selections);
	selectRows(newSelectionss);
}

void View::playSelectedTrack()
{
	int minRow = -1;

	const IndexSet selected = selectedItems();
	if(!selected.isEmpty()){
		minRow = *std::min_element(selected.begin(), selected.end());
	}

	emit sigDoubleClicked(minRow);
}

void View::gotoToCurrentTrack()
{
	gotoRow(m->model->currentTrack());
}

void View::findTrackTriggered()
{
	int row = this->currentIndex().row();
	if(row >= 0){
		m->playlist->findTrack(row);
	}
}

void View::reverseTriggered()
{
	m->playlist->reverse();
}

void View::bookmarkTriggered(Seconds timestamp)
{
	int row = this->currentIndex().row();
	if(row >= 0){
		emit sigBookmarkPressed(row, timestamp);
	}
}

void View::removeSelectedRows()
{
	int minRow = m->minimumSelectedItem();

	m->model->removeTracks(selectedItems());
	clearSelection();

	if(rowCount() > 0)
	{
		minRow = std::min(minRow, rowCount() - 1);
		selectRow(minRow);
	}
}

void View::deleteSelectedTracks()
{
	IndexSet selections = selectedItems();
	emit sigDeleteTracks(selections);
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
	if(!m->contextMenu){
		initContextMenu();
	}

	QPoint pos = e->globalPos();
	QModelIndex idx = indexAt(e->pos());

	Pl::ContextMenu::Entries entry_mask = 0;
	if(rowCount() > 0)	{
		entry_mask = (Pl::ContextMenu::EntryClear | Pl::ContextMenu::EntryRefresh | Pl::ContextMenu::EntryReverse);
	}

	IndexSet selections = selectedItems();
	if(selections.size() > 0)
	{
		entry_mask |= Pl::ContextMenu::EntryInfo;
		entry_mask |= Pl::ContextMenu::EntryRemove;
	}

	if(selections.size() == 1)
	{
		entry_mask |= (Pl::ContextMenu::EntryLyrics);
	}

	if(m->model->hasLocalMedia(selections) )
	{
		entry_mask |= Pl::ContextMenu::EntryEdit;
		entry_mask |= Pl::ContextMenu::EntryRating;
		entry_mask |= Pl::ContextMenu::EntryDelete;

		if(selections.size() == 1)
		{
			MetaData md = m->model->metadata(selections.first());
			m->contextMenu->setRating( md.rating() );
		}
	}

	if(idx.row() >= 0)
	{
		MetaData md = m->model->metadata(idx.row());
		m->contextMenu->setMetadata(md);

		if(md.id() >= 0)
		{
			entry_mask |= Pl::ContextMenu::EntryBookmarks;
			entry_mask |= Pl::ContextMenu::EntryFindInLibrary;
		}
	}

	if(m->model->currentTrack() >= 0){
		entry_mask |= Pl::ContextMenu::EntryCurrentTrack;
	}

	m->contextMenu->showActions(entry_mask);
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

QMimeData* View::dragableMimedata() const
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
		emit sigDoubleClicked(idx.row());
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

	int row = calcDragDropLine(event->pos());
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

		if(m->currentFileLabel){
			m->currentFileLabel->hide();
		}

		this->setDragDropMode(QAbstractItemView::DragDrop);
		this->setAcceptDrops(true);
		this->setFocus();
	}
}

void View::currentScannedFileChanged(const QString& current_file)
{
	if(!m->currentFileLabel)
	{
		m->currentFileLabel = new QLabel(this);
	}

	int offset_bottom = 3;
	offset_bottom += this->fontMetrics().height();
	if(m->progressbar) {
		offset_bottom += m->progressbar->height() + 2;
	}

	m->currentFileLabel->setText(current_file);
	m->currentFileLabel->setGeometry(0, this->height() - offset_bottom, this->width(), this->fontMetrics().height() + 4);
	m->currentFileLabel->show();
}

void View::dropEvent(QDropEvent* event)
{
	if(!this->acceptDrops()){
		event->ignore();
		return;
	}

	event->accept();
	handleDrop(event);
}


int View::mapModelIndexToIndex(const QModelIndex& idx) const
{
	return idx.row();
}

ModelIndexRange View::mapIndexToModelIndexes(int idx) const
{
	return ModelIndexRange
	(
		m->model->index(idx, 0),
		m->model->index(idx, m->model->columnCount() - 1)
	);
}

bool View::viewportEvent(QEvent* event)
{
	bool success = SearchableTableView::viewportEvent(event);

	if(event->type() == QEvent::Resize) {
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
	bool show_numbers = GetSetting(Set::PL_ShowNumbers);
	bool show_covers = GetSetting(Set::PL_ShowCovers);

	horizontalHeader()->setSectionHidden(Pl::Model::ColumnName::TrackNumber, !show_numbers);
	horizontalHeader()->setSectionHidden(Pl::Model::ColumnName::Cover, !show_covers);

	refresh();
}

void View::showRatingChanged()
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

	for(int i=0; i<rowCount(); i++)
	{
		if(h != rowHeight(i))
		{
			verticalHeader()->resizeSection(i, h);
		}
	}

	QHeaderView* hh = this->horizontalHeader();
	int viewport_width = viewport()->width();
	int w_time = Gui::Util::textWidget(fm, "1888:88");

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
		int w_tn = Gui::Util::textWidget(fm, QString::number(rowCount() * 100));
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

	m->model->setRowHeight(h);
}

void View::currentTrackChanged(int track_index, int playlist_index)
{
	if(m->playlist->index() == playlist_index){
		gotoRow(track_index);
	}
}
