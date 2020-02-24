#include "GUI_DirectoryView.h"
#include "Gui/Directories/ui_GUI_DirectoryView.h"
#include "Gui/Directories/DirectoryTreeView.h"
#include "Gui/Directories/FileListView.h"

#include "Gui/ImportDialog/GUI_ImportDialog.h"

#include "Components/LibraryManagement/LibraryManager.h"
#include "Components/Library/LocalLibrary.h"
#include "Components/Directories/DirectorySelectionHandler.h"

#include "Gui/Utils/Icons.h"
#include "Gui/Utils/InputDialog/LineInputDialog.h"

#include "Utils/Algorithm.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Language/Language.h"
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

static QString copyOrMoveLibraryRequested(const QStringList& paths, LibraryId id, QWidget* parent);
static void showImageLabel(const QString& filename);

struct GUI_DirectoryView::Private
{
	DirectorySelectionHandler* dsh=nullptr;
	QString filterTerm;

	Private()
	{
		dsh = new DirectorySelectionHandler();
	}

	Library::Info currentLibrary() const
	{
		return dsh->libraryInfo();
	}
};

GUI_DirectoryView::GUI_DirectoryView(QWidget* parent) :
	Gui::Widget(parent)
{
	m = Pimpl::make<Private>();
	ui = nullptr;
}

GUI_DirectoryView::~GUI_DirectoryView() = default;

void GUI_DirectoryView::initUi()
{
	if(ui) {
		return;
	}

	ui = new Ui::GUI_DirectoryView();
	ui->setupUi(this);

	connect(m->dsh, &DirectorySelectionHandler::sigImportDialogRequested, this, &GUI_DirectoryView::importDialogRequested);
	connect(m->dsh, &DirectorySelectionHandler::sigFileOperationStarted, this, &GUI_DirectoryView::fileOperationStarted);
	connect(m->dsh, &DirectorySelectionHandler::sigFileOperationFinished, this, &GUI_DirectoryView::fileOperationFinished);

	connect(ui->tv_dirs, &QTreeView::pressed, this, &GUI_DirectoryView::dirPressed);
	connect(ui->tv_dirs, &TreeView::sigCurrentIndexChanged, this, &GUI_DirectoryView::dirClicked);
	connect(ui->tv_dirs, &TreeView::sigImportRequested, this, &GUI_DirectoryView::importRequested);
	connect(ui->tv_dirs, &TreeView::sigEnterPressed, this, &GUI_DirectoryView::dirEnterPressed);
	connect(ui->tv_dirs, &TreeView::sigAppendClicked, this, &GUI_DirectoryView::dirAppendClicked);
	connect(ui->tv_dirs, &TreeView::sigPlayClicked, this, &GUI_DirectoryView::dirPlayClicked);
	connect(ui->tv_dirs, &TreeView::sigPlayNextClicked, this, &GUI_DirectoryView::dirPlayNextClicked);
	connect(ui->tv_dirs, &TreeView::sigPlayNewTabClicked, this, &GUI_DirectoryView::dirPlayInNewTabClicked);
	connect(ui->tv_dirs, &TreeView::sigDeleteClicked, this, &GUI_DirectoryView::dirDeleteClicked);
	connect(ui->tv_dirs, &TreeView::sigDirectoryLoaded, this, &GUI_DirectoryView::dirOpened);
	connect(ui->tv_dirs, &TreeView::sigCopyRequested, this, &GUI_DirectoryView::dirCopyRequested);
	connect(ui->tv_dirs, &TreeView::sigMoveRequested, this, &GUI_DirectoryView::dirMoveRequested);
	connect(ui->tv_dirs, &TreeView::sigRenameRequested, this, &GUI_DirectoryView::dirRenameRequested);
	connect(ui->tv_dirs, &TreeView::sigCopyToLibraryRequested, this, &GUI_DirectoryView::dirCopyToLibRequested);
	connect(ui->tv_dirs, &TreeView::sigMoveToLibraryRequested, this, &GUI_DirectoryView::dirMoveToLibRequested);
	connect(ui->tv_dirs->selectionModel(), &QItemSelectionModel::selectionChanged, this, [this](auto selected, auto descelected)
	{
		Q_UNUSED(descelected)
		ui->btn_clearSelection->setVisible(selected.size() > 0);
	});

	connect(ui->lv_files, &QListView::pressed, this, &GUI_DirectoryView::filePressed);
	connect(ui->lv_files, &QListView::doubleClicked, this, &GUI_DirectoryView::fileDoubleClicked);
	connect(ui->lv_files, &FileListView::sigImportRequested, this, &GUI_DirectoryView::importRequested);
	connect(ui->lv_files, &FileListView::sigEnterPressed, this, &GUI_DirectoryView::fileEnterPressed);
	connect(ui->lv_files, &FileListView::sigAppendClicked, this, &GUI_DirectoryView::fileAppendClicked);
	connect(ui->lv_files, &FileListView::sigPlayClicked, this, &GUI_DirectoryView::filePlayClicked);
	connect(ui->lv_files, &FileListView::sigPlayNextClicked, this, &GUI_DirectoryView::filePlayNextClicked);
	connect(ui->lv_files, &FileListView::sigPlayNewTabClicked, this, &GUI_DirectoryView::filePlayNewTabClicked);
	connect(ui->lv_files, &FileListView::sigDeleteClicked, this, &GUI_DirectoryView::fileDeleteClicked);
	connect(ui->lv_files, &FileListView::sigRenameRequested, this, &GUI_DirectoryView::fileRenameRequested);
	connect(ui->lv_files, &FileListView::sigRenameByExpressionRequested, this, &GUI_DirectoryView::fileRenameByExpressionRequested);
	connect(ui->lv_files, &FileListView::sigCopyToLibraryRequested, this, &GUI_DirectoryView::fileCopyToLibraryRequested);
	connect(ui->lv_files, &FileListView::sigMoveToLibraryRequested, this, &GUI_DirectoryView::fileMoveToLibraryRequested);

	connect(ui->splitter, &QSplitter::splitterMoved, this, &GUI_DirectoryView::splitterMoved);
	connect(ui->btn_createDir, &QPushButton::clicked, this, &GUI_DirectoryView::createDirectoryClicked);
	connect(ui->btn_clearSelection, &QPushButton::clicked, ui->tv_dirs, &TreeView::clearSelection);

	ui->tv_dirs->setEnabled(false);
	ui->tv_dirs->setBusy(true);

	QTimer::singleShot(200, this, &GUI_DirectoryView::load);
}

