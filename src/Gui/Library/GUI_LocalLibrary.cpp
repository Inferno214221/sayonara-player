/* GUI_LocalLibrary.cpp */

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
 * GUI_LocalLibrary.cpp
 *
 *  Created on: Apr 24, 2011
 *      Author: Lucio Carreras
 */

#include "GUI_LocalLibrary.h"
#include "Gui/Library/ui_GUI_LocalLibrary.h"

#include "Gui/Library/GUI_CoverView.h"
#include "Gui/Library/Utils/DirChooserDialog.h"
#include "Gui/Library/Utils/GUI_ReloadLibraryDialog.h"
#include "Gui/Library/Utils/GUI_LibraryInfoBox.h"
#include "Gui/Library/Utils/LocalLibraryMenu.h"

#include "Gui/ImportDialog/GUI_ImportDialog.h"
#include "Gui/Utils/Library/GUI_DeleteDialog.h"
#include "Gui/Utils/GuiUtils.h"
#include "Gui/Utils/Icons.h"
#include "Gui/Utils/Style.h"

#include "Components/Library/LocalLibrary.h"
#include "Components/Library/LibraryManager.h"

#include "Utils/Utils.h"
#include "Utils/Set.h"
#include "Utils/Language/Language.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Library/LibraryInfo.h"
#include "Utils/Logger/Logger.h"

#include <QDir>
#include <QFileDialog>
#include <QStringList>

enum StatusWidgetIndex
{
	ReloadLibraryIndex=0,
	FileExtensionsIndex=1
};

enum GenreWidgetIndex
{
	GenreTree=0,
	NoGenres=1
};

enum AlbumViewIndex
{
	ArtistAlbumTableView=0,
	AlbumCoverView=1
};

enum ReloadWidgetIndex
{
	StandardView=0,
	ReloadView=1
};

using namespace Library;

struct GUI_LocalLibrary::Private
{
	LocalLibrary*			library = nullptr;
	LocalLibraryMenu*		library_menu = nullptr;

	Private(LibraryId id, GUI_LocalLibrary* parent)
	{
		library = Manager::instance()->library_instance(id);
		library_menu = new LocalLibraryMenu(library->name(), library->path(), parent);
	}
};


GUI_LocalLibrary::GUI_LocalLibrary(LibraryId id, QWidget* parent) :
	GUI_AbstractLibrary(Manager::instance()->library_instance(id), parent)
{
	m = Pimpl::make<Private>(id, this);

	setup_parent(this, &ui);

	connect(m->library, &LocalLibrary::sig_reloading_library, this, &GUI_LocalLibrary::progress_changed);
	connect(m->library, &LocalLibrary::sig_reloading_library_finished, this, &GUI_LocalLibrary::reload_finished);
	connect(m->library, &LocalLibrary::sig_reloading_library_finished, ui->lv_genres, &GenreView::reload_genres);
	connect(m->library, &LocalLibrary::sig_all_tracks_loaded, this, &GUI_LocalLibrary::tracks_loaded);
	connect(m->library, &LocalLibrary::sig_import_dialog_requested, this, &GUI_LocalLibrary::import_dialog_requested);
	connect(m->library, &LocalLibrary::sig_filter_changed, this, &GUI_LocalLibrary::filter_changed);

	auto* manager = Manager::instance();
	connect(manager, &Manager::sig_path_changed, this, &GUI_LocalLibrary::path_changed);
	connect(manager, &Manager::sig_renamed, this, &GUI_LocalLibrary::name_changed);

	connect(ui->tv_albums, &AlbumView::sig_disc_pressed, m->library, &LocalLibrary::change_current_disc);
	connect(ui->lv_genres, &GenreView::sig_selected_changed, this, &GUI_LocalLibrary::genre_selection_changed);
	connect(ui->lv_genres, &GenreView::sig_invalid_genre_selected, this, &GUI_LocalLibrary::invalid_genre_selected);
	connect(ui->lv_genres, &GenreView::sig_progress, this, &GUI_LocalLibrary::progress_changed);

	connect(m->library_menu, &LocalLibraryMenu::sig_path_changed, m->library, &LocalLibrary::set_library_path);
	connect(m->library_menu, &LocalLibraryMenu::sig_name_changed, m->library, &LocalLibrary::set_library_name);
	connect(m->library_menu, &LocalLibraryMenu::sig_import_file, this, &GUI_LocalLibrary::import_files_requested);
	connect(m->library_menu, &LocalLibraryMenu::sig_import_folder, this, &GUI_LocalLibrary::import_dirs_requested);
	connect(m->library_menu, &LocalLibraryMenu::sig_info, this, &GUI_LocalLibrary::show_info_box);
	connect(m->library_menu, &LocalLibraryMenu::sig_reload_library, this, &GUI_LocalLibrary::reload_library_requested);

	connect(ui->btn_reload_library, &QPushButton::clicked, this, &GUI_LocalLibrary::reload_library_deep_requested);
	connect(ui->btn_reload_library_small, &QPushButton::clicked, this, &GUI_LocalLibrary::reload_library_deep_requested);

	connect(ui->splitter_artist_album, &QSplitter::splitterMoved, this, &GUI_LocalLibrary::splitter_artist_moved);
	connect(ui->splitter_tracks, &QSplitter::splitterMoved, this, &GUI_LocalLibrary::splitter_tracks_moved);
	connect(ui->splitter_genre, &QSplitter::splitterMoved, this, &GUI_LocalLibrary::splitter_genre_moved);

	connect(ui->tv_albums, &ItemView::sig_reload_clicked, this, &GUI_LocalLibrary::reload_library_requested);
	connect(ui->tv_artists, &ItemView::sig_reload_clicked, this, &GUI_LocalLibrary::reload_library_requested);
	connect(ui->tv_tracks, &ItemView::sig_reload_clicked, this, &GUI_LocalLibrary::reload_library_requested);

	connect(ui->btn_cancel_reload_widget, &QPushButton::clicked, this, [=](){
		ui->stacked_widget_reload->setCurrentIndex( int(ReloadWidgetIndex::StandardView) );
	});

	ui->extension_bar->init(m->library);
	ui->lv_genres->init(m->library);

	ListenSetting(Set::Lib_ShowAlbumCovers, GUI_LocalLibrary::switch_album_view);
	ListenSetting(Set::Lib_ShowFilterExtBar, GUI_LocalLibrary::tracks_loaded);

	m->library->load();	
}

