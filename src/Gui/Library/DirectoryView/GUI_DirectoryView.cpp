#include "GUI_DirectoryView.h"
#include "DirectoryTreeView.h"
#include "FileListView.h"

#include "Gui/Library/ui_GUI_DirectoryView.h"
#include "Gui/Library/GUI_ImportDialog.h"

#include "Components/LibraryManagement/LibraryManager.h"
#include "Components/Library/LocalLibrary.h"
#include "Components/Directories/DirectorySelectionHandler.h"

#include "Gui/Utils/Icons.h"
#include "Gui/Utils/InputDialog/LineInputDialog.h"

#include "Utils/Algorithm.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Language/Language.h"
#include "Utils/Utils.h"
#include "Utils/FileUtils.h"
#include "Utils/Message/Message.h"
#include "Utils/Library/LibraryInfo.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Logger/Logger.h"

#include <QAction>
#include <QDesktopServices>
#include <QItemSelectionModel>
#include <QFileDialog>
#include <QLabel>
#include <QTimer>

using Directory::TreeView;
using Directory::FileListView;

namespace
{
	QString copyOrMoveLibraryRequested(const Library::Info& libraryInfo, const QStringList& paths, QWidget* parent);
	void showImageLabel(const QString& filename);
}

struct GUI_DirectoryView::Private
{
	Library::Manager* libraryManager;
	DirectorySelectionHandler* directorySelectionHandler;
	QString filterTerm;

	Private(Library::Manager* libraryManager) :
		libraryManager {libraryManager},
		directorySelectionHandler {new DirectorySelectionHandler {libraryManager}} {}

	Library::Info currentLibrary() const
	{
		return directorySelectionHandler->libraryInfo();
	}
};

GUI_DirectoryView::GUI_DirectoryView(QWidget* parent) :
	Gui::Widget(parent),
	ui {nullptr}
{}

GUI_DirectoryView::~GUI_DirectoryView() = default;

void GUI_DirectoryView::init(Library::Manager* libraryManager)
{
	m = Pimpl::make<Private>(libraryManager);
}