void GUI_DirectoryView::load()
{
	Library::Info info = m->currentLibrary();
	ui->tv_dirs->setLibraryInfo(info);
	ui->tv_dirs->setFilterTerm(m->filterTerm);
	ui->lv_files->setParentDirectory(info.id(), info.path());
	ui->btn_clearSelection->setVisible(false);

	ui->tv_dirs->setEnabled(true);
	ui->tv_dirs->setBusy(false);
}

void GUI_DirectoryView::setCurrentLibrary(LibraryId libraryId)
{
	m->dsh->setLibraryId(libraryId);

	Library::Info info = m->currentLibrary();

	if(ui)
	{
		ui->tv_dirs->setLibraryInfo(info);
		ui->lv_files->setParentDirectory(info.id(), info.path());
	}
}

void GUI_DirectoryView::setFilterTerm(const QString& filter)
{
	m->filterTerm = filter;

	if(ui) {
		ui->tv_dirs->setFilterTerm(filter);
	}
}

void GUI_DirectoryView::importRequested(LibraryId id, const QStringList& paths, const QString& targetDirectory)
{
	m->dsh->requestImport(id, paths, targetDirectory);
}

void GUI_DirectoryView::importDialogRequested(const QString& targetDirectory)
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

void GUI_DirectoryView::newDirectoryClicked()
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

void GUI_DirectoryView::viewInFileManagerClicked()
{
	Library::Info info = m->currentLibrary();

	QString url = QString("file://%1").arg(info.path());
	QDesktopServices::openUrl(url);
}

