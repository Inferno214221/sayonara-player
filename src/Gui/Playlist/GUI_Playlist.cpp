/* GUI_Playlist.cpp */

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
 *  Created on: Apr 6, 2011
 */

#include "GUI_Playlist.h"
#include "PlaylistTabWidget.h"
#include "PlaylistView.h"
#include "PlaylistActionMenu.h"

#include "Gui/Playlist/ui_GUI_Playlist.h"
#include "Gui/Library/Utils/GUI_DeleteDialog.h"
#include "Gui/Utils/Icons.h"
#include "Gui/Utils/PreferenceAction.h"

#include "Utils/Utils.h"
#include "Utils/Set.h"
#include "Utils/Settings/Settings.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Language/Language.h"
#include "Utils/globals.h"
#include "Utils/Message/Message.h"

#include "Components/PlayManager/PlayManager.h"
#include "Components/Playlist/Playlist.h"
#include "Components/Playlist/PlaylistHandler.h"

#include <QFileDialog>
#include <QTimer>
#include <QTabBar>
#include <QShortcut>

using Playlist::Handler;
using Playlist::View;
using Playlist::TabWidget;


static Message::Answer showSaveMessageBox(QWidget* parent, Util::SaveAsAnswer answer)
{
	switch(answer)
	{
		case Util::SaveAsAnswer::OtherError:
			Message::warning(parent->tr("Cannot save playlist."), Lang::get(Lang::SaveAs));
			break;

		case Util::SaveAsAnswer::NameAlreadyThere:
			return Message::question_yn(parent->tr("Playlist exists") + "\n" + Lang::get(Lang::Overwrite).question(),
										Lang::get(Lang::SaveAs));

		case Util::SaveAsAnswer::NotStorable:
			return Message::warning(parent->tr("Playlists are currently only supported for library tracks."), parent->tr("Save playlist"));

		default:
			return Message::Answer::Undefined;
	}

	return Message::Answer::Undefined;
}


struct GUI_Playlist::Private {};

GUI_Playlist::GUI_Playlist(QWidget* parent) :
	Widget(parent)
{
	m = Pimpl::make<Private>();

	ui = new Ui::PlaylistWindow();
	ui->setupUi(this);

	setAcceptDrops(true);

	auto* handler = Handler::instance();
	connect(handler, &Handler::sigPlaylistCreated, this, &GUI_Playlist::playlistCreated);
	connect(handler, &Handler::sigPlaylistNameChanged, this, &GUI_Playlist::playlistNameChanged);
	connect(handler, &Handler::sigNewPlaylistAdded, this, &GUI_Playlist::playlistAdded);
	connect(handler, &Handler::sigCurrentPlaylistChanged, this, &GUI_Playlist::playlistIdxChanged);

	auto* playManager = PlayManager::instance();
	connect(playManager, &PlayManager::sigPlaylistFinished,	this, &GUI_Playlist::playlistFinished);
	connect(playManager, &PlayManager::sigPlaystateChanged,	this, &GUI_Playlist::playstateChanged);

	connect(ui->twPlaylists, &TabWidget::sigAddTabClicked, this, &GUI_Playlist::addPlaylistButtonPressed);
	connect(ui->twPlaylists, &TabWidget::tabCloseRequested, this, &GUI_Playlist::tabClosePlaylistClicked);
	connect(ui->twPlaylists, &TabWidget::currentChanged, handler, &Playlist::Handler::set_current_index);
	connect(ui->twPlaylists, &TabWidget::sigTabDelete, this, &GUI_Playlist::tabDeletePlaylistClicked);
	connect(ui->twPlaylists, &TabWidget::sigTabSave, this, &GUI_Playlist::tabSavePlaylistClicked);
	connect(ui->twPlaylists, &TabWidget::sigTabSaveAs, this, &GUI_Playlist::tabSavePlaylistAsClicked);
	connect(ui->twPlaylists, &TabWidget::sigTabSaveToFile, this, &GUI_Playlist::tabSavePlaylistToFileClicked);
	connect(ui->twPlaylists, &TabWidget::sigTabRename, this, &GUI_Playlist::tabRenameClicked);
	connect(ui->twPlaylists, &TabWidget::sigTabClear, this, &GUI_Playlist::clearButtonPressed);
	connect(ui->twPlaylists, &TabWidget::sigTabReset, handler, &Playlist::Handler::resetPlaylist);
	connect(ui->twPlaylists, &TabWidget::sigMetadataDropped, this, &GUI_Playlist::tabMetadataDropped);
	connect(ui->twPlaylists, &TabWidget::sigFilesDropped, this, &GUI_Playlist::tabFilesDropped);
	connect(ui->twPlaylists, &TabWidget::sigOpenFile, this, &GUI_Playlist::openFileClicked);
	connect(ui->twPlaylists, &TabWidget::sigOpenDir, this, &GUI_Playlist::openDirClicked);

	connect(ui->btnClear, &QPushButton::clicked, this, &GUI_Playlist::clearButtonPressed);

	initToolButton();

	ListenSetting(Set::PL_ShowBottomBar, GUI_Playlist::showBottomBarChanged);
	ListenSetting(Set::PL_ShowClearButton, GUI_Playlist::showClearButtonChanged);
}

