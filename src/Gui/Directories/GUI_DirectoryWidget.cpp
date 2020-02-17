/* GUI_DirectoryWidget.cpp */

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

#include "GUI_DirectoryWidget.h"
#include "FileListModel.h"
#include "DirectoryModel.h"

#include "Gui/Directories/ui_GUI_DirectoryWidget.h"
#include "Gui/Library/TrackModel.h"
#include "Gui/ImportDialog/GUI_ImportDialog.h"

#include "Gui/Utils/Library/GUI_EditLibrary.h"
#include "Gui/Utils/ContextMenu/LibraryContextMenu.h"
#include "Gui/Utils/InputDialog/LineInputDialog.h"
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
#include <QLabel>
#include <QDesktopServices>

struct GUI_DirectoryWidget::Private
{
	DirectorySelectionHandler* dsh=nullptr;
	QAction* actionNewDirectory=nullptr;
	QAction* actionViewInFileManager=nullptr;

	enum SelectedWidget
	{
		None=0,
		Dirs,
		Files
	} selectedWidget;

	bool					isSearchActive;

	Private() :
		selectedWidget(None),
		isSearchActive(false)
	{
		dsh = new DirectorySelectionHandler();

		actionNewDirectory = new QAction();
		actionViewInFileManager = new QAction();
	}


	Library::Info currentLibrary() const
	{
		return dsh->libraryInfo();
	}
};


