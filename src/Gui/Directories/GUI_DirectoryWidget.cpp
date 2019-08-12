/* GUI_DirectoryWidget.cpp */

/* Copyright (C) 2011-2019  Lucio Carreras
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

#include "GUI_DirectoryWidget.h"
#include "FileListModel.h"
#include "DirectoryModel.h"

#include "Gui/Directories/ui_GUI_DirectoryWidget.h"
#include "Gui/Library/TrackModel.h"
#include "Gui/ImportDialog/GUI_ImportDialog.h"

#include "Gui/Utils/Library/GUI_EditLibrary.h"
#include "Gui/Utils/ContextMenu/LibraryContextMenu.h"
#include "Gui/Utils/Icons.h"
#include "Gui/Utils/EventFilter.h"
#include "Gui/Utils/PreferenceAction.h"

#include "Components/Library/LibraryManager.h"
#include "Components/Library/LocalLibrary.h"
#include "Components/Playlist/PlaylistHandler.h"
#include "Components/Directories/DirectoryReader.h"

#include "Database/LibraryDatabase.h"
#include "Database/Connector.h"

#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Message/Message.h"
#include "Utils/Library/LibraryNamespaces.h"
#include "Utils/Library/LibraryInfo.h"
#include "Utils/FileUtils.h"
#include "Utils/globals.h"
#include "Utils/Language/Language.h"
#include "Utils/Settings/Settings.h"

#include <QItemSelectionModel>
#include <QApplication>
#include <QMouseEvent>
#include <QShortcut>
#include <QMenu>

struct GUI_DirectoryWidget::Private
{
	enum SelectedWidget
	{
		None=0,
		Dirs,
		Files
	} selected_widget;

	LocalLibrary*			generic_library=nullptr;
	bool					is_search_active;

	Private() :
		selected_widget(None),
		is_search_active(false)
	{}
};

GUI_DirectoryWidget::GUI_DirectoryWidget(QWidget *parent) :
	Widget(parent),
	InfoDialogContainer()
{
	ui = new Ui::GUI_DirectoryWidget();
	ui->setupUi(this);

	ui->splitter_dir_files->restoreState(GetSetting(Set::Dir_SplitterDirFile));
	ui->splitter_tracks->restoreState(GetSetting(Set::Dir_SplitterTracks));

	m = Pimpl::make<GUI_DirectoryWidget::Private>();

	m->selected_widget = Private::SelectedWidget::None;
	Library::Manager* library_manager = Library::Manager::instance();
	QList<Library::Info> all_libraries = library_manager->all_libraries();

	m->generic_library = library_manager->library_instance(-1);

	ui->tb_title->init(m->generic_library);

	connect(ui->btn_search, &QPushButton::clicked, this, &GUI_DirectoryWidget::search_button_clicked);
	connect(ui->le_search, &QLineEdit::returnPressed, this, &GUI_DirectoryWidget::search_button_clicked);
	connect(ui->le_search, &QLineEdit::textChanged, this, &GUI_DirectoryWidget::search_text_edited);

	ui->tv_dirs->setExpandsOnDoubleClick(true);
	ui->tv_dirs->setDragEnabled(true);
	ui->tv_dirs->setAcceptDrops(true);
	ui->tv_dirs->setDragDropMode(QAbstractItemView::DragDrop);
	ui->tv_dirs->setDropIndicatorShown(true);

	ui->lv_files->setDragEnabled(true);
	ui->lv_files->setAcceptDrops(true);
	ui->lv_files->setDragDropMode(QAbstractItemView::DragDrop);
	ui->lv_files->setDropIndicatorShown(true);

	connect(ui->tv_dirs, &DirectoryTreeView::sig_current_index_changed, this, &GUI_DirectoryWidget::dir_clicked);
	//connect(ui->tv_dirs, &QTreeView::pressed, this, &GUI_DirectoryWidget::dir_pressed);
	connect(ui->tv_dirs, &DirectoryTreeView::sig_import_requested, this, &GUI_DirectoryWidget::import_requested);
	connect(ui->tv_dirs, &DirectoryTreeView::sig_enter_pressed, this, &GUI_DirectoryWidget::dir_enter_pressed);
	connect(ui->tv_dirs, &DirectoryTreeView::sig_append_clicked, this, &GUI_DirectoryWidget::dir_append_clicked);
	connect(ui->tv_dirs, &DirectoryTreeView::sig_play_clicked, this, &GUI_DirectoryWidget::dir_play_clicked);
	connect(ui->tv_dirs, &DirectoryTreeView::sig_play_next_clicked, this, &GUI_DirectoryWidget::dir_play_next_clicked);
	connect(ui->tv_dirs, &DirectoryTreeView::sig_play_new_tab_clicked, this, &GUI_DirectoryWidget::dir_play_new_tab_clicked);
	connect(ui->tv_dirs, &DirectoryTreeView::sig_delete_clicked, this, &GUI_DirectoryWidget::dir_delete_clicked);
	connect(ui->tv_dirs, &DirectoryTreeView::sig_directory_loaded, this, &GUI_DirectoryWidget::dir_opened);

	connect(ui->tv_dirs, &DirectoryTreeView::sig_info_clicked, this, [=]()
	{
		m->selected_widget = Private::SelectedWidget::Dirs;
		show_info();
	});

	connect(ui->tv_dirs, &DirectoryTreeView::sig_edit_clicked, this, [=]()
	{
		m->selected_widget = Private::SelectedWidget::Dirs;
		show_edit();
	});

	connect(ui->tv_dirs, &DirectoryTreeView::sig_lyrics_clicked, this, [=]()
	{
		m->selected_widget = Private::SelectedWidget::Dirs;
		show_lyrics();
	});

	connect(ui->lv_files, &QListView::pressed, this, &GUI_DirectoryWidget::file_pressed);
	connect(ui->lv_files, &QListView::doubleClicked, this, &GUI_DirectoryWidget::file_dbl_clicked);
	connect(ui->lv_files, &FileListView::sig_import_requested, this, &GUI_DirectoryWidget::import_requested);
	connect(ui->lv_files, &FileListView::sig_enter_pressed, this, &GUI_DirectoryWidget::file_enter_pressed);
	connect(ui->lv_files, &FileListView::sig_append_clicked, this, &GUI_DirectoryWidget::file_append_clicked);
	connect(ui->lv_files, &FileListView::sig_play_clicked, this, &GUI_DirectoryWidget::file_play_clicked);
	connect(ui->lv_files, &FileListView::sig_play_next_clicked, this, &GUI_DirectoryWidget::file_play_next_clicked);
	connect(ui->lv_files, &FileListView::sig_play_new_tab_clicked, this, &GUI_DirectoryWidget::file_play_new_tab_clicked);
	connect(ui->lv_files, &FileListView::sig_delete_clicked, this, &GUI_DirectoryWidget::file_delete_clicked);
	connect(ui->lv_files, &FileListView::sig_info_clicked, this, [=]()
	{
		m->selected_widget = Private::SelectedWidget::Files;
		show_info();
	});

	connect(ui->lv_files, &FileListView::sig_edit_clicked, this, [=]()
	{
		m->selected_widget = Private::SelectedWidget::Files;
		show_edit();
	});

	connect(ui->lv_files, &FileListView::sig_lyrics_clicked, this, [=]()
	{
		m->selected_widget = Private::SelectedWidget::Files;
		show_lyrics();
	});

	connect(ui->splitter_dir_files, &QSplitter::splitterMoved, this, &GUI_DirectoryWidget::splitter_moved);
	connect(ui->splitter_tracks, &QSplitter::splitterMoved, this, &GUI_DirectoryWidget::splitter_moved);
	connect(ui->btn_set_library_path, &QPushButton::clicked, this, &GUI_DirectoryWidget::set_lib_path_clicked);

	connect(library_manager, &Library::Manager::sig_added, this, [=](LibraryId id){
		Q_UNUSED(id)
		check_libraries();
	});

	connect(library_manager, &Library::Manager::sig_removed, this, [=](LibraryId id){
		Q_UNUSED(id)
		check_libraries();
	});

	auto* search_context_menu = new QMenu(ui->le_search);
	auto* action = new Gui::SearchPreferenceAction(ui->le_search);
	search_context_menu->addActions({action});

	auto* cmf = new Gui::ContextMenuFilter(ui->le_search);
	connect(cmf, &Gui::ContextMenuFilter::sig_context_menu, search_context_menu, &QMenu::popup);
	ui->le_search->installEventFilter(cmf);

	init_shortcuts();
	check_libraries();
}

GUI_DirectoryWidget::~GUI_DirectoryWidget()
{
	if(ui){
		delete ui; ui = nullptr;
	}
}

QFrame* GUI_DirectoryWidget::header_frame() const
{
	return ui->header_frame;
}

MD::Interpretation GUI_DirectoryWidget::metadata_interpretation() const
{
	return MD::Interpretation::Tracks;
}

MetaDataList GUI_DirectoryWidget::info_dialog_data() const
{
	MetaDataList v_md;

	switch(m->selected_widget)
	{
		case Private::SelectedWidget::Dirs:
			return ui->tv_dirs->selected_metadata();
		case Private::SelectedWidget::Files:
			return ui->lv_files->selected_metadata();
		default:
			return v_md;
	}
}


void GUI_DirectoryWidget::dir_enter_pressed()
{
	QModelIndexList indexes = ui->tv_dirs->selected_indexes();
	if(!indexes.isEmpty()){
		ui->tv_dirs->expand(indexes.first());
	}
}


void GUI_DirectoryWidget::dir_pressed(QModelIndex idx)
{
	Q_UNUSED(idx)

	Qt::MouseButtons buttons = QApplication::mouseButtons();

	QStringList paths = ui->tv_dirs->selected_paths();
	if(buttons & Qt::MiddleButton)
	{
		if(!paths.isEmpty())
		{
			m->generic_library->prepare_tracks_for_playlist(paths, true);
		}
	}
}

void GUI_DirectoryWidget::dir_clicked(QModelIndex idx)
{
	m->is_search_active = false;
	ui->le_search->clear();
	ui->lv_files->clearSelection();

	dir_opened(idx);
}

#include "Utils/Logger/Logger.h"
void GUI_DirectoryWidget::dir_opened(QModelIndex idx)
{
	QModelIndexList selected_items = ui->tv_dirs->selected_indexes();

	QString dir = ui->tv_dirs->directory_name_origin(idx);

	sp_log(Log::Debug, this) << "Dir openend " << idx.data().toString() << " vs " << dir;

	QStringList dirs;
	for(const QModelIndex& index : selected_items){
		dirs << ui->tv_dirs->directory_name_origin(index);
	 }

	sp_log(Log::Debug, this) << " dirs: " << dirs;
	ui->lv_files->set_parent_directory(ui->tv_dirs->library_id(idx), dir);
	ui->lv_files->set_search_filter(ui->le_search->text());

	if(ui->lv_files->selected_metadata().isEmpty())
	{
		m->generic_library->fetch_tracks_by_paths(dirs);
	}
}


void GUI_DirectoryWidget::dir_append_clicked()
{
	MetaDataList v_md = ui->tv_dirs->selected_metadata();
	Playlist::Handler* plh = Playlist::Handler::instance();
	plh->append_tracks(v_md, plh->current_index());
}

void GUI_DirectoryWidget::dir_play_clicked()
{
	QStringList paths = ui->tv_dirs->selected_paths();
	m->generic_library->prepare_tracks_for_playlist(paths, false);
}

void GUI_DirectoryWidget::dir_play_next_clicked()
{
	MetaDataList v_md = ui->tv_dirs->selected_metadata();
	Playlist::Handler* plh = Playlist::Handler::instance();
	plh->play_next(v_md);
}

void GUI_DirectoryWidget::dir_play_new_tab_clicked()
{
	MetaDataList v_md = ui->tv_dirs->selected_metadata();
	Playlist::Handler* plh = Playlist::Handler::instance();
	plh->create_playlist(v_md, plh->request_new_playlist_name());
}


void GUI_DirectoryWidget::dir_delete_clicked()
{
	Message::Answer answer = Message::question_yn(Lang::get(Lang::Delete) + ": " + Lang::get(Lang::Really) + "?");

	if(answer != Message::Answer::Yes){
		return;
	}

	QStringList files = ui->tv_dirs->selected_paths();
	MetaDataList v_md = ui->tv_dirs->selected_metadata();

	m->generic_library->delete_tracks(v_md, Library::TrackDeletionMode::OnlyLibrary);

	Util::File::delete_files(files);
}


void GUI_DirectoryWidget::file_append_clicked()
{
	MetaDataList v_md = ui->lv_files->selected_metadata();
	Playlist::Handler* plh = Playlist::Handler::instance();
	plh->append_tracks(v_md, plh->current_index());
}

void GUI_DirectoryWidget::file_play_clicked()
{
	QStringList paths = ui->lv_files->selected_paths();
	m->generic_library->prepare_tracks_for_playlist(paths, false);
}


void GUI_DirectoryWidget::file_play_next_clicked()
{
	MetaDataList v_md = ui->lv_files->selected_metadata();
	Playlist::Handler* plh = Playlist::Handler::instance();
	plh->play_next(v_md);
}

void GUI_DirectoryWidget::file_play_new_tab_clicked()
{
	MetaDataList v_md = ui->lv_files->selected_metadata();
	Playlist::Handler* plh = Playlist::Handler::instance();
	plh->create_playlist(v_md, plh->request_new_playlist_name());
}


void GUI_DirectoryWidget::file_delete_clicked()
{
	Message::Answer answer = Message::question_yn(Lang::get(Lang::Delete) + ": " + Lang::get(Lang::Really) + "?");

	if(answer != Message::Answer::Yes){
		return;
	}

	MetaDataList v_md = ui->lv_files->selected_metadata();

	m->generic_library->delete_tracks(v_md, Library::TrackDeletionMode::OnlyLibrary);

	QStringList files = ui->lv_files->selected_paths();
	Util::File::delete_files(files);
}

void GUI_DirectoryWidget::import_requested(LibraryId id, const QStringList& paths, const QString& target_dir)
{
	Library::Manager* library_manager = Library::Manager::instance();
	LocalLibrary* library = library_manager->library_instance(id);
	if(!library){
		return;
	}

	connect(library, &LocalLibrary::sig_import_dialog_requested,
			this, &GUI_DirectoryWidget::import_dialog_requested);

	library->import_files_to(paths, target_dir);
}

void GUI_DirectoryWidget::import_dialog_requested(const QString& target_dir)
{
	if(!this->isVisible()){
		return;
	}

	LocalLibrary* library = dynamic_cast<LocalLibrary*>(sender());
	if(!library){
		return;
	}

	GUI_ImportDialog* importer = new GUI_ImportDialog(library, true, this);

	connect(importer, &GUI_ImportDialog::sig_closed, importer, &GUI_ImportDialog::deleteLater);

	importer->set_target_dir(target_dir);
	importer->show();
}

void GUI_DirectoryWidget::file_pressed(QModelIndex idx)
{
	Q_UNUSED(idx)

	Qt::MouseButtons buttons = QApplication::mouseButtons();
	QStringList paths = ui->lv_files->selected_paths();

	if(buttons & Qt::MiddleButton)
	{
		m->generic_library->prepare_tracks_for_playlist(paths, true);
	}

	m->generic_library->fetch_tracks_by_paths(paths);
}


void GUI_DirectoryWidget::file_dbl_clicked(QModelIndex idx)
{
	Q_UNUSED(idx)

	QStringList paths = ui->lv_files->selected_paths();
	m->generic_library->prepare_tracks_for_playlist(paths, false);
}

void GUI_DirectoryWidget::file_enter_pressed()
{
	QStringList paths = ui->lv_files->selected_paths();
	m->generic_library->prepare_tracks_for_playlist(paths, false);
}

void GUI_DirectoryWidget::search_button_clicked()
{
	if(ui->le_search->text().isEmpty()){
		m->is_search_active	= false;
		return;
	}

	Library::Filter filter;
	filter.set_filtertext(ui->le_search->text(), GetSetting(Set::Lib_SearchMode));
	filter.set_mode(Library::Filter::Mode::Filename);
	m->generic_library->change_filter(filter);

	QModelIndex found_idx = ui->tv_dirs->search(ui->le_search->text());
	if(found_idx.isValid()){
		dir_opened(found_idx);
		ui->btn_search->setText(Lang::get(Lang::SearchNext));
		m->is_search_active	= true;
	}
}

void GUI_DirectoryWidget::search_text_edited(const QString& text)
{
	Q_UNUSED(text)
	m->is_search_active = false;
	ui->btn_search->setText(Lang::get(Lang::SearchVerb));
}


void GUI_DirectoryWidget::init_shortcuts()
{
	new QShortcut(QKeySequence("Ctrl+f"), ui->le_search, SLOT(setFocus()), nullptr, Qt::WindowShortcut);
	new QShortcut(QKeySequence("Esc"), ui->le_search, SLOT(clear()), nullptr, Qt::WidgetShortcut);
}

void GUI_DirectoryWidget::language_changed()
{
	ui->retranslateUi(this);

	if(m->is_search_active) {
		ui->btn_search->setText(Lang::get(Lang::SearchNext));
	}

	else{
		ui->btn_search->setText(Lang::get(Lang::SearchVerb));
	}

	ui->btn_set_library_path->setText(Lang::get(Lang::CreateNewLibrary));
}

void GUI_DirectoryWidget::skin_changed()
{
	using namespace Gui;
	ui->btn_search->setIcon(Icons::icon(Icons::Search));
}


void GUI_DirectoryWidget::splitter_moved(int pos, int index)
{
	Q_UNUSED(pos)
	Q_UNUSED(index)

	SetSetting(Set::Dir_SplitterDirFile, ui->splitter_dir_files->saveState());
	SetSetting(Set::Dir_SplitterTracks, ui->splitter_tracks->saveState());
}

void GUI_DirectoryWidget::set_lib_path_clicked()
{
	GUI_EditLibrary* new_library = new GUI_EditLibrary(this);
	connect(new_library, &GUI_EditLibrary::sig_accepted, this, &GUI_DirectoryWidget::new_library_created);

	new_library->reset();
	new_library->show();
}

void GUI_DirectoryWidget::new_library_created()
{
	GUI_EditLibrary* new_library = dynamic_cast<GUI_EditLibrary*>(sender());
	if(!new_library) {
		return;
	}

	QString name = new_library->name();
	QString path = new_library->path();

	Library::Manager* lib_manager = Library::Manager::instance();
	lib_manager->add_library(name, path);
}


void GUI_DirectoryWidget::check_libraries()
{
	Library::Manager* lib_manager = Library::Manager::instance();
	if(lib_manager->count() == 0)
	{
		ui->stackedWidget->setCurrentIndex(1);
		ui->widget_search->setVisible(false);

	}

	else {
		ui->stackedWidget->setCurrentIndex(0);
		ui->widget_search->setVisible(true);
	}
}