GUI_Playlist::~GUI_Playlist()
{
	while(ui->twPlaylists->count() > 1)
	{
		int lastTab = ui->twPlaylists->count() - 1;

		QWidget* widget = ui->twPlaylists->widget(lastTab);
		ui->twPlaylists->removeTab(lastTab);

		if(widget){
			delete widget; widget = nullptr;
		}
	}

	if(ui){ delete ui; ui = nullptr; }
}

void GUI_Playlist::initToolButton()
{
	QMenu* menu = new Playlist::ActionMenu(this);
	QList<QAction*> actions = menu->actions();
	for(auto* action : actions)
	{
		ui->toolButton->registerAction(action);
	}

	ui->toolButton->registerPreferenceAction(new Gui::PlaylistPreferenceAction(this));
}

void GUI_Playlist::clearButtonPressed(int playlistIndex)
{
	Handler::instance()->clearPlaylist(playlistIndex);
}

void GUI_Playlist::bookmarkSelected(int idx, Seconds timestamp)
{
	Playlist::Handler* plh = Playlist::Handler::instance();
	plh->changeTrack(idx, plh->current_index());
	PlayManager::instance()->seekAbsoluteMs(timestamp * 1000);
}

void GUI_Playlist::addPlaylistButtonPressed()
{
	Handler::instance()->createEmptyPlaylist();
}

void GUI_Playlist::tabMetadataDropped(int playlistIndex, const MetaDataList& tracks)
{
	if(playlistIndex < 0){
		return;
	}

	Handler* handler = Handler::instance();
	int originTab = ui->twPlaylists->getDragOriginTab();

	if(ui->twPlaylists->wasDragFromPlaylist())
	{
		View* plv = viewByIndex(originTab);

		if(plv){
			plv->removeSelectedRows();
		}
	}

	if(originTab == playlistIndex){
		handler->insertTracks(tracks, 0, playlistIndex);
	}

	else if(playlistIndex == ui->twPlaylists->count() - 1)
	{
		QString name = handler->requestNewPlaylistName();
		handler->createPlaylist(tracks, name);
	}

	else
	{
		handler->appendTracks(tracks, playlistIndex);
	}
}

void GUI_Playlist::tabFilesDropped(int playlistIndex, const QStringList& paths)
{
	Handler* handler = Handler::instance();
	if(playlistIndex < 0 || playlistIndex >= ui->twPlaylists->count()){
		return;
	}

	int originTab = ui->twPlaylists->getDragOriginTab();
	bool wasDragFromPlaylist = ui->twPlaylists->wasDragFromPlaylist();
	if(originTab >= 0 || wasDragFromPlaylist) {
		return;
	}

	if(playlistIndex == ui->twPlaylists->count() - 1) {
		QString name = handler->requestNewPlaylistName();
		handler->createPlaylist(paths, name);
	}

	else {
		handler->appendTracks(paths, playlistIndex);
	}
}