GUI_DirectoryWidget::GUI_DirectoryWidget(QWidget* parent) :
	Widget(parent),
	InfoDialogContainer()
{
	ui = new Ui::GUI_DirectoryWidget();
	ui->setupUi(this);

	ui->splitter_dirs->restoreState(GetSetting(Set::Dir_SplitterDirFile));
	ui->splitter_tracks->restoreState(GetSetting(Set::Dir_SplitterTracks));

	m = Pimpl::make<GUI_DirectoryWidget::Private>();

	connect(m->dsh, &DirectorySelectionHandler::sigLibrariesChanged, this, &GUI_DirectoryWidget::checkLibraries);
	connect(m->dsh, &DirectorySelectionHandler::sigImportDialogRequested, this, &GUI_DirectoryWidget::importDialogRequested);
	connect(m->dsh, &DirectorySelectionHandler::sigFileOperationStarted, this, &GUI_DirectoryWidget::fileOperationStarted);
	connect(m->dsh, &DirectorySelectionHandler::sigFileOperationFinished, this, &GUI_DirectoryWidget::fileOperationFinished);

	m->selectedWidget = Private::SelectedWidget::None;

	{ // set current library
		initLibraryCombobox();
		connect(ui->combo_library, combo_current_index_changed_int, this, &GUI_DirectoryWidget::currentLibraryChanged);
	}

	connect(ui->btn_search, &QPushButton::clicked, this, &GUI_DirectoryWidget::searchButtonClicked);
	connect(ui->le_search, &QLineEdit::returnPressed, this, &GUI_DirectoryWidget::searchButtonClicked);
	connect(ui->le_search, &QLineEdit::textChanged, this, &GUI_DirectoryWidget::searchTextEdited);

	connect(ui->tv_dirs, &QTreeView::pressed, this, &GUI_DirectoryWidget::dirPressed);
	connect(ui->tv_dirs, &DirectoryTreeView::sigCurrentIndexChanged, this, &GUI_DirectoryWidget::dirClicked);
	connect(ui->tv_dirs, &DirectoryTreeView::sigImportRequested, this, &GUI_DirectoryWidget::importRequested);
	connect(ui->tv_dirs, &DirectoryTreeView::sigEnterPressed, this, &GUI_DirectoryWidget::dirEnterPressed);
	connect(ui->tv_dirs, &DirectoryTreeView::sigAppendClicked, this, &GUI_DirectoryWidget::dirAppendClicked);
	connect(ui->tv_dirs, &DirectoryTreeView::sigPlayClicked, this, &GUI_DirectoryWidget::dirPlayClicked);
	connect(ui->tv_dirs, &DirectoryTreeView::sigPlayNextClicked, this, &GUI_DirectoryWidget::dirPlayNextClicked);
	connect(ui->tv_dirs, &DirectoryTreeView::sigPlayNewTabClicked, this, &GUI_DirectoryWidget::dirPlayInNewTabClicked);
	connect(ui->tv_dirs, &DirectoryTreeView::sigDeleteClicked, this, &GUI_DirectoryWidget::dirDeleteClicked);
	connect(ui->tv_dirs, &DirectoryTreeView::sigDirectoryLoaded, this, &GUI_DirectoryWidget::dirOpened);
	connect(ui->tv_dirs, &DirectoryTreeView::sigCopyRequested, this, &GUI_DirectoryWidget::dirCopyRequested);
	connect(ui->tv_dirs, &DirectoryTreeView::sigMoveRequested, this, &GUI_DirectoryWidget::dirMoveRequested);
	connect(ui->tv_dirs, &DirectoryTreeView::sigRenameRequested, this, &GUI_DirectoryWidget::dirRenameRequested);
	connect(ui->tv_dirs, &DirectoryTreeView::sigCopyToLibraryRequested, this, &GUI_DirectoryWidget::dirCopyToLibRequested);
	connect(ui->tv_dirs, &DirectoryTreeView::sigMoveToLibraryRequested, this, &GUI_DirectoryWidget::dirMoveToLibRequested);

	connect(ui->tv_dirs, &DirectoryTreeView::sigInfoClicked, this, [=]()
	{
		m->selectedWidget = Private::SelectedWidget::Dirs;
		showInfo();
	});

	connect(ui->tv_dirs, &DirectoryTreeView::sigEditClicked, this, [=]()
	{
		m->selectedWidget = Private::SelectedWidget::Dirs;
		showEdit();
	});

	connect(ui->tv_dirs, &DirectoryTreeView::sigLyricsClicked, this, [=]()
	{
		m->selectedWidget = Private::SelectedWidget::Dirs;
		showLyrics();
	});

	connect(ui->lv_files, &QListView::pressed, this, &GUI_DirectoryWidget::filePressed);
	connect(ui->lv_files, &QListView::doubleClicked, this, &GUI_DirectoryWidget::fileDoubleClicked);
	connect(ui->lv_files, &FileListView::sigImportRequested, this, &GUI_DirectoryWidget::importRequested);
	connect(ui->lv_files, &FileListView::sigEnterPressed, this, &GUI_DirectoryWidget::fileEnterPressed);
	connect(ui->lv_files, &FileListView::sigAppendClicked, this, &GUI_DirectoryWidget::fileAppendClicked);
	connect(ui->lv_files, &FileListView::sigPlayClicked, this, &GUI_DirectoryWidget::filePlayClicked);
	connect(ui->lv_files, &FileListView::sigPlayNextClicked, this, &GUI_DirectoryWidget::filePlayNextClicked);
	connect(ui->lv_files, &FileListView::sigPlayNewTabClicked, this, &GUI_DirectoryWidget::filePlayNewTabClicked);
	connect(ui->lv_files, &FileListView::sigDeleteClicked, this, &GUI_DirectoryWidget::fileDeleteClicked);
	connect(ui->lv_files, &FileListView::sigRenameRequested, this, &GUI_DirectoryWidget::fileRenameRequested);
	connect(ui->lv_files, &FileListView::sigRenameByExpressionRequested, this, &GUI_DirectoryWidget::fileRenameByExpressionRequested);
	connect(ui->lv_files, &FileListView::sigCopyToLibraryRequested, this, &GUI_DirectoryWidget::fileCopyToLibraryRequested);
	connect(ui->lv_files, &FileListView::sigMoveToLibraryRequested, this, &GUI_DirectoryWidget::fileMoveToLibraryRequested);

	connect(ui->lv_files, &FileListView::sigInfoClicked, this, [=]()
	{
		m->selectedWidget = Private::SelectedWidget::Files;
		showInfo();
	});

	connect(ui->lv_files, &FileListView::sigEditClicked, this, [=]()
	{
		m->selectedWidget = Private::SelectedWidget::Files;
		showEdit();
	});

	connect(ui->lv_files, &FileListView::sigLyricsClicked, this, [=]()
	{
		m->selectedWidget = Private::SelectedWidget::Files;
		showLyrics();
	});

	connect(ui->splitter_dirs, &QSplitter::splitterMoved, this, &GUI_DirectoryWidget::splitterMoved);
	connect(ui->splitter_tracks, &QSplitter::splitterMoved, this, &GUI_DirectoryWidget::splitterMoved);
	connect(ui->btn_set_library_path, &QPushButton::clicked, this, &GUI_DirectoryWidget::setLibraryPathClicked);

	auto* search_context_menu = new QMenu(ui->le_search);
	auto* action = new Gui::SearchPreferenceAction(ui->le_search);
	search_context_menu->addActions({action});

	auto* cmf = new Gui::ContextMenuFilter(ui->le_search);
	connect(cmf, &Gui::ContextMenuFilter::sigContextMenu, search_context_menu, &QMenu::popup);
	ui->le_search->installEventFilter(cmf);

	Library::Info currentLibrary = m->currentLibrary();
	ui->tv_dirs->setLibraryInfo(currentLibrary);
	ui->tb_title->init(m->dsh->libraryInstance());

	initMenuButton();
	initShortcuts();
	checkLibraries();
}