GUI_LocalLibrary::~GUI_LocalLibrary()
{
	delete ui; ui = nullptr;
}

void GUI_LocalLibrary::language_changed()
{
	ui->retranslateUi(this);
	ui->gb_genres->setTitle(Lang::get(Lang::Genres));
	ui->btn_reload_library->setText(Lang::get(Lang::ReloadLibrary));
	ui->btn_reload_library_small->setText(Lang::get(Lang::ReloadLibrary));
	ui->btn_cancel_reload_widget->setText(Lang::get(Lang::Cancel));

	GUI_AbstractLibrary::language_changed();
}

void GUI_LocalLibrary::skin_changed()
{
	GUI_AbstractLibrary::skin_changed();

	check_view_state();
}

void GUI_LocalLibrary::check_view_state()
{
	if(this->isVisible())
	{
		check_reload_status();

		if(!m->library->is_reloading())
		{
			check_file_extension_bar();
		}
	}
}

void GUI_LocalLibrary::check_reload_status()
{
	bool is_reloading = m->library->is_reloading();
	bool is_library_empty = m->library->is_empty();
	bool in_library_state = (is_reloading || !is_library_empty);

	ReloadWidgetIndex index = (in_library_state == true) ? ReloadWidgetIndex::StandardView : ReloadWidgetIndex::ReloadView;
	ui->stacked_widget_reload->setCurrentIndex( int(index) );

	ui->pb_progress->setVisible(is_reloading);
	ui->lab_progress->setVisible(is_reloading);

	ui->sw_bottom_bar->setVisible(is_reloading || is_library_empty);
	ui->sw_bottom_bar->setCurrentIndex(StatusWidgetIndex::ReloadLibraryIndex);

	ui->le_search->setVisible(in_library_state);
	ui->btn_reload_library->setVisible(!in_library_state);
	ui->btn_reload_library_small->setVisible(!in_library_state);
}

void GUI_LocalLibrary::check_file_extension_bar()
{
	if(m->library->is_reloading() || m->library->is_empty()) {
		return;
	}

	if(!GetSetting(Set::Lib_ShowFilterExtBar))
	{
		ui->sw_bottom_bar->setVisible(false);
		return;
	}

	ui->extension_bar->refresh();

	ui->sw_bottom_bar->setCurrentIndex(StatusWidgetIndex::FileExtensionsIndex);
	ui->sw_bottom_bar->setVisible(ui->extension_bar->has_extensions());
}