void GUI_Playlist::doubleClicked(int row)
{
	int currentIndex = ui->twPlaylists->currentIndex();
	Handler::instance()->changeTrack(row, currentIndex);
}

void GUI_Playlist::setTotalTimeLabel()
{
	int currentIndex = ui->twPlaylists->currentIndex();
	PlaylistConstPtr pl = Handler::instance()->playlist(currentIndex);

	MilliSeconds durationMs = 0;
	if(pl) {
		durationMs = pl->runningTime();
	}

	int rows = 0;
	View* currentView = this->currentView();
	if(currentView) {
		rows = currentView->model()->rowCount();
	}

	QString playlistString = Lang::getWithNumber(Lang::NrTracks, rows);
	if(rows == 0) {
		playlistString = tr("Playlist empty");
	}

	if(durationMs > 0){
		playlistString += " - " + Util::msToString(durationMs, "$He $M:$S");
	}

	ui->labTotalTime->setText(playlistString);
	ui->labTotalTime->setContentsMargins(0, 2, 0, 2);
}

void GUI_Playlist::openFileClicked(int targetIndex)
{
	Q_UNUSED(targetIndex)

	const QString filter = Util::getFileFilter(
		Util::Extensions(Util::Extension::Soundfile | Util::Extension::Playlist),
		tr("Media files")
	);

	const QStringList list = QFileDialog::getOpenFileNames
	(
		this,
		tr("Open Media files"),
		QDir::homePath(),
		filter
	);

	if(!list.isEmpty()){
		Handler::instance()->createPlaylist(list);
	}
}

void GUI_Playlist::openDirClicked(int targetIndex)
{
	Q_UNUSED(targetIndex)

	const QString dir = QFileDialog::getExistingDirectory
	(
		this,
		Lang::get(Lang::OpenDir),
		QDir::homePath(),
		QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
	);

	if(!dir.isEmpty()){
		Handler::instance()->createPlaylist(dir);
	}
}

void GUI_Playlist::deleteTracksClicked(const IndexSet& rows)
{
	GUI_DeleteDialog dialog(rows.count(), this);
	dialog.exec();

	Library::TrackDeletionMode deletion_mode = dialog.answer();
	if(deletion_mode == Library::TrackDeletionMode::None){
		return;
	}

	int currentIndex = ui->twPlaylists->currentIndex();
	Handler::instance()->deleteTracks(currentIndex, rows, deletion_mode);
}

void GUI_Playlist::playlistNameChanged(int idx)
{
	PlaylistConstPtr pl = Handler::instance()->playlist(idx);
	if(!pl){
		return;
	}

	QString name = pl->name();
	checkPlaylistName(pl);

	for(int i = ui->twPlaylists->count() - 2; i>=0; i--)
	{
		if(i == idx){
			continue;
		}

		if(ui->twPlaylists->tabText(i).compare(name) == 0){
			ui->twPlaylists->removeTab(i);
		}
	}
}


void GUI_Playlist::playlistChanged(int idx)
{
	PlaylistConstPtr pl = Handler::instance()->playlist(idx);
	checkPlaylistName(pl);

	if(idx != ui->twPlaylists->currentIndex()){
		return;
	}

	setTotalTimeLabel();
	checkPlaylistMenu(pl);
}

void GUI_Playlist::playlistIdxChanged(int playlistIndex)
{
	if(!Util::between(playlistIndex, ui->twPlaylists->count() - 1)){
		return;
	}

	PlaylistConstPtr pl = Handler::instance()->playlist(playlistIndex);
	ui->twPlaylists->setCurrentIndex(playlistIndex);

	this->setFocusProxy(ui->twPlaylists->currentWidget());

	setTotalTimeLabel();
	checkPlaylistMenu(pl);
}