GUI_DirectoryWidget::~GUI_DirectoryWidget()
{
	if(ui) {
		delete ui; ui = nullptr;
	}
}

QFrame* GUI_DirectoryWidget::headerFrame() const
{
	return ui->header_frame;
}

MD::Interpretation GUI_DirectoryWidget::metadataInterpretation() const
{
	return MD::Interpretation::Tracks;
}

MetaDataList GUI_DirectoryWidget::infoDialogData() const
{
	return MetaDataList();
}

bool GUI_DirectoryWidget::hasMetadata() const
{
	return false;
}

QStringList GUI_DirectoryWidget::pathlist() const
{
	switch(m->selectedWidget)
	{
		case Private::SelectedWidget::Dirs:
			return ui->tv_dirs->selectedPaths();
		case Private::SelectedWidget::Files:
			return ui->lv_files->selectedPaths();
		default:
			return QStringList();
	}
}


void GUI_DirectoryWidget::initShortcuts()
{
	new QShortcut(QKeySequence::Find, ui->le_search, SLOT(setFocus()), nullptr, Qt::WindowShortcut);
	new QShortcut(QKeySequence("Esc"), ui->le_search, SLOT(clear()), nullptr, Qt::WidgetShortcut);
}

void GUI_DirectoryWidget::initLibraryCombobox()
{
	LibraryId lib_id = m->dsh->libraryId();
	ui->combo_library->clear();

	const QList<Library::Info> libraries = Library::Manager::instance()->allLibraries();
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
		currentLibraryChanged(index);
	}
}

void GUI_DirectoryWidget::initMenuButton()
{
	ui->btn_menu->registerAction(m->actionNewDirectory);
	ui->btn_menu->registerAction(m->actionViewInFileManager);

	connect(m->actionNewDirectory, &QAction::triggered, this, &GUI_DirectoryWidget::newDirectoryClicked);
	connect(m->actionViewInFileManager, &QAction::triggered, this, &GUI_DirectoryWidget::viewInFileManagerClicked);
}

void GUI_DirectoryWidget::newDirectoryClicked()
{
	QString text = Gui::LineInputDialog::getNewFilename(this, Lang::get(Lang::CreateDirectory));
	if(text.isEmpty()) {
		return;
	}

	Library::Info info = m->currentLibrary();

	QString newPath = info.path() + "/" + text;
	bool success = Util::File::createDir(newPath);
	if(!success)
	{
		QString message = tr("Could not create directory") + "<br>" + newPath;
		Message::error(message);
	}
}

void GUI_DirectoryWidget::viewInFileManagerClicked()
{
	Library::Info info = m->currentLibrary();

	QString url = QString("file://%1").arg(info.path());
	QDesktopServices::openUrl(url);
}

