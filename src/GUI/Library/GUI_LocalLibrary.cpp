/* GUI_LocalLibrary.cpp */

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
 * GUI_LocalLibrary.cpp
 *
 *  Created on: Apr 24, 2011
 *      Author: Lucio Carreras
 */

#include "GUI_LocalLibrary.h"
#include "GUI/Library/ui_GUI_LocalLibrary.h"
#include "GUI/Library/Helper/LocalLibraryMenu.h"
#include "GUI/Library/Models/DateSearchModel.h"
#include "GUI/Library/Views/AlbumCoverView.h"
#include "GUI/Library/Models/AlbumCoverModel.h"

#include "GUI/Helper/Library/LibraryDeleteDialog.h"
#include "GUI/Helper/SearchableWidget/SearchableView.h"

#include "Components/Library/LocalLibrary.h"
#include "Components/Library/LibraryManager.h"
#include "InfoBox/GUI_LibraryInfoBox.h"
#include "ImportFolderDialog/GUI_ImportFolder.h"

#include "Helper/Helper.h"
#include "Helper/Library/DateFilter.h"
#include "Helper/Settings/Settings.h"
#include "Helper/Language.h"
#include "Helper/Message/Message.h"
#include "Components/Covers/CoverLocation.h"

#include <QFileDialog>
#include <QDir>
#include <QTimer>
#include <QShortcut>
#include <QInputDialog>
#include <QMessageBox>
#include <QTreeView>
#include <QStandardPaths>

struct GUI_LocalLibrary::Private
{
	LocalLibrary*			library=nullptr;
	GUI_LibraryInfoBox*		library_info_box=nullptr;
	GUI_ImportFolder*		ui_importer=nullptr;
	LocalLibraryMenu*		library_menu=nullptr;
	AlbumCoverView*			acv = nullptr;
	AlbumCoverModel*		acm = nullptr;
};