void GUI_LocalLibrary::tracks_loaded()
{
	check_view_state();

	ui->lab_library_name->setText(m->library->name());
	ui->lab_path->setText(Util::create_link(m->library->path(), Style::is_dark()));

	ui->btn_reload_library->setIcon(Gui::Icons::icon(Gui::Icons::Refresh));
	ui->btn_reload_library_small->setIcon(Gui::Icons::icon(Gui::Icons::Refresh));
}

void GUI_LocalLibrary::clear_selections()
{
	GUI_AbstractLibrary::clear_selections();

	if(ui->cover_view) {
		ui->cover_view->clear_selections();
	}

	ui->lv_genres->clearSelection();
}

void GUI_LocalLibrary::invalid_genre_selected()
{
	ui->le_search->set_invalid_genre_mode(true);
	ui->le_search->set_current_mode(Filter::Genre);
	ui->le_search->setText(GenreView::invalid_genre_name());

	search_triggered();

	ui->le_search->set_invalid_genre_mode(false);
}

void GUI_LocalLibrary::genre_selection_changed(const QStringList& genres)
{
	if(genres.isEmpty()) {
		return;
	}

	ui->le_search->set_invalid_genre_mode(false);
	ui->le_search->set_current_mode(Filter::Genre);
	ui->le_search->setText(genres.join(","));

	search_triggered();
}


TrackDeletionMode GUI_LocalLibrary::show_delete_dialog(int track_count)
{
	GUI_DeleteDialog dialog(track_count, this);
	dialog.exec();

	return dialog.answer();
}


void GUI_LocalLibrary::progress_changed(const QString& type, int progress)
{
	check_view_state();

	QFontMetrics fm(this->font());

	ui->pb_progress->setMaximum((progress > 0) ? 100 : 0);
	ui->pb_progress->setValue(progress);
	ui->lab_progress->setText(fm.elidedText(type, Qt::ElideRight, ui->sw_bottom_bar->width() / 2));
}

void GUI_LocalLibrary::reload_library_requested()
{
	reload_library_requested_with_quality(ReloadQuality::Unknown);
}

void GUI_LocalLibrary::reload_library_deep_requested()
{
	reload_library_requested_with_quality(ReloadQuality::Accurate);
}

void GUI_LocalLibrary::reload_library_requested_with_quality(ReloadQuality quality)
{
	if(quality == ReloadQuality::Unknown)
	{
		auto* dialog = new GUI_LibraryReloadDialog(m->library->name(), this);
		connect(dialog, &GUI_LibraryReloadDialog::sig_accepted, this, &GUI_LocalLibrary::reload_library_accepted);

		dialog->set_quality(quality);
		dialog->show();
	}

	else
	{
		reload_library(quality);
	}
}

void GUI_LocalLibrary::reload_library_accepted(ReloadQuality quality)
{
	if(sender()) {
		sender()->deleteLater();
	}

	reload_library(quality);
}


void GUI_LocalLibrary::reload_library(ReloadQuality quality)
{
	m->library_menu->set_library_busy(true);
	m->library->reload_library(false, quality);
}


void GUI_LocalLibrary::reload_finished()
{
	m->library_menu->set_library_busy(false);

	check_view_state();
}

void GUI_LocalLibrary::show_info_box()
{
	GUI_LibraryInfoBox(m->library->id(), this).exec();
}

void GUI_LocalLibrary::import_dirs_requested()
{
	DirChooserDialog dialog(this);

	QStringList dirs;
	if(dialog.exec() == QFileDialog::Accepted){
		dirs = dialog.selectedFiles();
	}

	m->library->import_files(dirs);
}

void GUI_LocalLibrary::import_files_requested()
{
	QStringList files = QFileDialog::getOpenFileNames
	(
		this,
		Lang::get(Lang::ImportFiles),
		QDir::homePath(),
		Util::get_file_filter(Util::Extensions(Util::Extension::Soundfile), tr("Audio files"))
	);

	m->library->import_files(files);
}


void GUI_LocalLibrary::name_changed(LibraryId id)
{
	if(m->library->id() != id) {
		return;
	}

	Info info = Manager::instance()->library_info(id);
	if(info.valid())
	{
		m->library_menu->refresh_name(info.name());
		ui->lab_library_name->setText(info.name());
	}
}


void GUI_LocalLibrary::path_changed(LibraryId id)
{
	if(m->library->id() != id) {
		return;
	}

	Info info = Manager::instance()->library_info(id);
	if(info.valid())
	{
		m->library_menu->refresh_path(info.path());

		if(this->isVisible())
		{
			reload_library_requested_with_quality(ReloadQuality::Accurate);
			ui->lab_path->setText(info.path());
		}
	}
}