void GUI_DirectoryWidget::dirEnterPressed()
{
	const QModelIndexList indexes = ui->tv_dirs->selctedRows();
	if(!indexes.isEmpty()){
		ui->tv_dirs->expand(indexes.first());
	}
}

void GUI_DirectoryWidget::dirPressed(QModelIndex idx)
{
	Q_UNUSED(idx)

	const Qt::MouseButtons buttons = QApplication::mouseButtons();
	if(buttons & Qt::MiddleButton)
	{
		m->dsh->prepareTracksForPlaylist(ui->tv_dirs->selectedPaths(), true);
	}
}

void GUI_DirectoryWidget::dirClicked(QModelIndex idx)
{
	m->isSearchActive = false;
	ui->lv_files->clearSelection();

	dirOpened(idx);
}

void GUI_DirectoryWidget::dirOpened(QModelIndex idx)
{
	QString dir = ui->tv_dirs->directoryName(idx);
	if(!idx.isValid()){
		dir = m->currentLibrary().path();
	}

	QStringList dirs = ui->tv_dirs->selectedPaths();
	if(dirs.isEmpty()){
		dirs << dir;
	}

	ui->lv_files->setParentDirectory(m->dsh->libraryId(), dir);
	ui->lv_files->setSearchFilter(ui->le_search->text());

	// show in metadata table view
	m->dsh->libraryInstance()->fetchTracksByPath(dirs);
}

void GUI_DirectoryWidget::dirAppendClicked()
{
	m->dsh->appendTracks(ui->tv_dirs->selectedPaths());
}

void GUI_DirectoryWidget::dirPlayClicked()
{
	m->dsh->prepareTracksForPlaylist(ui->tv_dirs->selectedPaths(), false);
}

void GUI_DirectoryWidget::dirPlayNextClicked()
{
	m->dsh->playNext(ui->tv_dirs->selectedPaths());
}

void GUI_DirectoryWidget::dirPlayInNewTabClicked()
{
	m->dsh->createPlaylist(ui->tv_dirs->selectedPaths(), true);
}

void GUI_DirectoryWidget::dirDeleteClicked()
{
	Message::Answer answer = Message::question_yn(Lang::get(Lang::Delete) + ": " + Lang::get(Lang::Really) + "?");
	if(answer == Message::Answer::Yes){
		m->dsh->deletePaths(ui->tv_dirs->selectedPaths());
	}
}

void GUI_DirectoryWidget::dirCopyRequested(const QStringList& files, const QString& target)
{
	m->dsh->copyPaths(files, target);
}

void GUI_DirectoryWidget::dirMoveRequested(const QStringList& files, const QString& target)
{
	m->dsh->movePaths(files, target);
}

void GUI_DirectoryWidget::dirRenameRequested(const QString& old_name, const QString& new_name)
{
	m->dsh->renamePath(old_name, new_name);
}

static QString copyOrMoveLibraryRequested(const QStringList& paths, LibraryId id, QWidget* parent)
{
	namespace File = Util::File;

	if(paths.isEmpty()) {
		return QString();
	}

	Library::Info info = Library::Manager::instance()->libraryInfo(id);

	const QString targetDirectory = QFileDialog::getExistingDirectory(parent, parent->tr("Choose target directory"), info.path());
	if(targetDirectory.isEmpty()) {
		return QString();
	}

	if(!File::isSubdir(targetDirectory, info.path()) && !File::isSamePath(targetDirectory, info.path()))
	{
		Message::error(parent->tr("%1 is not a subdirectory of %2").arg(targetDirectory).arg(info.path()));
		return QString();
	}

	return targetDirectory;
}

void GUI_DirectoryWidget::dirCopyToLibRequested(LibraryId libraryId)
{
	const QString targetDirectory = copyOrMoveLibraryRequested(ui->tv_dirs->selectedPaths(), libraryId, this);
	if(!targetDirectory.isEmpty())
	{
		m->dsh->copyPaths(ui->tv_dirs->selectedPaths(), targetDirectory);
	}
}