GUI_LocalLibrary::GUI_LocalLibrary(int id, QWidget* parent) :
	GUI_AbstractLibrary(LibraryManager::getInstance()->library_instance(id), parent)
{
	setup_parent(this, &ui);

	m = Pimpl::make<Private>();

	m->library = LibraryManager::getInstance()->library_instance(id);
	m->library_menu = new LocalLibraryMenu(
						   m->library->library_name(),
						   m->library->library_path(),
						   this);

	ui->pb_progress->setVisible(false);
	ui->lab_progress->setVisible(false);
	ui->btn_reload_library->setVisible(false);

	connect(m->library, &LocalLibrary::sig_reloading_library, this, &GUI_LocalLibrary::progress_changed);
	connect(m->library, &LocalLibrary::sig_reloading_library_finished, this, &GUI_LocalLibrary::reload_finished);
	connect(m->library, &LocalLibrary::sig_reloading_library_finished, ui->lv_genres, &LibraryGenreView::reload_genres);
	connect(m->library, &LocalLibrary::sig_path_changed, this, &GUI_LocalLibrary::path_changed);
	connect(m->library, &LocalLibrary::sig_name_changed, this, &GUI_LocalLibrary::name_changed);

	connect(ui->lv_album, &LibraryViewAlbum::sig_disc_pressed, this, &GUI_LocalLibrary::disc_pressed);
	connect(ui->lv_album, &LibraryViewAlbum::sig_import_files, this, &GUI_LocalLibrary::import_files);
	connect(ui->lv_album, &LibraryView::sig_merge, m->library, &LocalLibrary::merge_albums);

	connect(ui->lv_artist, &LibraryView::sig_import_files, this, &GUI_LocalLibrary::import_files);
	connect(ui->lv_artist, &LibraryView::sig_merge, m->library, &LocalLibrary::merge_artists);
	connect(ui->tb_title, &LibraryView::sig_import_files, this, &GUI_LocalLibrary::import_files);
	connect(ui->lv_genres, &QAbstractItemView::clicked, this, &GUI_LocalLibrary::genre_selection_changed);
	connect(ui->lv_genres, &QAbstractItemView::activated, this, &GUI_LocalLibrary::genre_selection_changed);
	connect(ui->lv_genres, &LibraryGenreView::sig_progress, this, &GUI_LocalLibrary::progress_changed);
	connect(ui->lv_genres, &LibraryGenreView::sig_genres_reloaded, this, &GUI_LocalLibrary::genres_reloaded);

	connect(ui->lv_date_search, &QAbstractItemView::clicked, this, &GUI_LocalLibrary::date_selection_changed);
	connect(ui->lv_date_search, &QAbstractItemView::activated, this, &GUI_LocalLibrary::date_selection_changed);

	connect(m->library_menu, &LocalLibraryMenu::sig_path_changed, this, &GUI_LocalLibrary::change_library_path);
	connect(m->library_menu, &LocalLibraryMenu::sig_name_changed, this, &GUI_LocalLibrary::change_library_name);

	connect(m->library_menu, &LocalLibraryMenu::sig_import_file, this, &GUI_LocalLibrary::import_files_requested);
	connect(m->library_menu, &LocalLibraryMenu::sig_import_folder, this, &GUI_LocalLibrary::import_dirs_requested);
	connect(m->library_menu, &LocalLibraryMenu::sig_info, this, &GUI_LocalLibrary::show_info_box);
	connect(m->library_menu, &LocalLibraryMenu::sig_show_album_artists_changed, this, &GUI_LocalLibrary::refresh);
	connect(m->library_menu, &LocalLibraryMenu::sig_reload_library, this, [=](){
		this->reload_library_requested();
	});

	connect(ui->btn_reload_library, &QPushButton::clicked, this, [=](){
		this->reload_library_requested();
	});

	connect(ui->splitter_artist_album, &QSplitter::splitterMoved, this, &GUI_LocalLibrary::splitter_artist_moved);
	connect(ui->splitter_tracks, &QSplitter::splitterMoved, this, &GUI_LocalLibrary::splitter_tracks_moved);
	connect(ui->splitter_genre, &QSplitter::splitterMoved, this, &GUI_LocalLibrary::splitter_genre_moved);
	connect(ui->splitter_date, &QSplitter::splitterMoved, this, &GUI_LocalLibrary::splitter_date_moved);

	connect(m->library, &LocalLibrary::sig_import_dialog_requested, this, &GUI_LocalLibrary::import_dialog_requested);

	setAcceptDrops(true);

	QTimer::singleShot(100, m->library, SLOT(load()));

	ui->lv_genres->set_local_library(m->library);
	language_changed();

	REGISTER_LISTENER_NO_CALL(Set::Lib_ShowAlbumCovers, switch_album_view);
}


GUI_LocalLibrary::~GUI_LocalLibrary()
{
	if(ui)
	{
		delete ui; ui = nullptr;
	}
}

QMenu* GUI_LocalLibrary::menu() const
{
	return m->library_menu;
}

QFrame* GUI_LocalLibrary::header_frame() const
{
	return ui->header_frame;
}


void GUI_LocalLibrary::showEvent(QShowEvent* e)
{
	GUI_AbstractLibrary::showEvent(e);

	QByteArray artist_splitter_state, track_splitter_state, genre_splitter_state, date_splitter_state;

	artist_splitter_state = _settings->get(Set::Lib_SplitterStateArtist);
	track_splitter_state = _settings->get(Set::Lib_SplitterStateTrack);
	genre_splitter_state = _settings->get(Set::Lib_SplitterStateGenre);
	date_splitter_state = _settings->get(Set::Lib_SplitterStateDate);

	if(!artist_splitter_state.isEmpty()){
		ui->splitter_artist_album->restoreState(artist_splitter_state);
	}

	if(!track_splitter_state.isEmpty()){
		ui->splitter_tracks->restoreState(track_splitter_state);
	}

	if(!genre_splitter_state.isEmpty()){
		ui->splitter_genre->restoreState(genre_splitter_state);
	}

	if(!date_splitter_state.isEmpty()){
		ui->splitter_date->restoreState(date_splitter_state);
	}
}