void GUI_DirectoryView::dirEnterPressed()
{
	const QModelIndexList indexes = ui->tv_dirs->selctedRows();
	if(!indexes.isEmpty()){
		ui->tv_dirs->expand(indexes.first());
	}
}

void GUI_DirectoryView::dirOpened(QModelIndex idx)
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

	// show in metadata table view
	m->dsh->libraryInstance()->fetchTracksByPath(dirs);
}

void GUI_DirectoryView::dirPressed(QModelIndex idx)
{
	Q_UNUSED(idx)

	const Qt::MouseButtons buttons = QApplication::mouseButtons();
	if(buttons & Qt::MiddleButton)
	{
		m->dsh->prepareTracksForPlaylist(ui->tv_dirs->selectedPaths(), true);
	}
}

void GUI_DirectoryView::dirClicked(QModelIndex idx)
{
	ui->lv_files->clearSelection();

	dirOpened(idx);
}

void GUI_DirectoryView::dirAppendClicked()
{
	m->dsh->appendTracks(ui->tv_dirs->selectedPaths());
}

void GUI_DirectoryView::dirPlayClicked()
{
	m->dsh->prepareTracksForPlaylist(ui->tv_dirs->selectedPaths(), false);
}

void GUI_DirectoryView::dirPlayNextClicked()
{
	m->dsh->playNext(ui->tv_dirs->selectedPaths());
}

void GUI_DirectoryView::dirPlayInNewTabClicked()
{
	m->dsh->createPlaylist(ui->tv_dirs->selectedPaths(), true);
}

void GUI_DirectoryView::dirDeleteClicked()
{
	Message::Answer answer = Message::question_yn(Lang::get(Lang::Delete) + ": " + Lang::get(Lang::Really) + "?");
	if(answer == Message::Answer::Yes){
		m->dsh->deletePaths(ui->tv_dirs->selectedPaths());
	}
}

void GUI_DirectoryView::dirCopyRequested(const QStringList& files, const QString& target)
{
	m->dsh->copyPaths(files, target);
}

void GUI_DirectoryView::dirMoveRequested(const QStringList& files, const QString& target)
{
	m->dsh->movePaths(files, target);
}

void GUI_DirectoryView::dirRenameRequested(const QString& oldName, const QString& newName)
{
	m->dsh->renamePath(oldName, newName);
}

void GUI_DirectoryView::dirCopyToLibRequested(LibraryId libraryId)
{
	const QString targetDirectory = copyOrMoveLibraryRequested(ui->tv_dirs->selectedPaths(), libraryId, this);
	if(!targetDirectory.isEmpty())
	{
		m->dsh->copyPaths(ui->tv_dirs->selectedPaths(), targetDirectory);
	}
}

void GUI_DirectoryView::dirMoveToLibRequested(LibraryId libraryId)
{
	const QString targetDirectory = copyOrMoveLibraryRequested(ui->tv_dirs->selectedPaths(), libraryId, this);
	if(!targetDirectory.isEmpty())
	{
		m->dsh->movePaths(ui->tv_dirs->selectedPaths(), targetDirectory);
	}
}

void GUI_DirectoryView::filePressed(QModelIndex idx)
{
	Q_UNUSED(idx)

	Qt::MouseButtons buttons = QApplication::mouseButtons();
	if(buttons & Qt::MiddleButton)
	{
		m->dsh->prepareTracksForPlaylist(ui->lv_files->selectedPaths(), true);
	}

	QStringList selectedPaths = ui->lv_files->selectedPaths();
	auto lastIt = std::remove_if(selectedPaths.begin(), selectedPaths.end(), [](const QString& path){
		return( !Util::File::isSoundFile(path) && !Util::File::isPlaylistFile(path) );
	});

	selectedPaths.erase(lastIt, selectedPaths.end());

	if(!selectedPaths.isEmpty())
	{ // may happen if an invalid path is clicked
		m->dsh->libraryInstance()->fetchTracksByPath(selectedPaths);
	}
}

void GUI_DirectoryView::fileDoubleClicked(QModelIndex idx)
{
	Q_UNUSED(idx)
	fileEnterPressed();
}