void GUI_DirectoryWidget::dirMoveToLibRequested(LibraryId libraryId)
{
	const QString targetDirectory = copyOrMoveLibraryRequested(ui->tv_dirs->selectedPaths(), libraryId, this);
	if(!targetDirectory.isEmpty())
	{
		m->dsh->movePaths(ui->tv_dirs->selectedPaths(), targetDirectory);
	}
}

void GUI_DirectoryWidget::fileAppendClicked()
{
	m->dsh->appendTracks(ui->lv_files->selectedPaths());
}

void GUI_DirectoryWidget::filePlayClicked()
{
	m->dsh->prepareTracksForPlaylist(ui->lv_files->selectedPaths(), false);
}

void GUI_DirectoryWidget::filePlayNextClicked()
{
	m->dsh->playNext(ui->lv_files->selectedPaths());
}

void GUI_DirectoryWidget::filePlayNewTabClicked()
{
	m->dsh->createPlaylist(ui->lv_files->selectedPaths(), true);
}

void GUI_DirectoryWidget::fileDeleteClicked()
{
	Message::Answer answer = Message::question_yn(Lang::get(Lang::Delete) + ": " + Lang::get(Lang::Really) + "?");
	if(answer == Message::Answer::Yes){
		m->dsh->deletePaths(ui->lv_files->selectedPaths());
	}
}

void GUI_DirectoryWidget::fileRenameRequested(const QString& old_name, const QString& new_name)
{
	m->dsh->renamePath(old_name, new_name);
}

void GUI_DirectoryWidget::fileRenameByExpressionRequested(const QString& old_name, const QString& expression)
{
	m->dsh->renameByExpression(old_name, expression);
	fileOperationFinished();
}

void GUI_DirectoryWidget::fileCopyToLibraryRequested(LibraryId libraryId)
{
	QString targetDirectory = copyOrMoveLibraryRequested(ui->lv_files->selectedPaths(), libraryId, this);
	if(!targetDirectory.isEmpty())
	{
		m->dsh->copyPaths(ui->lv_files->selectedPaths(), targetDirectory);
	}
}

void GUI_DirectoryWidget::fileMoveToLibraryRequested(LibraryId libraryId)
{
	QString targetDirectory = copyOrMoveLibraryRequested(ui->lv_files->selectedPaths(), libraryId, this);
	if(!targetDirectory.isEmpty())
	{
		m->dsh->movePaths(ui->lv_files->selectedPaths(), targetDirectory);
	}
}

void GUI_DirectoryWidget::fileOperationStarted()
{
	ui->tv_dirs->setBusy(true);
}

void GUI_DirectoryWidget::fileOperationFinished()
{
	ui->tv_dirs->setBusy(false);
	ui->lv_files->setParentDirectory(m->dsh->libraryId(), ui->lv_files->parentDirectory());
}

void GUI_DirectoryWidget::importRequested(LibraryId id, const QStringList& paths, const QString& targetDirectory)
{
	m->dsh->requestImport(id, paths, targetDirectory);
}

void GUI_DirectoryWidget::importDialogRequested(const QString& targetDirectory)
{
	if(!this->isVisible()){
		return;
	}

	LocalLibrary* library = m->dsh->libraryInstance();
	auto* importer = new GUI_ImportDialog(library, true, this);
	connect(importer, &GUI_ImportDialog::sigClosed, importer, &GUI_ImportDialog::deleteLater);

	importer->setTargetDirectory(targetDirectory);
	importer->show();
}

void GUI_DirectoryWidget::filePressed(QModelIndex idx)
{
	Q_UNUSED(idx)

	Qt::MouseButtons buttons = QApplication::mouseButtons();
	if(buttons & Qt::MiddleButton)
	{
		m->dsh->prepareTracksForPlaylist(ui->lv_files->selectedPaths(), true);
	}

	m->dsh->libraryInstance()->fetchTracksByPath(ui->lv_files->selectedPaths());
}

void GUI_DirectoryWidget::fileDoubleClicked(QModelIndex idx)
{
	Q_UNUSED(idx)
	fileEnterPressed();
}


