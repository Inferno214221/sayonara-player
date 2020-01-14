/* GUI_DirectoryWidget.cpp */

/* Copyright (C) 2011-2020  Lucio Carreras
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

#include "Components/LibraryManagement/LibraryManager.h"
#include "Components/Library/LocalLibrary.h"
#include "Components/Directories/DirectorySelectionHandler.h"

#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Message/Message.h"
#include "Utils/Library/LibraryNamespaces.h"
#include "Utils/Library/LibraryInfo.h"
#include "Utils/FileUtils.h"
#include "Utils/globals.h"
#include "Utils/Language/Language.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Utils.h"
#include "Utils/Algorithm.h"

#include <QItemSelectionModel>
#include <QApplication>
#include <QMouseEvent>
#include <QShortcut>
#include <QMenu>
#include <QHeaderView>
#include <QFileDialog>

struct GUI_DirectoryWidget::Private
{
	DirectorySelectionHandler* dsh=nullptr;

	enum SelectedWidget
	{
		None=0,
		Dirs,
		Files
	} selected_widget;

	bool					is_search_active;

	Private() :
		selected_widget(None),
		is_search_active(false)
	{
		dsh = new DirectorySelectionHandler();
	}
};


GUI_DirectoryWidget::GUI_DirectoryWidget(QWidget *parent) :
	Widget(parent),
	InfoDialogContainer()
{
	ui = new Ui::GUI_DirectoryWidget();
	ui->setupUi(this);

	ui->splitter_dirs->restoreState(GetSetting(Set::Dir_SplitterDirFile));
	ui->splitter_tracks->restoreState(GetSetting(Set::Dir_SplitterTracks));

	m = Pimpl::make<GUI_DirectoryWidget::Private>();

	connect(m->dsh, &DirectorySelectionHandler::sig_libraries_changed, this, &GUI_DirectoryWidget::check_libraries);
	connect(m->dsh, &DirectorySelectionHandler::sig_import_dialog_requested, this, &GUI_DirectoryWidget::import_dialog_requested);
	connect(m->dsh, &DirectorySelectionHandler::sig_file_operation_started, this, &GUI_DirectoryWidget::file_operation_started);
	connect(m->dsh, &DirectorySelectionHandler::sig_file_operation_finished, this, &GUI_DirectoryWidget::file_operation_finished);

	m->selected_widget = Private::SelectedWidget::None;

	{ // set current library
		init_library_combobox();
		connect(ui->combo_library, combo_current_index_changed_int, this, &GUI_DirectoryWidget::current_library_changed);
	}

	connect(ui->btn_search, &QPushButton::clicked, this, &GUI_DirectoryWidget::search_button_clicked);
	connect(ui->le_search, &QLineEdit::returnPressed, this, &GUI_DirectoryWidget::search_button_clicked);
	connect(ui->le_search, &QLineEdit::textChanged, this, &GUI_DirectoryWidget::search_text_edited);

	connect(ui->tv_dirs, &QTreeView::pressed, this, &GUI_DirectoryWidget::dir_pressed);
	connect(ui->tv_dirs, &DirectoryTreeView::sig_current_index_changed, this, &GUI_DirectoryWidget::dir_clicked);
	connect(ui->tv_dirs, &DirectoryTreeView::sig_import_requested, this, &GUI_DirectoryWidget::import_requested);
	connect(ui->tv_dirs, &DirectoryTreeView::sig_enter_pressed, this, &GUI_DirectoryWidget::dir_enter_pressed);
	connect(ui->tv_dirs, &DirectoryTreeView::sig_append_clicked, this, &GUI_DirectoryWidget::dir_append_clicked);
	connect(ui->tv_dirs, &DirectoryTreeView::sig_play_clicked, this, &GUI_DirectoryWidget::dir_play_clicked);
	connect(ui->tv_dirs, &DirectoryTreeView::sig_play_next_clicked, this, &GUI_DirectoryWidget::dir_play_next_clicked);
	connect(ui->tv_dirs, &DirectoryTreeView::sig_play_new_tab_clicked, this, &GUI_DirectoryWidget::dir_play_new_tab_clicked);
	connect(ui->tv_dirs, &DirectoryTreeView::sig_delete_clicked, this, &GUI_DirectoryWidget::dir_delete_clicked);
	connect(ui->tv_dirs, &DirectoryTreeView::sig_directory_loaded, this, &GUI_DirectoryWidget::dir_opened);
	connect(ui->tv_dirs, &DirectoryTreeView::sig_copy_requested, this, &GUI_DirectoryWidget::dir_copy_requested);
	connect(ui->tv_dirs, &DirectoryTreeView::sig_move_requested, this, &GUI_DirectoryWidget::dir_move_requested);
	connect(ui->tv_dirs, &DirectoryTreeView::sig_rename_requested, this, &GUI_DirectoryWidget::dir_rename_requested);
	connect(ui->tv_dirs, &DirectoryTreeView::sig_copy_to_library_requested, this, &GUI_DirectoryWidget::dir_copy_to_lib_requested);
	connect(ui->tv_dirs, &DirectoryTreeView::sig_move_to_library_requested, this, &GUI_DirectoryWidget::dir_move_to_lib_requested);

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
	connect(ui->lv_files, &FileListView::sig_rename_requested, this, &GUI_DirectoryWidget::file_rename_requested);
	connect(ui->lv_files, &FileListView::sig_rename_by_expression_requested, this, &GUI_DirectoryWidget::file_rename_by_expression_requested);
	connect(ui->lv_files, &FileListView::sig_copy_to_library_requested, this, &GUI_DirectoryWidget::file_copy_to_lib_requested);
	connect(ui->lv_files, &FileListView::sig_move_to_library_requested, this, &GUI_DirectoryWidget::file_move_to_lib_requested);

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

	connect(ui->splitter_dirs, &QSplitter::splitterMoved, this, &GUI_DirectoryWidget::splitter_moved);
	connect(ui->splitter_tracks, &QSplitter::splitterMoved, this, &GUI_DirectoryWidget::splitter_moved);
	connect(ui->btn_set_library_path, &QPushButton::clicked, this, &GUI_DirectoryWidget::set_library_path_clicked);

	auto* search_context_menu = new QMenu(ui->le_search);
	auto* action = new Gui::SearchPreferenceAction(ui->le_search);
	search_context_menu->addActions({action});

	auto* cmf = new Gui::ContextMenuFilter(ui->le_search);
	connect(cmf, &Gui::ContextMenuFilter::sig_context_menu, search_context_menu, &QMenu::popup);
	ui->le_search->installEventFilter(cmf);

	ui->tv_dirs->set_library(m->dsh->library_info());
	ui->tb_title->init(m->dsh->library_instance());

	init_shortcuts();
	check_libraries();
}

GUI_DirectoryWidget::~GUI_DirectoryWidget()
{
	if(ui) {
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
	return MetaDataList();
}

bool GUI_DirectoryWidget::has_metadata() const
{
	return false;
}

QStringList GUI_DirectoryWidget::pathlist() const
{
	switch(m->selected_widget)
	{
		case Private::SelectedWidget::Dirs:
			return ui->tv_dirs->selected_paths();
		case Private::SelectedWidget::Files:
			return ui->lv_files->selected_paths();
		default:
			return QStringList();
	}
}


void GUI_DirectoryWidget::init_shortcuts()
{
	new QShortcut(QKeySequence::Find, ui->le_search, SLOT(setFocus()), nullptr, Qt::WindowShortcut);
	new QShortcut(QKeySequence("Esc"), ui->le_search, SLOT(clear()), nullptr, Qt::WidgetShortcut);
}

void GUI_DirectoryWidget::init_library_combobox()
{
	LibraryId lib_id = m->dsh->library_id();
	ui->combo_library->clear();

	QList<Library::Info> libraries = Library::Manager::instance()->all_libraries();
	for(const Library::Info& info : libraries)
	{
		ui->combo_library->addItem(info.name(), QVariant::fromValue(info.id()));
	}

	int index = Util::Algorithm::indexOf(libraries, [lib_id](const Library::Info& info){
		return (lib_id == info.id());
	});

	if(Util::between(index, ui->combo_library->count()))
	{
		ui->combo_library->setCurrentIndex(index);
		current_library_changed(index);
	}
}

void GUI_DirectoryWidget::dir_enter_pressed()
{
	const QModelIndexList indexes = ui->tv_dirs->selected_indexes();
	if(!indexes.isEmpty()){
		ui->tv_dirs->expand(indexes.first());
	}
}

void GUI_DirectoryWidget::dir_pressed(QModelIndex idx)
{
	Q_UNUSED(idx)

	const Qt::MouseButtons buttons = QApplication::mouseButtons();
	if(buttons & Qt::MiddleButton)
	{
		m->dsh->prepare_tracks_for_playlist(ui->tv_dirs->selected_paths(), true);
	}
}

void GUI_DirectoryWidget::dir_clicked(QModelIndex idx)
{
	m->is_search_active = false;
	ui->lv_files->clearSelection();

	dir_opened(idx);
}

void GUI_DirectoryWidget::dir_opened(QModelIndex idx)
{
	QString dir = ui->tv_dirs->directory_name(idx);
	if(!idx.isValid()){
		dir = m->dsh->library_info().path();
	}

	QStringList dirs = ui->tv_dirs->selected_paths();
	if(dirs.isEmpty()){
		dirs << dir;
	}

	ui->lv_files->set_parent_directory(m->dsh->library_id(), dir);
	ui->lv_files->set_search_filter(ui->le_search->text());

	// show in metadata table view
	m->dsh->library_instance()->fetch_tracks_by_paths(dirs);
}

void GUI_DirectoryWidget::dir_append_clicked()
{
	m->dsh->append_tracks(ui->tv_dirs->selected_paths());
}

void GUI_DirectoryWidget::dir_play_clicked()
{
	m->dsh->prepare_tracks_for_playlist(ui->tv_dirs->selected_paths(), false);
}

void GUI_DirectoryWidget::dir_play_next_clicked()
{
	m->dsh->play_next(ui->tv_dirs->selected_paths());
}

void GUI_DirectoryWidget::dir_play_new_tab_clicked()
{
	m->dsh->create_playlist(ui->tv_dirs->selected_paths(), true);
}

void GUI_DirectoryWidget::dir_delete_clicked()
{
	Message::Answer answer = Message::question_yn(Lang::get(Lang::Delete) + ": " + Lang::get(Lang::Really) + "?");
	if(answer == Message::Answer::Yes){
		m->dsh->delete_paths(ui->tv_dirs->selected_paths());
	}
}

void GUI_DirectoryWidget::dir_copy_requested(const QStringList& files, const QString& target)
{
	m->dsh->copy_paths(files, target);
}

void GUI_DirectoryWidget::dir_move_requested(const QStringList& files, const QString& target)
{
	m->dsh->move_paths(files, target);
}

void GUI_DirectoryWidget::dir_rename_requested(const QString& old_name, const QString& new_name)
{
	m->dsh->rename_path(old_name, new_name);
}

static QString copy_or_move_to_library_requested(const QStringList& paths, LibraryId id, QWidget* parent)
{
	if(paths.isEmpty()) {
		return QString();
	}

	Library::Info info = Library::Manager::instance()->library_info(id);

	const QString dir = QFileDialog::getExistingDirectory(parent, parent->tr("Choose target directory"), info.path());
	if(dir.isEmpty()) {
		return QString();
	}

	if(!Util::File::is_subdir(dir, info.path()))
	{
		Message::error(parent->tr("%1 is not a subdirectory of %2").arg(dir).arg(info.path()));
		return QString();
	}

	return dir;
}

void GUI_DirectoryWidget::dir_copy_to_lib_requested(LibraryId library_id)
{
	QString target_dir = copy_or_move_to_library_requested(ui->tv_dirs->selected_paths(), library_id, this);
	if(!target_dir.isEmpty())
	{
		m->dsh->copy_paths(ui->tv_dirs->selected_paths(), target_dir);
	}
}

void GUI_DirectoryWidget::dir_move_to_lib_requested(LibraryId library_id)
{
	QString target_dir = copy_or_move_to_library_requested(ui->tv_dirs->selected_paths(), library_id, this);
	if(!target_dir.isEmpty())
	{
		m->dsh->move_paths(ui->tv_dirs->selected_paths(), target_dir);
	}
}

void GUI_DirectoryWidget::file_append_clicked()
{
	m->dsh->append_tracks(ui->lv_files->selected_paths());
}

void GUI_DirectoryWidget::file_play_clicked()
{
	m->dsh->prepare_tracks_for_playlist(ui->lv_files->selected_paths(), false);
}

void GUI_DirectoryWidget::file_play_next_clicked()
{
	m->dsh->play_next(ui->lv_files->selected_paths());
}

void GUI_DirectoryWidget::file_play_new_tab_clicked()
{
	m->dsh->create_playlist(ui->lv_files->selected_paths(), true);
}

void GUI_DirectoryWidget::file_delete_clicked()
{
	Message::Answer answer = Message::question_yn(Lang::get(Lang::Delete) + ": " + Lang::get(Lang::Really) + "?");
	if(answer == Message::Answer::Yes){
		m->dsh->delete_paths(ui->lv_files->selected_paths());
	}
}

void GUI_DirectoryWidget::file_rename_requested(const QString& old_name, const QString& new_name)
{
	m->dsh->rename_path(old_name, new_name);
}

void GUI_DirectoryWidget::file_rename_by_expression_requested(const QString& old_name, const QString& expression)
{
	m->dsh->rename_by_expression(old_name, expression);
	file_operation_finished();
}

void GUI_DirectoryWidget::file_copy_to_lib_requested(LibraryId library_id)
{
	QString target_dir = copy_or_move_to_library_requested(ui->lv_files->selected_paths(), library_id, this);
	if(!target_dir.isEmpty())
	{
		m->dsh->copy_paths(ui->lv_files->selected_paths(), target_dir);
	}
}

void GUI_DirectoryWidget::file_move_to_lib_requested(LibraryId library_id)
{
	QString target_dir = copy_or_move_to_library_requested(ui->lv_files->selected_paths(), library_id, this);
	if(!target_dir.isEmpty())
	{
		m->dsh->move_paths(ui->lv_files->selected_paths(), target_dir);
	}
}

void GUI_DirectoryWidget::file_operation_started()
{
	ui->tv_dirs->set_busy(true);
}

void GUI_DirectoryWidget::file_operation_finished()
{
	ui->tv_dirs->set_busy(false);
	ui->lv_files->set_parent_directory(m->dsh->library_id(), ui->lv_files->parent_directory());
}

void GUI_DirectoryWidget::import_requested(LibraryId id, const QStringList& paths, const QString& target_dir)
{
	m->dsh->import_requested(id, paths, target_dir);
}

void GUI_DirectoryWidget::import_dialog_requested(const QString& target_dir)
{
	if(!this->isVisible()){
		return;
	}

	LocalLibrary* library = m->dsh->library_instance();
	auto* importer = new GUI_ImportDialog(library, true, this);
	connect(importer, &GUI_ImportDialog::sig_closed, importer, &GUI_ImportDialog::deleteLater);

	importer->set_target_dir(target_dir);
	importer->show();
}

void GUI_DirectoryWidget::file_pressed(QModelIndex idx)
{
	Q_UNUSED(idx)

	Qt::MouseButtons buttons = QApplication::mouseButtons();
	if(buttons & Qt::MiddleButton)
	{
		m->dsh->prepare_tracks_for_playlist(ui->lv_files->selected_paths(), true);
	}

	m->dsh->library_instance()->fetch_tracks_by_paths(ui->lv_files->selected_paths());
}

void GUI_DirectoryWidget::file_dbl_clicked(QModelIndex idx)
{
	Q_UNUSED(idx)
	file_enter_pressed();
}

void GUI_DirectoryWidget::file_enter_pressed()
{
	m->dsh->prepare_tracks_for_playlist(ui->lv_files->selected_paths(), false);
}

void GUI_DirectoryWidget::search_button_clicked()
{
	if(ui->le_search->text().isEmpty()){
		m->is_search_active	= false;
		return;
	}

	m->dsh->set_search_text(ui->le_search->text());

	QModelIndex found_idx = ui->tv_dirs->search(ui->le_search->text());
	if(found_idx.isValid())
	{
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

	SetSetting(Set::Dir_SplitterDirFile, ui->splitter_dirs->saveState());
	SetSetting(Set::Dir_SplitterTracks, ui->splitter_tracks->saveState());
}

void GUI_DirectoryWidget::set_library_path_clicked()
{
	auto* new_library_dialog = new GUI_EditLibrary(this);
	connect(new_library_dialog, &GUI_EditLibrary::sig_accepted, this, [this, new_library_dialog]()
	{
		m->dsh->create_new_library(new_library_dialog->name(), new_library_dialog->path());
	});

	new_library_dialog->reset();
	new_library_dialog->show();
}

void GUI_DirectoryWidget::check_libraries()
{
	auto* lib_manager = Library::Manager::instance();
	if(lib_manager->count() == 0)
	{
		ui->stackedWidget->setCurrentIndex(1);
		ui->widget_search->setVisible(false);
	}

	else
	{
		ui->stackedWidget->setCurrentIndex(0);
		ui->widget_search->setVisible(true);
	}

	init_library_combobox();
}

void GUI_DirectoryWidget::current_library_changed(int index)
{
	Q_UNUSED(index)

	LibraryId lib_id = ui->combo_library->currentData().value<LibraryId>();
	m->dsh->set_library_id(lib_id);

	Library::Info info = m->dsh->library_info();

	ui->tv_dirs->set_library(info);
	ui->tb_title->init(m->dsh->library_instance());
	ui->lv_files->set_parent_directory(info.id(), info.path());
}