void GUI_LocalLibrary::import_dialog_requested(const QString& target_dir)
{
	if(!this->isVisible()){
		return;
	}

	auto* ui_importer = new GUI_ImportDialog(m->library, true, this);
	ui_importer->set_target_dir(target_dir);

	connect(ui_importer, &Gui::Dialog::sig_closed, ui_importer, &QObject::deleteLater);
	ui_importer->show();
}

void GUI_LocalLibrary::splitter_artist_moved(int pos, int idx)
{
	Q_UNUSED(pos) Q_UNUSED(idx)

	QByteArray arr = ui->splitter_artist_album->saveState();
	SetSetting(Set::Lib_SplitterStateArtist, arr);
}

void GUI_LocalLibrary::splitter_tracks_moved(int pos, int idx)
{
	Q_UNUSED(pos) Q_UNUSED(idx)

	QByteArray arr = ui->splitter_tracks->saveState();
	SetSetting(Set::Lib_SplitterStateTrack, arr);
}

void GUI_LocalLibrary::splitter_genre_moved(int pos, int idx)
{
	Q_UNUSED(pos) Q_UNUSED(idx)

	QByteArray arr = ui->splitter_genre->saveState();
	SetSetting(Set::Lib_SplitterStateGenre, arr);
}


void GUI_LocalLibrary::switch_album_view()
{
	bool show_covers = GetSetting(Set::Lib_ShowAlbumCovers);
	if(!show_covers)
	{
		ui->sw_album_covers->setCurrentIndex(AlbumViewIndex::ArtistAlbumTableView);
	}

	else
	{
		if(!ui->cover_view->is_initialized())
		{
			ui->cover_view->init(m->library);
			connect(ui->cover_view, &GUI_CoverView::sig_delete_clicked, this, &GUI_LocalLibrary::item_delete_clicked);
			connect(ui->cover_view, &GUI_CoverView::sig_reload_clicked, this, &GUI_LocalLibrary::reload_library_requested);
		}

		if(m->library->is_loaded() && (!m->library->selected_artists().isEmpty()))
		{
			m->library->selected_artists_changed(IndexSet());
		}

		ui->sw_album_covers->setCurrentIndex(AlbumViewIndex::AlbumCoverView);
	}
}

void GUI_LocalLibrary::filter_changed()
{
	Filter filter = m->library->filter();
	ui->le_search->set_current_mode(filter.mode());
	ui->le_search->setText(filter.filtertext(false).join(","));
}

bool GUI_LocalLibrary::has_selections() const
{
	return GUI_AbstractLibrary::has_selections() ||
			(!ui->lv_genres->selectedItems().isEmpty()) ||
			(!ui->cover_view->selected_items().isEmpty());
}

QList<Filter::Mode> GUI_LocalLibrary::search_options() const
{
	return { Filter::Fulltext, Filter::Filename, Filter::Genre, Filter::Track };
}

void GUI_LocalLibrary::showEvent(QShowEvent* e)
{
	GUI_AbstractLibrary::showEvent(e);

	this->lv_album()->resizeRowsToContents();
	this->lv_artist()->resizeRowsToContents();
	this->lv_tracks()->resizeRowsToContents();

	QMap<QSplitter*, QByteArray> splitters
	{
		{ui->splitter_artist_album, GetSetting(Set::Lib_SplitterStateArtist)},
		{ui->splitter_tracks, GetSetting(Set::Lib_SplitterStateTrack)},
		{ui->splitter_genre, GetSetting(Set::Lib_SplitterStateGenre)}
	};

	for(auto it=splitters.begin(); it != splitters.end(); it++)
	{
		if(!it.value().isEmpty()) {
			it.key()->restoreState(it.value());
		}
	}

	check_view_state();
}

// GUI_AbstractLibrary
TableView* GUI_LocalLibrary::lv_artist() const { return ui->tv_artists; }
TableView* GUI_LocalLibrary::lv_album() const { return ui->tv_albums; }
TableView* GUI_LocalLibrary::lv_tracks() const { return ui->tv_tracks; }
SearchBar* GUI_LocalLibrary::le_search() const { return ui->le_search; }

// LocalLibraryContainer
QMenu* GUI_LocalLibrary::menu() const {	return m->library_menu; }
QFrame* GUI_LocalLibrary::header_frame() const { return ui->header_frame; }