static void show_image_label(const QString& filename)
{
	QString f = Util::File::getFilenameOfPath(filename);
	QPixmap pm = QPixmap(filename);

	auto* label = new QLabel(nullptr);

	label->setPixmap(pm);
	label->setScaledContents(true);
	label->setAttribute(Qt::WA_DeleteOnClose);
	label->resize((600 * pm.width()) / pm.height(), 600);
	label->setToolTip(QString("%1x%2").arg(pm.width()).arg(pm.height()));
	label->setWindowTitle(QString("%1: %2x%3")
		.arg(f)
		.arg(pm.width())
		.arg(pm.height())
	);

	label->show();
}


void GUI_DirectoryWidget::fileEnterPressed()
{
	QStringList paths = ui->lv_files->selectedPaths();
	if(paths.size() == 1 && Util::File::isImageFile(paths[0]))
	{
		show_image_label(paths[0]);
		return;
	}

	bool has_soundfiles = Util::Algorithm::contains(paths, [](auto path){
		return (Util::File::isSoundFile(path) || Util::File::isPlaylistFile(path));
	});

	if(has_soundfiles)
	{
		m->dsh->prepareTracksForPlaylist(paths, false);
	}
}

void GUI_DirectoryWidget::searchButtonClicked()
{
	if(ui->le_search->text().isEmpty()){
		m->isSearchActive	= false;
		return;
	}

	m->dsh->setSearchText(ui->le_search->text());

	QModelIndex found_idx = ui->tv_dirs->search(ui->le_search->text());
	if(found_idx.isValid())
	{
		dirOpened(found_idx);
		ui->btn_search->setText(Lang::get(Lang::SearchNext));
		m->isSearchActive	= true;
	}
}

void GUI_DirectoryWidget::searchTextEdited(const QString& text)
{
	Q_UNUSED(text)
	m->isSearchActive = false;
	ui->btn_search->setText(Lang::get(Lang::SearchVerb));
}


void GUI_DirectoryWidget::splitterMoved(int pos, int index)
{
	Q_UNUSED(pos)
	Q_UNUSED(index)

	SetSetting(Set::Dir_SplitterDirFile, ui->splitter_dirs->saveState());
	SetSetting(Set::Dir_SplitterTracks, ui->splitter_tracks->saveState());
}

void GUI_DirectoryWidget::setLibraryPathClicked()
{
	auto* new_library_dialog = new GUI_EditLibrary(this);
	connect(new_library_dialog, &GUI_EditLibrary::sigAccepted, this, [this, new_library_dialog]()
	{
		m->dsh->createNewLibrary(new_library_dialog->name(), new_library_dialog->path());
	});

	new_library_dialog->reset();
	new_library_dialog->show();
}

void GUI_DirectoryWidget::checkLibraries()
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

	initLibraryCombobox();
}

void GUI_DirectoryWidget::currentLibraryChanged(int index)
{
	Q_UNUSED(index)

	LibraryId libraryId = ui->combo_library->currentData().value<LibraryId>();
	m->dsh->setLibraryId(libraryId);

	Library::Info info = m->currentLibrary();

	ui->tv_dirs->setLibraryInfo(info);
	ui->tb_title->init(m->dsh->libraryInstance());
	ui->lv_files->setParentDirectory(info.id(), info.path());
}


void GUI_DirectoryWidget::languageChanged()
{
	ui->retranslateUi(this);

	if(m->isSearchActive) {
		ui->btn_search->setText(Lang::get(Lang::SearchNext));
	}

	else {
		ui->btn_search->setText(Lang::get(Lang::SearchVerb));
	}

	ui->btn_set_library_path->setText(Lang::get(Lang::CreateNewLibrary));

	m->actionNewDirectory->setText(Lang::get(Lang::CreateDirectory) + "...");
	m->actionViewInFileManager->setText(tr("Show in file manager"));
}

void GUI_DirectoryWidget::skinChanged()
{
	using namespace Gui;
	ui->btn_search->setIcon(Icons::icon(Icons::Search));

	m->actionNewDirectory->setIcon(Gui::Icons::icon(Gui::Icons::New));
	m->actionViewInFileManager->setIcon(Gui::Icons::icon(Gui::Icons::Folder));
}