void GUI_DirectoryView::initUi()
{
	if(ui)
	{
		return;
	}

	ui = new Ui::GUI_DirectoryView();
	ui->setupUi(this);
	ui->tvDirs->init(m->libraryManager);

	connect(m->directorySelectionHandler,
	        &DirectorySelectionHandler::sigImportDialogRequested,
	        this,
	        &GUI_DirectoryView::importDialogRequested);
	connect(m->directorySelectionHandler,
	        &DirectorySelectionHandler::sigFileOperationStarted,
	        this,
	        &GUI_DirectoryView::fileOperationStarted);
	connect(m->directorySelectionHandler,
	        &DirectorySelectionHandler::sigFileOperationFinished,
	        this,
	        &GUI_DirectoryView::fileOperationFinished);
	connect(m->directorySelectionHandler,
	        &DirectorySelectionHandler::sigLibrariesChanged,
	        this,
	        &GUI_DirectoryView::load);

	connect(ui->tvDirs, &QTreeView::pressed, this, &GUI_DirectoryView::dirPressed);
	connect(ui->tvDirs, &TreeView::sigCurrentIndexChanged, this, &GUI_DirectoryView::dirClicked);
	connect(ui->tvDirs, &TreeView::sigImportRequested, this, &GUI_DirectoryView::importRequested);
	connect(ui->tvDirs, &TreeView::sigEnterPressed, this, &GUI_DirectoryView::dirEnterPressed);
	connect(ui->tvDirs, &TreeView::sigAppendClicked, this, &GUI_DirectoryView::dirAppendClicked);
	connect(ui->tvDirs, &TreeView::sigPlayClicked, this, &GUI_DirectoryView::dirPlayClicked);
	connect(ui->tvDirs, &TreeView::sigPlayNextClicked, this, &GUI_DirectoryView::dirPlayNextClicked);
	connect(ui->tvDirs, &TreeView::sigPlayNewTabClicked, this, &GUI_DirectoryView::dirPlayInNewTabClicked);
	connect(ui->tvDirs, &TreeView::sigDeleteClicked, this, &GUI_DirectoryView::dirDeleteClicked);
	connect(ui->tvDirs, &TreeView::sigDirectoryLoaded, this, &GUI_DirectoryView::dirOpened);
	connect(ui->tvDirs, &TreeView::sigCopyRequested, this, &GUI_DirectoryView::dirCopyRequested);
	connect(ui->tvDirs, &TreeView::sigMoveRequested, this, &GUI_DirectoryView::dirMoveRequested);
	connect(ui->tvDirs, &TreeView::sigRenameRequested, this, &GUI_DirectoryView::dirRenameRequested);
	connect(ui->tvDirs, &TreeView::sigCopyToLibraryRequested, this, &GUI_DirectoryView::dirCopyToLibRequested);
	connect(ui->tvDirs, &TreeView::sigMoveToLibraryRequested, this, &GUI_DirectoryView::dirMoveToLibRequested);
	connect(ui->tvDirs->selectionModel(),
	        &QItemSelectionModel::selectionChanged,
	        this,
	        &GUI_DirectoryView::dirSelectionChanged);

	connect(ui->lvFiles, &QListView::pressed, this, &GUI_DirectoryView::filePressed);
	connect(ui->lvFiles, &QListView::doubleClicked, this, &GUI_DirectoryView::fileDoubleClicked);
	connect(ui->lvFiles, &FileListView::sigImportRequested, this, &GUI_DirectoryView::importRequested);
	connect(ui->lvFiles, &FileListView::sigEnterPressed, this, &GUI_DirectoryView::fileEnterPressed);
	connect(ui->lvFiles, &FileListView::sigAppendClicked, this, &GUI_DirectoryView::fileAppendClicked);
	connect(ui->lvFiles, &FileListView::sigPlayClicked, this, &GUI_DirectoryView::filePlayClicked);
	connect(ui->lvFiles, &FileListView::sigPlayNextClicked, this, &GUI_DirectoryView::filePlayNextClicked);
	connect(ui->lvFiles, &FileListView::sigPlayNewTabClicked, this, &GUI_DirectoryView::filePlayNewTabClicked);
	connect(ui->lvFiles, &FileListView::sigDeleteClicked, this, &GUI_DirectoryView::fileDeleteClicked);
	connect(ui->lvFiles, &FileListView::sigRenameRequested, this, &GUI_DirectoryView::fileRenameRequested);
	connect(ui->lvFiles,
	        &FileListView::sigRenameByExpressionRequested,
	        this,
	        &GUI_DirectoryView::fileRenameByExpressionRequested);
	connect(ui->lvFiles,
	        &FileListView::sigCopyToLibraryRequested,
	        this,
	        &GUI_DirectoryView::fileCopyToLibraryRequested);
	connect(ui->lvFiles,
	        &FileListView::sigMoveToLibraryRequested,
	        this,
	        &GUI_DirectoryView::fileMoveToLibraryRequested);
	connect(ui->lvFiles->selectionModel(),
	        &QItemSelectionModel::selectionChanged,
	        this,
	        &GUI_DirectoryView::fileSelectionChanged);

	connect(ui->splitter, &QSplitter::splitterMoved, this, &GUI_DirectoryView::splitterMoved);
	connect(ui->btnCreateDir, &QPushButton::clicked, this, &GUI_DirectoryView::createDirectoryClicked);
	connect(ui->btnClearSelection, &QPushButton::clicked, ui->tvDirs, &TreeView::clearSelection);

	ui->tvDirs->setEnabled(false);
	ui->tvDirs->setBusy(true);

	QTimer::singleShot(500, this, &GUI_DirectoryView::load);
}

void GUI_DirectoryView::load()
{
	const auto info = m->currentLibrary();

	ui->tvDirs->setLibraryInfo(info);
	ui->tvDirs->setFilterTerm(m->filterTerm);
	ui->lvFiles->setParentDirectory(info.id(), info.path());
	ui->btnClearSelection->setVisible(false);

	ui->tvDirs->setEnabled(true);
	ui->tvDirs->setBusy(false);
}

void GUI_DirectoryView::setCurrentLibrary(LibraryId libraryId)
{
	m->directorySelectionHandler->setLibraryId(libraryId);

	const auto info = m->currentLibrary();

	if(ui)
	{
		ui->tvDirs->setLibraryInfo(info);
		ui->lvFiles->setParentDirectory(info.id(), info.path());
	}
}

void GUI_DirectoryView::setFilterTerm(const QString& filter)
{
	m->filterTerm = filter;

	if(ui)
	{
		ui->tvDirs->setFilterTerm(filter);
	}
}