void GUI_LocalLibrary::init_shortcuts()
{
	ui->le_search->setShortcutEnabled(QKeySequence::Find, true);

	new QShortcut(QKeySequence(Qt::Key_Escape), this, SLOT(clear_button_pressed()), nullptr, Qt::WidgetWithChildrenShortcut);
	new QShortcut(QKeySequence::Find, ui->le_search, SLOT(setFocus()), nullptr, Qt::WindowShortcut);
}


Library::ReloadQuality GUI_LocalLibrary::show_quality_dialog()
{
	QStringList lst;
	bool ok = false;

	lst << tr("Check for changed files (fast)") + "\t";
	lst << tr("Deep scan (slow)") + "\t";

	QString str = QInputDialog::getItem(this,
										"Sayonara",
										tr("Select reload mode") + "\n",
										lst,
										0,
										false,
										&ok);

	if(!ok){
		return Library::ReloadQuality::Unknown;
	}

	if(str.isEmpty()){
		return Library::ReloadQuality::Unknown;
	}

	if(str.compare(lst.first()) == 0){
		return Library::ReloadQuality::Fast;
	}

	if(str.compare(lst[1]) == 0){
		return Library::ReloadQuality::Accurate;
	}

	return Library::ReloadQuality::Unknown;
}

void GUI_LocalLibrary::switch_album_view()
{
	bool show_cover_view = _settings->get(Set::Lib_ShowAlbumCovers);
	int idx = 0;
	if(show_cover_view){
		idx = 1;
		if(!m->acv){
			init_album_cover_view();
		}
	}

	ui->sw_album_covers->setCurrentIndex( idx );
    	search_cleared();
}


void GUI_LocalLibrary::language_changed()
{
	ui->retranslateUi(this);
	ui->gb_genres->setTitle(Lang::get(Lang::Genres));

	GUI_AbstractLibrary::language_changed();
}

void GUI_LocalLibrary::search_cleared()
{
	ui->lv_genres->clearSelection();
	ui->lv_date_search->clearSelection();

    GUI_AbstractLibrary::search_cleared();
}

void GUI_LocalLibrary::genre_selection_changed(const QModelIndex& index)
{
	QVariant data = index.data();
    combo_search_changed(::Library::Filter::Genre);

    ui->le_search->setText(data.toString());
    search_edited(data.toString());
}

void GUI_LocalLibrary::date_selection_changed(const QModelIndex& index)
{
	Library::Filter filter;
	Library::DateFilter date_filter = ui->lv_date_search->get_filter(index.row());
	filter.set_mode(Library::Filter::Date);
	filter.set_date_filter(date_filter);
	m->library->psl_filter_changed(filter);
}


Library::TrackDeletionMode GUI_LocalLibrary::show_delete_dialog(int n_tracks)
{
	LibraryDeleteDialog dialog(n_tracks, this);
	dialog.exec();
	return dialog.answer();
}


void GUI_LocalLibrary::disc_pressed(int disc)
{
	m->library->psl_disc_pressed(disc);
}


void GUI_LocalLibrary::progress_changed(const QString& type, int progress)
{
	ui->pb_progress->setVisible(progress >= 0);
	ui->lab_progress->setVisible(progress >= 0);

	ui->lab_progress->setText(type);

	if(progress == 0){
		if(ui->pb_progress->maximum() != 0){
			ui->pb_progress->setMaximum(0);
		}
	}

	if(progress > 0){
		if(ui->pb_progress->maximum() != 100){
			ui->pb_progress->setMaximum(100);
		}

		ui->pb_progress->setValue(progress);
	}
}

void GUI_LocalLibrary::genres_reloaded()
{
	if(ui->lv_genres->row_count() == 0){
		ui->stacked_genre_widget->setCurrentIndex(1);
	}

	else{
		ui->stacked_genre_widget->setCurrentIndex(0);
	}
}

void GUI_LocalLibrary::reload_library_requested()
{
	 reload_library_requested(Library::ReloadQuality::Unknown);
}