void GUI_Playlist::playlistAdded(PlaylistPtr pl)
{
	int idx = pl->index();
	QString name = pl->name();

	View* plv = new View(pl, ui->twPlaylists);

	ui->twPlaylists->insertTab(ui->twPlaylists->count() - 1, plv, name);

	connect(plv, &View::sigDoubleClicked, this, &GUI_Playlist::doubleClicked);
	connect(plv, &View::sigDeleteTracks, this, &GUI_Playlist::deleteTracksClicked);
	connect(plv, &View::sigBookmarkPressed, this, &GUI_Playlist::bookmarkSelected);
	connect(pl.get(), &Playlist::Playlist::sigItemsChanged, this, &GUI_Playlist::playlistChanged);

	Handler::instance()->set_current_index(idx);
}

void GUI_Playlist::playlistCreated(PlaylistPtr pl)
{
	setTotalTimeLabel();
	checkPlaylistName(pl);
}


void GUI_Playlist::playstateChanged(PlayState state)
{
	Q_UNUSED(state)
	checkTabIcon();
}

void GUI_Playlist::playlistFinished()
{
	checkTabIcon();
}

void GUI_Playlist::tabClosePlaylistClicked(int idx)
{
	int count = ui->twPlaylists->count();
	if( !Util::between(idx, count - 1)) {
		return;
	}

	QWidget* playlist_widget = ui->twPlaylists->widget(idx);
	ui->twPlaylists->removeTab(idx);

	View* plv = currentView();
	if(plv){
		plv->setFocus();
	}

	delete playlist_widget; playlist_widget = nullptr;

	Handler::instance()->closePlaylist(idx);
}


void GUI_Playlist::tabSavePlaylistClicked(int idx)
{
	Util::SaveAsAnswer success =
			Handler::instance()->savePlaylist(idx);

	if(success == Util::SaveAsAnswer::Success)
	{
		QString old_string = ui->twPlaylists->tabText(idx);

		if(old_string.startsWith("* ")){
			old_string.remove(0, 2);
		}

		ui->twPlaylists->setTabText(idx, old_string);
	}

	showSaveMessageBox(this, success);
}


void GUI_Playlist::tabSavePlaylistAsClicked(int idx, const QString& str)
{
	if(str.isEmpty()){
		return;
	}

	Handler* handler = Handler::instance();

	Util::SaveAsAnswer success =
			handler->savePlaylistAs(idx, str, false);

	if(success == Util::SaveAsAnswer::NameAlreadyThere)
	{
		Message::Answer answer = showSaveMessageBox(this, success);

		if(answer == Message::Answer::No) {
			return;
		}

		success = handler->savePlaylistAs(idx, str, true);
	}

	showSaveMessageBox(this, success);
}


void GUI_Playlist::tabSavePlaylistToFileClicked(int playlistIndex, const QString& filename)
{
	Handler::instance()->savePlaylistToFile(playlistIndex, filename, false);
}


void GUI_Playlist::tabRenameClicked(int idx, const QString& str)
{
	Util::SaveAsAnswer success = Handler::instance()->renamePlaylist(idx, str);

	if(success == Util::SaveAsAnswer::NameAlreadyThere) {
		Message::error(tr("Playlist name already exists"));
	}

	else {
		showSaveMessageBox(this, success);
	}
}


void GUI_Playlist::tabDeletePlaylistClicked(int idx)
{
	Message::Answer answer = Message::question_yn(Lang::get(Lang::Really).question(), Lang::get(Lang::Delete));

	if(answer == Message::Answer::No) {
		return;
	}

	Handler::instance()->deletePlaylist(idx);
}


void GUI_Playlist::checkTabIcon()
{
	for(int i=0; i<ui->twPlaylists->count(); i++)
	{
		ui->twPlaylists->setIconSize(QSize(16, 16));
		ui->twPlaylists->setTabIcon(i, QIcon());
	}

	int activeIndex = Handler::instance()->activeIndex();
	View* plv = viewByIndex(activeIndex);
	if(!plv){
		return;
	}

	PlayState state = PlayManager::instance()->playstate();
	if(state == PlayState::Stopped){
		return;
	}

	if(plv->model()->rowCount() == 0){
		return;
	}

	QIcon icon = Gui::Icons::icon(Gui::Icons::PlayBorder);

	ui->twPlaylists->tabBar()->setTabIcon(activeIndex, icon);
}