void GUI_DirectoryView::importRequested(LibraryId id, const QStringList& paths, const QString& targetDirectory)
{
	m->directorySelectionHandler->requestImport(id, paths, targetDirectory);
}

void GUI_DirectoryView::importDialogRequested(const QString& targetDirectory)
{
	if(!this->isVisible())
	{
		return;
	}

	auto* library = m->directorySelectionHandler->libraryInstance();
	auto* importer = new GUI_ImportDialog(library, true, this);
	connect(importer, &GUI_ImportDialog::sigClosed, importer, &GUI_ImportDialog::deleteLater);

	importer->setTargetDirectory(targetDirectory);
	importer->show();
}

void GUI_DirectoryView::newDirectoryClicked()
{
	const auto newDirName = Gui::LineInputDialog::getNewFilename(this, Lang::get(Lang::CreateDirectory));
	if(newDirName.isEmpty())
	{
		return;
	}

	const auto info = m->currentLibrary();
	const auto libraryPath = info.path();
	const auto libraryDir = QDir {libraryPath};

	const auto success = libraryDir.mkdir(newDirName);
	if(!success)
	{
		const auto message = QString("%1<br>%2")
			.arg(tr("Could not create directory"))
			.arg(libraryDir.absoluteFilePath(newDirName));

		Message::error(message);
	}
}

void GUI_DirectoryView::viewInFileManagerClicked()
{
	const auto info = m->currentLibrary();
	const auto url = QUrl::fromLocalFile(info.path());

	QDesktopServices::openUrl(url);
}

void GUI_DirectoryView::dirEnterPressed()
{
	const auto indexes = ui->tvDirs->selctedRows();
	if(!indexes.isEmpty())
	{
		ui->tvDirs->expand(indexes.first());
	}
}

void GUI_DirectoryView::dirOpened(QModelIndex idx)
{
	const auto dir = (idx.isValid())
	                 ? ui->tvDirs->directoryName(idx)
	                 : m->currentLibrary().path();

	const auto selectedPaths = ui->tvDirs->selectedPaths();
	const auto dirs = (!selectedPaths.isEmpty())
	                  ? selectedPaths
	                  : QStringList {dir};

	ui->lvFiles->setParentDirectory(m->directorySelectionHandler->libraryId(), dir);

	// show in metadata table view
	m->directorySelectionHandler->libraryInstance()->fetchTracksByPath(dirs);
}

void GUI_DirectoryView::dirPressed(QModelIndex idx)
{
	Q_UNUSED(idx)

	const auto buttons = QApplication::mouseButtons();
	if(buttons & Qt::MiddleButton)
	{
		const auto selectedPaths = ui->tvDirs->selectedPaths();
		m->directorySelectionHandler->prepareTracksForPlaylist(selectedPaths, true);
	}
}

void GUI_DirectoryView::dirSelectionChanged(const QItemSelection& selected, const QItemSelection& /*deselected*/)
{
	ui->btnClearSelection->setVisible(!selected.isEmpty());
}

void GUI_DirectoryView::dirClicked(QModelIndex idx)
{
	ui->lvFiles->clearSelection();

	dirOpened(idx);
}

void GUI_DirectoryView::dirAppendClicked()
{
	const auto selectedPaths = ui->tvDirs->selectedPaths();
	m->directorySelectionHandler->appendTracks(selectedPaths);
}

void GUI_DirectoryView::dirPlayClicked()
{
	const auto selectedPaths = ui->tvDirs->selectedPaths();
	m->directorySelectionHandler->prepareTracksForPlaylist(selectedPaths, false);
}

void GUI_DirectoryView::dirPlayNextClicked()
{
	const auto selectedPaths = ui->tvDirs->selectedPaths();
	m->directorySelectionHandler->playNext(selectedPaths);
}

void GUI_DirectoryView::dirPlayInNewTabClicked()
{
	const auto selectedPaths = ui->tvDirs->selectedPaths();
	m->directorySelectionHandler->createPlaylist(selectedPaths, true);
}

void GUI_DirectoryView::dirDeleteClicked()
{
	const auto answer = Message::question_yn(Lang::get(Lang::Delete) + ": " + Lang::get(Lang::Really) + "?");
	if(answer == Message::Answer::Yes)
	{
		const auto selectedPaths = ui->tvDirs->selectedPaths();
		m->directorySelectionHandler->deletePaths(selectedPaths);
	}
}