void GUI_LocalLibrary::reload_library_requested(Library::ReloadQuality quality)
{
	if(quality == Library::ReloadQuality::Unknown)
	{
		int n_rows = ui->tb_title->model()->rowCount();
		if(n_rows > 0){
			quality = show_quality_dialog();

			if(quality == Library::ReloadQuality::Unknown){
				return;
			}
		}

		else {
			quality = Library::ReloadQuality::Accurate;
		}
	}

	m->library->psl_reload_library(false, quality);
	ui->btn_reload_library->setVisible(false);
}

void GUI_LocalLibrary::reload_finished()
{
	ui->btn_reload_library->setVisible(false);

	if(ui->lv_genres->row_count() <= 1){
		ui->stacked_genre_widget->setCurrentIndex(1);
	}
	else{
		ui->stacked_genre_widget->setCurrentIndex(0);
	}
}

void GUI_LocalLibrary::show_info_box()
{
	if(!m->library_info_box){
		m->library_info_box = new GUI_LibraryInfoBox(
								   m->library->library_id(),
								   this);
	}

	m->library_info_box->psl_refresh();
}


void GUI_LocalLibrary::import_dirs_requested()
{
	QStringList dirs;

	QFileDialog* dialog = new QFileDialog(this);
	dialog->setDirectory(QDir::homePath());
	dialog->setWindowTitle(Lang::get(Lang::ImportDir));
	dialog->setFileMode(QFileDialog::DirectoryOnly);
	dialog->setOption(QFileDialog::DontUseNativeDialog, true);
	QList<QUrl> sidebar_urls = dialog->sidebarUrls();

	QList<QStandardPaths::StandardLocation> locations;
	locations << QStandardPaths::HomeLocation;
	locations << QStandardPaths::DesktopLocation;
	locations << QStandardPaths::DownloadLocation;
	locations << QStandardPaths::MusicLocation;
	locations << QStandardPaths::TempLocation;

	for(QStandardPaths::StandardLocation location : locations)
	{
		QStringList std_locations = QStandardPaths::standardLocations(location);
		for(const QString& std_location : std_locations){
			QUrl url = QUrl::fromLocalFile(std_location);
			if(sidebar_urls.contains(url)){
				continue;
			}

			sidebar_urls << url;
		}
	}

	dialog->setSidebarUrls(sidebar_urls);

	QListView* list_view = dialog->findChild<QListView*>("listView");
	if(list_view == nullptr)
	{
		delete dialog;

		QString dir = QFileDialog::getExistingDirectory(this, Lang::get(Lang::ImportDir),
														m->library->library_path(),
														QFileDialog::ShowDirsOnly);
		if(!dir.isEmpty()){
			dirs << dir;
		}
	}

	else{
		list_view->setSelectionMode(QAbstractItemView::MultiSelection);
		QTreeView* tree_view = dialog->findChild<QTreeView*>();
		if(tree_view){
			tree_view->setSelectionMode(QAbstractItemView::MultiSelection);
		}

		if(dialog->exec() == QFileDialog::Accepted){
			dirs = dialog->selectedFiles();
		}
	}

	if(!dirs.isEmpty()){
		m->library->import_files(dirs);
	}
}

void GUI_LocalLibrary::import_files_requested()
{
	QStringList extensions = Helper::soundfile_extensions();
	QString filter = QString("Soundfiles (") + extensions.join(" ") + ")";
	QStringList files = QFileDialog::getOpenFileNames(this, Lang::get(Lang::ImportFiles),
													  QDir::homePath(), filter);

	if(files.size() > 0) {
		m->library->import_files(files);
	}
}


void GUI_LocalLibrary::import_files(const QStringList& files)
{
	m->library->import_files(files);
}

void GUI_LocalLibrary::change_library_name(const QString& name)
{
	m->library->set_library_name(name);
}

void GUI_LocalLibrary::change_library_path(const QString& path)
{
	m->library->set_library_path(path);
	reload_library_requested(Library::ReloadQuality::Accurate);
}