void GUI_DirectoryView::fileEnterPressed()
{
	QStringList paths = ui->lv_files->selectedPaths();
	if(paths.size() == 1 && Util::File::isImageFile(paths[0]))
	{
		showImageLabel(paths[0]);
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

void GUI_DirectoryView::fileAppendClicked()
{
	m->dsh->appendTracks(ui->lv_files->selectedPaths());
}

void GUI_DirectoryView::filePlayClicked()
{
	m->dsh->prepareTracksForPlaylist(ui->lv_files->selectedPaths(), false);
}

void GUI_DirectoryView::filePlayNextClicked()
{
	m->dsh->playNext(ui->lv_files->selectedPaths());
}

void GUI_DirectoryView::filePlayNewTabClicked()
{
	m->dsh->createPlaylist(ui->lv_files->selectedPaths(), true);
}

void GUI_DirectoryView::fileDeleteClicked()
{
	Message::Answer answer = Message::question_yn(Lang::get(Lang::Delete) + ": " + Lang::get(Lang::Really) + "?");
	if(answer == Message::Answer::Yes){
		m->dsh->deletePaths(ui->lv_files->selectedPaths());
	}
}

void GUI_DirectoryView::fileRenameRequested(const QString& oldName, const QString& newName)
{
	m->dsh->renamePath(oldName, newName);
}

void GUI_DirectoryView::fileRenameByExpressionRequested(const QString& oldName, const QString& expression)
{
	m->dsh->renameByExpression(oldName, expression);
	fileOperationFinished();
}

void GUI_DirectoryView::fileCopyToLibraryRequested(LibraryId libraryId)
{
	QString targetDirectory = copyOrMoveLibraryRequested(ui->lv_files->selectedPaths(), libraryId, this);
	if(!targetDirectory.isEmpty())
	{
		m->dsh->copyPaths(ui->lv_files->selectedPaths(), targetDirectory);
	}
}

void GUI_DirectoryView::fileMoveToLibraryRequested(LibraryId libraryId)
{
	QString targetDirectory = copyOrMoveLibraryRequested(ui->lv_files->selectedPaths(), libraryId, this);
	if(!targetDirectory.isEmpty())
	{
		m->dsh->movePaths(ui->lv_files->selectedPaths(), targetDirectory);
	}
}

void GUI_DirectoryView::fileOperationStarted()
{
	ui->tv_dirs->setBusy(true);
}

void GUI_DirectoryView::fileOperationFinished()
{
	ui->tv_dirs->setBusy(false);
	ui->lv_files->setParentDirectory(m->dsh->libraryId(), ui->lv_files->parentDirectory());
}

void GUI_DirectoryView::splitterMoved(int pos, int index)
{
	Q_UNUSED(pos)
	Q_UNUSED(index)

	SetSetting(Set::Dir_SplitterDirFile, ui->splitter->saveState());
}

void GUI_DirectoryView::createDirectoryClicked()
{
	const QString libraryPath = m->dsh->libraryInfo().path();
	QString text = Gui::LineInputDialog::getNewFilename(this, Lang::get(Lang::CreateDirectory), libraryPath);

	if(!text.isEmpty())
	{
		Util::File::createDir(m->dsh->libraryInfo().path() + "/" + text);
	}
}

void GUI_DirectoryView::languageChanged()
{
	if(ui)
	{
		ui->retranslateUi(this);
		ui->btn_createDir->setText(Lang::get(Lang::CreateDirectory));
		ui->btn_clearSelection->setText(Lang::get(Lang::ClearSelection));
	}
}

void GUI_DirectoryView::skinChanged()
{
	if(ui)
	{
		ui->btn_createDir->setIcon(Gui::Icons::icon(Gui::Icons::Folder));
		ui->btn_clearSelection->setIcon(Gui::Icons::icon(Gui::Icons::Clear));
	}
}

QString copyOrMoveLibraryRequested(const QStringList& paths, LibraryId id, QWidget* parent)
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


void showImageLabel(const QString& filename)
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

void GUI_DirectoryView::showEvent(QShowEvent* event)
{
	initUi();
	Gui::Widget::showEvent(event);
}