void GUI_DirectoryView::dirCopyRequested(const QStringList& files, const QString& target)
{
	m->directorySelectionHandler->copyPaths(files, target);
}

void GUI_DirectoryView::dirMoveRequested(const QStringList& files, const QString& target)
{
	m->directorySelectionHandler->movePaths(files, target);
}

void GUI_DirectoryView::dirRenameRequested(const QString& oldName, const QString& newName)
{
	m->directorySelectionHandler->renamePath(oldName, newName);
}

void GUI_DirectoryView::dirCopyToLibRequested(LibraryId libraryId)
{
	const auto selectedPaths = ui->tvDirs->selectedPaths();
	const auto targetDirectory =
		copyOrMoveLibraryRequested(m->libraryManager->libraryInfo(libraryId),
		                           selectedPaths,
		                           this);
	if(!targetDirectory.isEmpty())
	{
		m->directorySelectionHandler->copyPaths(selectedPaths, targetDirectory);
	}
}

void GUI_DirectoryView::dirMoveToLibRequested(LibraryId libraryId)
{
	const auto selectedPaths = ui->tvDirs->selectedPaths();
	const auto targetDirectory =
		copyOrMoveLibraryRequested(m->libraryManager->libraryInfo(libraryId),
		                           selectedPaths,
		                           this);
	if(!targetDirectory.isEmpty())
	{
		m->directorySelectionHandler->movePaths(selectedPaths, targetDirectory);
	}
}

void GUI_DirectoryView::filePressed(QModelIndex idx)
{
	Q_UNUSED(idx)

	const auto buttons = QApplication::mouseButtons();
	if(buttons & Qt::MiddleButton)
	{
		m->directorySelectionHandler->prepareTracksForPlaylist(ui->lvFiles->selectedPaths(), true);
	}
}

void GUI_DirectoryView::fileSelectionChanged(const QItemSelection& /*selected*/, const QItemSelection& /*deselected*/)
{
	auto selectedPaths = ui->lvFiles->selectedPaths();
	const auto lastIt = std::remove_if(selectedPaths.begin(), selectedPaths.end(), [](const auto& path) {
		return (!Util::File::isSoundFile(path) && !Util::File::isPlaylistFile(path));
	});

	selectedPaths.erase(lastIt, selectedPaths.end());

	if(!selectedPaths.isEmpty())
	{ // may happen if an invalid path is clicked
		m->directorySelectionHandler->libraryInstance()->fetchTracksByPath(selectedPaths);
	}

	else if(!ui->tvDirs->selectedPaths().isEmpty())
	{
		m->directorySelectionHandler->libraryInstance()->fetchTracksByPath(ui->tvDirs->selectedPaths());
	}

	else
	{
		m->directorySelectionHandler->libraryInstance()->refetch();
	}
}

void GUI_DirectoryView::fileDoubleClicked(QModelIndex idx)
{
	Q_UNUSED(idx)
	fileEnterPressed();
}

void GUI_DirectoryView::fileEnterPressed()
{
	const auto paths = ui->lvFiles->selectedPaths();
	if(paths.size() == 1 && Util::File::isImageFile(paths[0]))
	{
		showImageLabel(paths[0]);
		return;
	}

	const auto hasSoundfiles = Util::Algorithm::contains(paths, [](auto path) {
		return (Util::File::isSoundFile(path) || Util::File::isPlaylistFile(path));
	});

	if(hasSoundfiles)
	{
		m->directorySelectionHandler->prepareTracksForPlaylist(paths, false);
	}
}

void GUI_DirectoryView::fileAppendClicked()
{
	m->directorySelectionHandler->appendTracks(ui->lvFiles->selectedPaths());
}

void GUI_DirectoryView::filePlayClicked()
{
	m->directorySelectionHandler->prepareTracksForPlaylist(ui->lvFiles->selectedPaths(), false);
}

void GUI_DirectoryView::filePlayNextClicked()
{
	m->directorySelectionHandler->playNext(ui->lvFiles->selectedPaths());
}

void GUI_DirectoryView::filePlayNewTabClicked()
{
	m->directorySelectionHandler->createPlaylist(ui->lvFiles->selectedPaths(), true);
}