void GUI_LocalLibrary::name_changed(const QString& name)
{
	m->library_menu->refresh_name(name);
}


void GUI_LocalLibrary::path_changed(const QString& path)
{
	m->library_menu->refresh_path(path);
	ui->btn_reload_library->setVisible(true);
}

void GUI_LocalLibrary::import_dialog_requested()
{
	if(!m->ui_importer){
		m->ui_importer = new GUI_ImportFolder(m->library, true, this);
	}

	m->ui_importer->show();
}

void GUI_LocalLibrary::splitter_artist_moved(int pos, int idx)
{
	Q_UNUSED(pos)
	Q_UNUSED(idx)

	QByteArray arr = ui->splitter_artist_album->saveState();
	_settings->set(Set::Lib_SplitterStateArtist, arr);
}

void GUI_LocalLibrary::splitter_tracks_moved(int pos, int idx)
{
	Q_UNUSED(pos)
	Q_UNUSED(idx)

	QByteArray arr = ui->splitter_tracks->saveState();
	_settings->set(Set::Lib_SplitterStateTrack, arr);
}

void GUI_LocalLibrary::splitter_genre_moved(int pos, int idx)
{
	Q_UNUSED(pos)
	Q_UNUSED(idx)

	QByteArray arr = ui->splitter_genre->saveState();
	_settings->set(Set::Lib_SplitterStateGenre, arr);
}

void GUI_LocalLibrary::splitter_date_moved(int pos, int idx)
{
	Q_UNUSED(pos)
	Q_UNUSED(idx)

	QByteArray arr = ui->splitter_date->saveState();
	_settings->set(Set::Lib_SplitterStateDate, arr);
}



void GUI_LocalLibrary::init_album_cover_view()
{
	if(m->acv){
		return;
	}

	m->acv = new AlbumCoverView(ui->page_4);
	QLayout* layout = ui->page_4->layout();
	if(layout){
		layout->addWidget(m->acv);
	}

	m->acm = new AlbumCoverModel(m->acv);
	m->acv->setModel(m->acm);

	connect(m->acv, &LibraryView::doubleClicked, this, &GUI_LocalLibrary::album_dbl_clicked);
	connect(m->acv, &LibraryView::sig_sel_changed, this, &GUI_LocalLibrary::album_sel_changed);
	connect(m->acv, &LibraryView::sig_middle_button_clicked, this, &GUI_LocalLibrary::album_middle_clicked);
	connect(m->acv, &LibraryView::sig_play_next_clicked, this, &GUI_LocalLibrary::play_next);
	connect(m->acv, &LibraryView::sig_append_clicked, this, &GUI_LocalLibrary::append);
	connect(m->acv, &LibraryView::sig_merge, m->library, &LocalLibrary::merge_albums);

	m->acv->show();
}


void GUI_LocalLibrary::lib_fill_albums(const AlbumList& albums)
{
	GUI_AbstractLibrary::lib_fill_albums(albums);

	if(!m->acv){
		return;
	}

	m->acm->set_data(albums);
	m->acv->refresh();
}


void GUI_LocalLibrary::lib_fill_tracks(const MetaDataList& v_md)
{
	GUI_AbstractLibrary::lib_fill_tracks(v_md);

	if(m->acm){
		m->acm->set_mimedata(v_md);
	}
}



LibraryTableView* GUI_LocalLibrary::lv_artist() const
{
	return ui->lv_artist;
}

LibraryTableView* GUI_LocalLibrary::lv_album() const
{
	return ui->lv_album;
}

LibraryTableView* GUI_LocalLibrary::lv_tracks() const
{
	return ui->tb_title;
}

QLineEdit* GUI_LocalLibrary::le_search() const
{
    return ui->le_search;
}

QList<Library::Filter::Mode> GUI_LocalLibrary::search_options() const
{
    return {
        ::Library::Filter::Fulltext,
        ::Library::Filter::Filename,
        ::Library::Filter::Genre
    };
}