void GUI_Playlist::checkPlaylistMenu(PlaylistConstPtr pl)
{
	using Playlist::MenuEntry;

	Playlist::MenuEntries entries = MenuEntry::None;

	bool temporary =		pl->isTemporary();
	bool wasChanged =		pl->wasChanged();
	bool isEmpty =			(pl->count() == 0);

	bool saveEnabled =		(!temporary);
	bool saveAsEnabled =	true;
	bool saveToFileEnabled = (!isEmpty);
	bool deleteEnabled =	(!temporary);
	bool resetEnabled =		(!temporary && wasChanged);
	bool closeEnabled =		(ui->twPlaylists->count() > 2);
	bool renameEnabled =	true;
	bool clearEnabled =		(!isEmpty);

	entries |= MenuEntry::OpenFile;
	entries |= MenuEntry::OpenDir;

	if(saveEnabled){
		entries |= MenuEntry::Save;
	}
	if(saveAsEnabled){
		entries |= MenuEntry::SaveAs;
	}
	if(saveToFileEnabled){
		entries |= MenuEntry::SaveToFile;
	}
	if(deleteEnabled){
		entries |= MenuEntry::Delete;
	}
	if(resetEnabled){
		entries |= MenuEntry::Reset;
	}
	if(closeEnabled){
		entries |= MenuEntry::Close;
		entries |= MenuEntry::CloseOthers;
	}
	if(renameEnabled){
		entries |= MenuEntry::Rename;
	}
	if(clearEnabled){
		entries |= MenuEntry::Clear;
	}

	ui->twPlaylists->showMenuItems(entries);
}

void GUI_Playlist::checkPlaylistName(PlaylistConstPtr pl)
{
	QString name = pl->name();

	if(!pl->isTemporary() &&
		pl->wasChanged())
	{
		name.prepend("* ");
	}

	ui->twPlaylists->setTabText(pl->index(), name);
}

View* GUI_Playlist::viewByIndex(int idx)
{
	if(!Util::between(idx, ui->twPlaylists->count() - 1)){
		return nullptr;
	}

	return static_cast<View*>(ui->twPlaylists->widget(idx));
}

View* GUI_Playlist::currentView()
{
	int idx = ui->twPlaylists->currentIndex();
	if(!Util::between(idx, ui->twPlaylists->count() - 1)){
		return nullptr;
	}

	return static_cast<View*>(ui->twPlaylists->widget(idx));
}

void GUI_Playlist::showClearButtonChanged()
{
	ui->btnClear->setVisible(GetSetting(Set::PL_ShowClearButton));
}

void GUI_Playlist::showBottomBarChanged()
{
	bool showBottomBar = GetSetting(Set::PL_ShowBottomBar);

	ui->toolButton->setVisible(!showBottomBar);
	ui->bottomBar->setVisible(showBottomBar);
}

void GUI_Playlist::languageChanged()
{
	ui->retranslateUi(this);
	setTotalTimeLabel();
}

void GUI_Playlist::skinChanged()
{
	checkTabIcon();
	ui->btnClear->setIcon(Gui::Icons::icon(Gui::Icons::Clear));
}

void GUI_Playlist::dragLeaveEvent(QDragLeaveEvent* event)
{
	event->accept();
}

void GUI_Playlist::dragEnterEvent(QDragEnterEvent* event)
{
	event->accept();
}

void GUI_Playlist::dragMoveEvent(QDragMoveEvent* event)
{
	event->accept();
}

void GUI_Playlist::dropEvent(QDropEvent* event)
{
	View* currentView = this->currentView();
	if(currentView){
		currentView->dropEventFromOutside(event);
	}
}
