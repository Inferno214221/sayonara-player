/* GUI_Playlist.cpp */

/* Copyright (C) 2011-2019 Lucio Carreras
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
#include "PlaylistActionMenu.h"
#include "Gui/Playlist/ui_GUI_Playlist.h"

#include "TabWidget.h"
#include "ListView.h"
#include "Gui/Utils/Icons.h"
#include "Gui/Utils/Library/GUI_DeleteDialog.h"

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

struct GUI_Playlist::Private {};

GUI_Playlist::GUI_Playlist(QWidget *parent) :
	Widget(parent)
{
	m = Pimpl::make<Private>();

	ui = new Ui::Playlist_Window();
	ui->setupUi(this);

	setAcceptDrops(true);

//	ui->btn_tool->setMenu(new PlaylistActionMenu(this));
	ui->btn_tool->setVisible(false);

	Handler* handler = Handler::instance();
	connect(handler, &Handler::sig_playlist_created, this, &GUI_Playlist::playlist_created);
	connect(handler, &Handler::sig_playlist_name_changed, this, &GUI_Playlist::playlist_name_changed);
	connect(handler, &Handler::sig_new_playlist_added, this, &GUI_Playlist::playlist_added);
	connect(handler, &Handler::sig_current_playlist_changed, this, &GUI_Playlist::playlist_idx_changed);

	PlayManager* play_manager = PlayManager::instance();
	connect(play_manager, &PlayManager::sig_playlist_finished,	this, &GUI_Playlist::playlist_finished);
	connect(play_manager, &PlayManager::sig_playstate_changed,	this, &GUI_Playlist::playstate_changed);

	connect(ui->tw_playlists, &PlaylistTabWidget::sig_add_tab_clicked, this, &GUI_Playlist::add_playlist_button_pressed);
	connect(ui->tw_playlists, &PlaylistTabWidget::tabCloseRequested, this, &GUI_Playlist::tab_close_playlist_clicked);
	connect(ui->tw_playlists, &PlaylistTabWidget::currentChanged, handler, &Playlist::Handler::set_current_index);
	connect(ui->tw_playlists, &PlaylistTabWidget::sig_tab_delete, this, &GUI_Playlist::tab_delete_playlist_clicked);
	connect(ui->tw_playlists, &PlaylistTabWidget::sig_tab_save, this, &GUI_Playlist::tab_save_playlist_clicked);
	connect(ui->tw_playlists, &PlaylistTabWidget::sig_tab_save_as, this, &GUI_Playlist::tab_save_playlist_as_clicked);
	connect(ui->tw_playlists, &PlaylistTabWidget::sig_tab_save_to_file, this, &GUI_Playlist::tab_save_playlist_to_file_clicked);
	connect(ui->tw_playlists, &PlaylistTabWidget::sig_tab_rename, this, &GUI_Playlist::tab_rename_clicked);
	connect(ui->tw_playlists, &PlaylistTabWidget::sig_tab_clear, this, &GUI_Playlist::clear_button_pressed);
	connect(ui->tw_playlists, &PlaylistTabWidget::sig_tab_reset, handler, &Playlist::Handler::reset_playlist);
	connect(ui->tw_playlists, &PlaylistTabWidget::sig_metadata_dropped, this, &GUI_Playlist::tab_metadata_dropped);
	connect(ui->tw_playlists, &PlaylistTabWidget::sig_open_file, this, &GUI_Playlist::open_file_clicked);
	connect(ui->tw_playlists, &PlaylistTabWidget::sig_open_dir, this, &GUI_Playlist::open_dir_clicked);

	connect(ui->btn_clear, &QPushButton::clicked, this, &GUI_Playlist::clear_button_pressed);

	ListenSetting(Set::PL_ShowClearButton, GUI_Playlist::sl_show_clear_button_changed);
}

GUI_Playlist::~GUI_Playlist()
{
	while(ui->tw_playlists->count() > 1)
	{
		int last_tab = ui->tw_playlists->count() - 1;

		QWidget* widget = ui->tw_playlists->widget(last_tab);
		ui->tw_playlists->removeTab(last_tab);

		if(widget){
			delete widget; widget = nullptr;
		}
	}

	if(ui){ delete ui; ui = nullptr; }
}

void GUI_Playlist::language_changed()
{
	ui->retranslateUi(this);
	set_total_time_label();
}

void GUI_Playlist::skin_changed()
{
	check_tab_icon();
}

void GUI_Playlist::clear_button_pressed(int pl_idx)
{
	Handler::instance()->clear_playlist(pl_idx);
}

void GUI_Playlist::bookmark_selected(int idx, Seconds timestamp)
{
	Playlist::Handler* plh = Playlist::Handler::instance();
	plh->change_track(idx, plh->current_index());
	PlayManager::instance()->seek_abs_ms(timestamp * 1000);
}


void GUI_Playlist::add_playlist_button_pressed()
{
	Handler::instance()->create_empty_playlist();
}

void GUI_Playlist::tab_metadata_dropped(int pl_idx, const MetaDataList& v_md)
{
	if(pl_idx < 0){
		return;
	}

	Handler* handler = Handler::instance();
	int origin_tab = ui->tw_playlists->get_drag_origin_tab();

	if(ui->tw_playlists->was_drag_from_playlist())
	{
		PlaylistView* plv = view_by_index(origin_tab);

		if(plv){
			plv->remove_selected_rows();
		}
	}

	if(origin_tab == pl_idx){
		handler->insert_tracks(v_md, 0, pl_idx);
	}

	else if(pl_idx == ui->tw_playlists->count() - 1)
	{
		QString name = Handler::instance()->request_new_playlist_name();
		handler->create_playlist(v_md, name);
	}

	else
	{
		handler->append_tracks(v_md, pl_idx);
	}
}

void GUI_Playlist::double_clicked(int row)
{
	int cur_idx = ui->tw_playlists->currentIndex();
	Handler::instance()->change_track(row, cur_idx);
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
	PlaylistView* cur_view = current_view();
	if(cur_view){
		cur_view->dropEventFromOutside(event);
	}
}

void GUI_Playlist::set_total_time_label()
{
	int n_rows = 0;
	int current_idx = ui->tw_playlists->currentIndex();
	PlaylistConstPtr pl = Handler::instance()->playlist(current_idx);

	MilliSeconds dur_ms = 0;
	if(pl){
		dur_ms = pl->running_time();
	}

	QString time_str;
	if(dur_ms > 0){
		time_str = Util::cvt_ms_to_string(dur_ms, true, false);
	}

	PlaylistView* cur_view = current_view();
	if(cur_view){
		n_rows = cur_view->row_count();
	}

	QString playlist_string = QString::number(n_rows);

	if(n_rows == 0){
		playlist_string = tr("Playlist empty");
	}

	else if(n_rows == 1)	{
		playlist_string += " " + Lang::get(Lang::Track);
	}

	else {
		playlist_string += " " + Lang::get(Lang::Tracks);
	}

	if( dur_ms > 0 ){
		playlist_string += " - " + time_str;
	}

	ui->lab_totalTime->setText(playlist_string);
	ui->lab_totalTime->setContentsMargins(0, 2, 0, 2);
}

void GUI_Playlist::open_file_clicked(int tgt_idx)
{
	Q_UNUSED(tgt_idx)

	QStringList filetypes;

	filetypes << Util::soundfile_extensions();
	filetypes << Util::playlist_extensions();

	QString filetypes_str = tr("Media files") + " (" + filetypes.join(" ") + ")";

	QStringList list =
			QFileDialog::getOpenFileNames(
					this,
					tr("Open Media files"),
					QDir::homePath(),
					filetypes_str);

	if(list.isEmpty()){
		return;
	}

	Handler::instance()->create_playlist(list);
}

void GUI_Playlist::open_dir_clicked(int tgt_idx)
{
	Q_UNUSED(tgt_idx)

	QString dir = QFileDialog::getExistingDirectory(this,
			Lang::get(Lang::OpenDir),
			QDir::homePath(),
			QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

	if (dir.isEmpty()){
		return;
	}

	Handler::instance()->create_playlist(dir);
}


void GUI_Playlist::delete_tracks_clicked(const IndexSet& rows)
{
	GUI_DeleteDialog dialog(rows.count(), this);
	dialog.exec();

	Library::TrackDeletionMode deletion_mode = dialog.answer();
	if(deletion_mode == Library::TrackDeletionMode::None){
		return;
	}

	int cur_idx = ui->tw_playlists->currentIndex();
	Handler::instance()->delete_tracks(cur_idx, rows, deletion_mode);
}

void GUI_Playlist::playlist_name_changed(int idx)
{
	PlaylistConstPtr pl = Handler::instance()->playlist(idx);
	if(!pl){
		return;
	}

	QString name = pl->get_name();
	check_playlist_name(pl);

	for(int i = ui->tw_playlists->count() - 2; i>=0; i--)
	{
		if(i == idx){
			continue;
		}

		if(ui->tw_playlists->tabText(i).compare(name) == 0){
			ui->tw_playlists->removeTab(i);
		}
	}
}


void GUI_Playlist::playlist_changed(int idx)
{
	PlaylistConstPtr pl = Handler::instance()->playlist(idx);
	check_playlist_name(pl);

	if(idx != ui->tw_playlists->currentIndex()){
		return;
	}

	set_total_time_label();
	check_playlist_menu(pl);
}

void GUI_Playlist::playlist_idx_changed(int pl_idx)
{
	if(!Util::between(pl_idx, ui->tw_playlists->count() - 1)){
		return;
	}

	PlaylistConstPtr pl = Handler::instance()->playlist(pl_idx);
	ui->tw_playlists->setCurrentIndex(pl_idx);

	set_total_time_label();
	check_playlist_menu(pl);
}


void GUI_Playlist::playlist_added(PlaylistPtr pl)
{
	int idx = pl->index();
	QString name = pl->get_name();

	PlaylistView* plv = new PlaylistView(pl, ui->tw_playlists);

	ui->tw_playlists->insertTab(ui->tw_playlists->count() - 1, plv, name);

	connect(plv, &PlaylistView::sig_double_clicked, this, &GUI_Playlist::double_clicked);
	connect(plv, &PlaylistView::sig_delete_tracks, this, &GUI_Playlist::delete_tracks_clicked);
	connect(plv, &PlaylistView::sig_bookmark_pressed, this, &GUI_Playlist::bookmark_selected);
	connect(pl.get(), &Playlist::Playlist::sig_items_changed, this, &GUI_Playlist::playlist_changed);

	Handler::instance()->set_current_index(idx);
}

void GUI_Playlist::playlist_created(PlaylistPtr pl)
{
	set_total_time_label();
	check_playlist_name(pl);
}


void GUI_Playlist::playstate_changed(PlayState state)
{
	Q_UNUSED(state)
	check_tab_icon();
}

void GUI_Playlist::playlist_finished()
{
	check_tab_icon();
}


void GUI_Playlist::tab_close_playlist_clicked(int idx)
{
	int count = ui->tw_playlists->count();
	if( !Util::between(idx, count - 1)) {
		return;
	}

	QWidget* playlist_widget = ui->tw_playlists->widget(idx);
	ui->tw_playlists->removeTab(idx);

	PlaylistView* plv = current_view();
	if(plv){
		plv->setFocus();
	}

	delete playlist_widget; playlist_widget = nullptr;

	Handler::instance()->close_playlist(idx);
}


void GUI_Playlist::tab_save_playlist_clicked(int idx)
{
	Util::SaveAsAnswer success =
			Handler::instance()->save_playlist(idx);

	if(success == Util::SaveAsAnswer::Success)
	{
		QString old_string = ui->tw_playlists->tabText(idx);

		if(old_string.startsWith("* ")){
			old_string.remove(0, 2);
		}

		ui->tw_playlists->setTabText(idx, old_string);
	}

	show_save_message_box(success);
}


void GUI_Playlist::tab_save_playlist_as_clicked(int idx, const QString& str)
{
	if(str.isEmpty()){
		return;
	}

	Handler* handler = Handler::instance();

	Util::SaveAsAnswer success =
			handler->save_playlist_as(idx, str, false);

	if(success == Util::SaveAsAnswer::NameAlreadyThere)
	{
		Message::Answer answer = show_save_message_box(success);

		if(answer == Message::Answer::No) {
			return;
		}

		success = handler->save_playlist_as(idx, str, true);
	}

	show_save_message_box(success);
}


void GUI_Playlist::tab_save_playlist_to_file_clicked(int pl_idx, const QString &filename)
{
	Handler::instance()->save_playlist_to_file(pl_idx, filename, false);
}


void GUI_Playlist::tab_rename_clicked(int idx, const QString& str)
{
	Util::SaveAsAnswer success = Handler::instance()->rename_playlist(idx, str);

	if(success == Util::SaveAsAnswer::NameAlreadyThere) {
		Message::error(tr("Playlist name already exists"));
	}

	else {
		show_save_message_box(success);
	}
}


void GUI_Playlist::tab_delete_playlist_clicked(int idx)
{
	Message::Answer answer = Message::question_yn(Lang::get(Lang::Really).question(), Lang::get(Lang::Delete));

	if(answer == Message::Answer::No) {
		return;
	}

	Handler::instance()->delete_playlist(idx);
}


void GUI_Playlist::check_tab_icon()
{
	for(int i=0; i<ui->tw_playlists->count(); i++)
	{
		ui->tw_playlists->setIconSize(QSize(16, 16));
		ui->tw_playlists->setTabIcon(i, QIcon());
	}

	int active_idx = Handler::instance()->active_index();
	PlaylistView* plv = view_by_index(active_idx);
	if(!plv){
		return;
	}

	PlayState state = PlayManager::instance()->playstate();
	if(state == PlayState::Stopped){
		return;
	}

	if(plv->row_count() == 0){
		return;
	}

	QIcon icon = Gui::Icons::icon(Gui::Icons::PlayBorder);

	ui->tw_playlists->tabBar()->setTabIcon(active_idx, icon);
}


void GUI_Playlist::check_playlist_menu(PlaylistConstPtr pl)
{
	PlaylistMenuEntries entries = PlaylistMenuEntry::None;

	bool temporary =		pl->is_temporary();
	bool was_changed =		pl->was_changed();
	bool storable =			pl->is_storable();
	bool is_empty =			(pl->count() == 0);

	bool save_enabled =		(!temporary && storable);
	bool save_as_enabled =	(storable);
	bool save_to_file_enabled = (!is_empty);
	bool delete_enabled =	(!temporary && storable);
	bool reset_enabled =	(!temporary && storable && was_changed);
	bool close_enabled =	(ui->tw_playlists->count() > 2);
	bool rename_enabled =	(storable);
	bool clear_enabled =	(!is_empty);

	entries |= PlaylistMenuEntry::OpenFile;
	entries |= PlaylistMenuEntry::OpenDir;

	if(save_enabled){
		entries |= PlaylistMenuEntry::Save;
	}
	if(save_as_enabled){
		entries |= PlaylistMenuEntry::SaveAs;
	}
	if(save_to_file_enabled){
		entries |= PlaylistMenuEntry::SaveToFile;
	}
	if(delete_enabled){
		entries |= PlaylistMenuEntry::Delete;
	}
	if(reset_enabled){
		entries |= PlaylistMenuEntry::Reset;
	}
	if(close_enabled){
		entries |= PlaylistMenuEntry::Close;
		entries |= PlaylistMenuEntry::CloseOthers;
	}
	if(rename_enabled){
		entries |= PlaylistMenuEntry::Rename;
	}
	if(clear_enabled){
		entries |= PlaylistMenuEntry::Clear;
	}

	ui->tw_playlists->show_menu_items(entries);
}


void GUI_Playlist::check_playlist_name(PlaylistConstPtr pl)
{
	QString name = pl->get_name();

	if(!pl->is_temporary() &&
		pl->was_changed() &&
		pl->is_storable())
	{
		name.prepend("* ");
	}

	ui->tw_playlists->setTabText(pl->index(), name);
}


Message::Answer GUI_Playlist::show_save_message_box(Util::SaveAsAnswer answer)
{
	switch(answer)
	{
		case Util::SaveAsAnswer::OtherError:
			Message::warning(tr("Cannot save playlist."), Lang::get(Lang::SaveAs));
			break;

		case Util::SaveAsAnswer::NameAlreadyThere:
			return Message::question_yn(tr("Playlist exists") + "\n" + Lang::get(Lang::Overwrite).question(),
										Lang::get(Lang::SaveAs));

		case Util::SaveAsAnswer::NotStorable:
			return Message::warning(tr("Playlists are currently only supported for library tracks."), tr("Save playlist"));

		default:
			return Message::Answer::Undefined;
	}

	return Message::Answer::Undefined;
}


PlaylistView* GUI_Playlist::view_by_index(int idx)
{
	if(!Util::between(idx, ui->tw_playlists->count() - 1)){
		return nullptr;
	}

	return static_cast<PlaylistView*>(ui->tw_playlists->widget(idx));
}


PlaylistView* GUI_Playlist::current_view()
{
	int idx = ui->tw_playlists->currentIndex();
	if(!Util::between(idx, ui->tw_playlists->count() - 1)){
		return nullptr;
	}

	return static_cast<PlaylistView*>(ui->tw_playlists->widget(idx));
}

void GUI_Playlist::sl_show_clear_button_changed()
{
	ui->btn_clear->setVisible(GetSetting(Set::PL_ShowClearButton));
}