void GUI_DirectoryView::fileDeleteClicked()
{
	const auto answer = Message::question_yn(Lang::get(Lang::Delete) + ": " + Lang::get(Lang::Really) + "?");
	if(answer == Message::Answer::Yes)
	{
		m->directorySelectionHandler->deletePaths(ui->lvFiles->selectedPaths());
	}
}

void GUI_DirectoryView::fileRenameRequested(const QString& oldName, const QString& newName)
{
	m->directorySelectionHandler->renamePath(oldName, newName);
}

void GUI_DirectoryView::fileRenameByExpressionRequested(const QString& oldName, const QString& expression)
{
	m->directorySelectionHandler->renameByExpression(oldName, expression);
	fileOperationFinished();
}

void GUI_DirectoryView::fileCopyToLibraryRequested(LibraryId libraryId)
{
	const auto targetDirectory =
		copyOrMoveLibraryRequested(m->libraryManager->libraryInfo(libraryId),
		                           ui->lvFiles->selectedPaths(),
		                           this);
	if(!targetDirectory.isEmpty())
	{
		m->directorySelectionHandler->copyPaths(ui->lvFiles->selectedPaths(), targetDirectory);
	}
}

void GUI_DirectoryView::fileMoveToLibraryRequested(LibraryId libraryId)
{
	const auto targetDirectory = copyOrMoveLibraryRequested(m->libraryManager->libraryInfo(libraryId),
	                                                        ui->lvFiles->selectedPaths(),
	                                                        this);
	if(!targetDirectory.isEmpty())
	{
		m->directorySelectionHandler->movePaths(ui->lvFiles->selectedPaths(), targetDirectory);
	}
}

void GUI_DirectoryView::fileOperationStarted()
{
	ui->tvDirs->setBusy(true);
}

void GUI_DirectoryView::fileOperationFinished()
{
	ui->tvDirs->setBusy(false);
	ui->lvFiles->setParentDirectory(m->directorySelectionHandler->libraryId(), ui->lvFiles->parentDirectory());
}

void GUI_DirectoryView::splitterMoved(int pos, int index)
{
	Q_UNUSED(pos)
	Q_UNUSED(index)

	SetSetting(Set::Dir_SplitterDirFile, ui->splitter->saveState());
}

void GUI_DirectoryView::createDirectoryClicked()
{
	const auto libraryPath = m->directorySelectionHandler->libraryInfo().path();
	const auto text =
		Gui::LineInputDialog::getNewFilename(this, Lang::get(Lang::CreateDirectory), libraryPath);

	if(!text.isEmpty())
	{
		Util::File::createDir(m->directorySelectionHandler->libraryInfo().path() + "/" + text);
	}
}

void GUI_DirectoryView::languageChanged()
{
	if(ui)
	{
		ui->retranslateUi(this);
		ui->btnCreateDir->setText(Lang::get(Lang::CreateDirectory));
		ui->btnClearSelection->setText(Lang::get(Lang::ClearSelection));
	}
}

void GUI_DirectoryView::skinChanged()
{
	if(ui)
	{
		ui->btnCreateDir->setIcon(Gui::Icons::icon(Gui::Icons::Folder));
		ui->btnClearSelection->setIcon(Gui::Icons::icon(Gui::Icons::Clear));
	}
}

void GUI_DirectoryView::showEvent(QShowEvent* event)
{
	initUi();
	Gui::Widget::showEvent(event);
}

namespace
{
	QString copyOrMoveLibraryRequested(const Library::Info& info, const QStringList& paths, QWidget* parent)
	{
		namespace File = Util::File;

		if(paths.isEmpty())
		{
			return QString();
		}

		const QString targetDirectory = QFileDialog::getExistingDirectory(parent,
		                                                                  parent->tr("Choose target directory"),
		                                                                  info.path());
		if(targetDirectory.isEmpty())
		{
			return QString();
		}

		if(!File::isSubdir(targetDirectory, info.path()) && !File::isSamePath(targetDirectory, info.path()))
		{
			Message::error(parent->tr("%1 is not a subdirectory of %2").arg(targetDirectory).arg(info.path()));
			return QString();
		}

		return targetDirectory;
	}

	void showImageLabel(const QString& filename)
	{
		const auto f = Util::File::getFilenameOfPath(filename);
		const auto pm = QPixmap(filename);

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
}
