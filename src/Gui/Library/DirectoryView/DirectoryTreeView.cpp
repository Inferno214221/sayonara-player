/* DirectoryTreeView.cpp */

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

#include "DirectoryTreeView.h"
#include "DirectoryModel.h"
#include "DirectoryContextMenu.h"


#include "Gui/Utils/Delegates/StyledItemDelegate.h"
#include "Gui/Utils/PreferenceAction.h"
#include "Gui/Utils/MimeData/CustomMimeData.h"
#include "Gui/Utils/MimeData/MimeDataUtils.h"
#include "Gui/Utils/InputDialog/LineInputDialog.h"
#include "Gui/Utils/Icons.h"
#include "Gui/Utils/Widgets/ProgressBar.h"

#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Library/LibraryInfo.h"
#include "Utils/Utils.h"
#include "Utils/FileUtils.h"
#include "Utils/Algorithm.h"
#include "Utils/Language/Language.h"
#include "Utils/Logger/Logger.h"

#include <QDir>
#include <QUrl>
#include <QMouseEvent>
#include <QDrag>
#include <QTimer>
#include <QAction>
#include <QDesktopServices>

#include <algorithm>
#include <utility> // std::pair

using Directory::TreeView;

struct TreeView::Private
{
	Directory::ContextMenu* contextMenu=nullptr;
	Directory::Model* model=nullptr;
	Gui::ProgressBar* progressBar=nullptr;

	QTimer*	dragTimer=nullptr;
	QModelIndex	dragTargetIndex;

	QString	lastSearchTerm;

	Private(QObject* parent)
	{
		dragTimer = new QTimer(parent);
		dragTimer->setSingleShot(true);
		dragTimer->setInterval(750);
	}

	void resetDrag()
	{
		dragTimer->stop();
		dragTargetIndex = QModelIndex();
	}
};


TreeView::TreeView(QWidget* parent) :
	Gui::WidgetTemplate<QTreeView>(parent),
	InfoDialogContainer(),
	Gui::Dragable(this)
{
	m = Pimpl::make<Private>(this);

	m->model = new Model(this);
	connect(m->model, &Model::sigBusy, this, &TreeView::setBusy);
	this->setModel(m->model);

	connect(m->dragTimer, &QTimer::timeout, this, &TreeView::dragTimerTimeout);

	this->setItemDelegate(new Gui::StyledItemDelegate(this));
	this->setDragDropMode(QAbstractItemView::DragDrop);

	auto* action = new QAction(this);
	action->setShortcut(QKeySequence("F2"));
	action->setShortcutContext(Qt::WidgetShortcut);
	connect(action, &QAction::triggered, this, &TreeView::renameDirectoryClicked);
	this->addAction(action);
}

TreeView::~TreeView() = default;

void TreeView::initContextMenu()
{
	if(m->contextMenu){
		return;
	}

	m->contextMenu = new ContextMenu(ContextMenu::Mode::Dir, this);

	connect(m->contextMenu, &ContextMenu::sigDeleteClicked, this, &TreeView::sigDeleteClicked);
	connect(m->contextMenu, &ContextMenu::sigPlayClicked, this, &TreeView::sigPlayClicked);
	connect(m->contextMenu, &ContextMenu::sigPlayNewTabClicked, this, &TreeView::sigPlayNewTabClicked);
	connect(m->contextMenu, &ContextMenu::sigPlayNextClicked, this, &TreeView::sigPlayNextClicked);
	connect(m->contextMenu, &ContextMenu::sigAppendClicked, this, &TreeView::sigAppendClicked);
	connect(m->contextMenu, &ContextMenu::sigCreateDirectoryClicked, this, &TreeView::createDirectoryClicked);
	connect(m->contextMenu, &ContextMenu::sigRenameClicked, this, &TreeView::renameDirectoryClicked);
	connect(m->contextMenu, &ContextMenu::sigCollapseAllClicked, this, &TreeView::collapseAll);
	connect(m->contextMenu, &ContextMenu::sigViewInFileManagerClicked, this, &TreeView::viewInFileManagerClicked);
	connect(m->contextMenu, &ContextMenu::sigCopyToLibrary, this, &TreeView::sigCopyToLibraryRequested);
	connect(m->contextMenu, &ContextMenu::sigMoveToLibrary, this, &TreeView::sigMoveToLibraryRequested);

	connect(m->contextMenu, &ContextMenu::sigInfoClicked, this, [this](){ this->showInfo(); });
	connect(m->contextMenu, &ContextMenu::sigEditClicked, this, [this](){ this->showEdit(); });
}

QString TreeView::directoryName(const QModelIndex& index)
{
	return m->model->filePath(index);
}

QModelIndexList TreeView::selctedRows() const
{
	return this->selectionModel()->selectedRows();
}

QStringList TreeView::selectedPaths() const
{
	QStringList paths;

	const QModelIndexList selections = this->selctedRows();
	for(const QModelIndex& idx : selections)
	{
		paths << m->model->filePath(idx);
	}

	return paths;
}

void TreeView::createDirectoryClicked()
{
	const QStringList paths = selectedPaths();
	if(paths.size() != 1){
		return;
	}

	QString newName = Gui::LineInputDialog::getNewFilename(this, Lang::get(Lang::CreateDirectory), paths[0]);
	if(!newName.isEmpty())
	{
		Util::File::createDir(paths[0] + "/" + newName);
		this->expand(m->model->indexOfPath(paths[0]));
	}
}

void TreeView::renameDirectoryClicked()
{
	const QStringList paths = selectedPaths();
	if(paths.size() != 1){
		return;
	}

	QDir originalDir(paths[0]);
	QDir parentDir(originalDir);

	if(parentDir.cdUp())
	{
		QString newName = Gui::LineInputDialog::getRenameFilename(this, originalDir.dirName(), parentDir.absolutePath());
		if(!newName.isEmpty())
		{
			emit sigRenameRequested(originalDir.absolutePath(), parentDir.absoluteFilePath(newName));
		}
	}
}

void TreeView::viewInFileManagerClicked()
{
	const QStringList paths = this->selectedPaths();
	for(const QString& path : paths)
	{
		const QUrl url = QUrl::fromLocalFile(path);
		QDesktopServices::openUrl(url);
	}
}

void TreeView::setBusy(bool b)
{
	if(b)
	{
		this->setDragDropMode(DragDropMode::NoDragDrop);

		if(!m->progressBar) {
			m->progressBar = new Gui::ProgressBar(this);
		}

		m->progressBar->show();
	}

	else
	{
		this->setDragDropMode(DragDropMode::DragDrop);

		if(m->progressBar) {
			m->progressBar->hide();
		}
	}
}

void TreeView::dragTimerTimeout()
{
	if(m->dragTargetIndex.isValid())
	{
		this->expand(m->dragTargetIndex);
		emit sigCurrentIndexChanged(m->dragTargetIndex);
	}

	m->resetDrag();
}

void TreeView::dragEnterEvent(QDragEnterEvent* event)
{
	m->resetDrag();

	const QMimeData* mimeData = event->mimeData();
	if(!mimeData || !mimeData->hasUrls())
	{
		event->ignore();
		return;
	}

	event->accept();
}

void TreeView::dragMoveEvent(QDragMoveEvent* event)
{
	Parent::dragMoveEvent(event);

	const QMimeData* mimeData = event->mimeData();
	if(!mimeData) {
		event->ignore();
		return;
	}

	const QModelIndex index = this->indexAt(event->pos());
	if(index != m->dragTargetIndex)
	{
		m->dragTargetIndex = index;

		if(index.isValid()) {
			m->dragTimer->start();
		}
	}

	const auto* cmd = Gui::MimeData::customMimedata(mimeData);

	if(mimeData->hasUrls()) {
		event->acceptProposedAction();
	}

	else if(cmd && cmd->hasSource(this)) {
		event->acceptProposedAction();
	}

	else {
		event->ignore();
	}

	if(event->isAccepted() && !selectionModel()->isSelected(index))
	{
		selectionModel()->select(index, QItemSelectionModel::ClearAndSelect);
	}	
}

void TreeView::dragLeaveEvent(QDragLeaveEvent* event)
{
	m->resetDrag();
	event->accept();

	Parent::dragLeaveEvent(event);
}

void TreeView::dropEvent(QDropEvent* event)
{
	event->accept();

	m->dragTimer->stop();
	m->dragTargetIndex = QModelIndex();

	QModelIndex index = this->indexAt(event->pos());
	if(!index.isValid()){
		return;
	}

	const QMimeData* mimedata = event->mimeData();
	if(!mimedata){
		return;
	}

	const QString targetDirectory = m->model->filePath(index);
	const auto* cmd = Gui::MimeData::customMimedata(mimedata);
	if(cmd)
	{
		handleSayonaraDrop(cmd, targetDirectory);
	}

	else if(mimedata->hasUrls())
	{
		QStringList files;

		const QList<QUrl> urls = mimedata->urls();
		for(const QUrl& url : urls)
		{
			QString local_file = url.toLocalFile();
			if(!local_file.isEmpty()){
				files << local_file;
			}
		}

		LibraryId id = m->model->libraryDataSource();
		if(id >= 0)
		{
			emit sigImportRequested(id, files, targetDirectory);
		}
	}
}

void TreeView::handleSayonaraDrop(const Gui::CustomMimeData* cmd, const QString& targetDirectory)
{
	QList<QUrl> urls = cmd->urls();
	QStringList sourceFiles, sourceDirectories;

	for(const QUrl& url : urls)
	{
		QString localFile = url.toLocalFile();
		if(localFile.isEmpty()){
			localFile = url.toString(QUrl::PreferLocalFile);
		}

		if(Util::File::isDir(localFile))
		{
			if(Util::File::canCopyDir(localFile, targetDirectory)){
				sourceDirectories << localFile;
			}
		}

		else if(Util::File::isFile(localFile))
		{
			sourceFiles << localFile;
		}
	}

	if(sourceDirectories.isEmpty() && sourceFiles.isEmpty()){
		return;
	}

	TreeView::DropAction drop_action = showDropMenu(QCursor::pos());

	if(!sourceDirectories.isEmpty())
	{
		switch(drop_action)
		{
			case TreeView::DropAction::Copy:
				emit sigCopyRequested(sourceDirectories, targetDirectory);
				break;
			case TreeView::DropAction::Move:
				emit sigMoveRequested(sourceDirectories, targetDirectory);
				break;
			default:
				break;
		}
	}

	if(!sourceFiles.isEmpty())
	{
		switch(drop_action)
		{
			case TreeView::DropAction::Copy:
				emit sigCopyRequested(sourceFiles, targetDirectory);
				break;
			case TreeView::DropAction::Move:
				emit sigMoveRequested(sourceFiles, targetDirectory);
				break;
			default:
				break;
		}
	}
}

TreeView::DropAction TreeView::showDropMenu(const QPoint& pos)
{
	auto* menu = new QMenu(this);

	const QList<QAction*> actions
	{
		new QAction(tr("Copy here"), menu),
		new QAction(tr("Move here"), menu),
		menu->addSeparator(),
		new QAction(Lang::get(Lang::Cancel), menu),
	};

	menu->addActions(actions);

	QAction* action = menu->exec(pos);
	TreeView::DropAction dropAction = TreeView::DropAction::Cancel;

	if(action == actions[0]){
		dropAction = TreeView::DropAction::Copy;
	}

	else if(action == actions[1]){
		dropAction = TreeView::DropAction::Move;
	}

	menu->deleteLater();

	return dropAction;
}

void TreeView::selectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
	QTreeView::selectionChanged(selected, deselected);

	QModelIndex index;
	if(!selected.indexes().isEmpty()) {
		index = selected.indexes().first();
	}

	if(!m->dragTimer->isActive()) {
		emit sigCurrentIndexChanged(index);
	}
}

void TreeView::setLibraryInfo(const Library::Info& info)
{
	const QModelIndex index = m->model->setDataSource(info.id());
	if(index.isValid())
	{
		this->setRootIndex(index);
	}
}

void TreeView::setFilterTerm(const QString& filter)
{
	m->model->setFilter(filter);
}

MD::Interpretation TreeView::metadataInterpretation() const
{
	return MD::Interpretation::Tracks;
}

MetaDataList TreeView::infoDialogData() const
{
	return MetaDataList();
}

bool TreeView::hasMetadata() const
{
	return false;
}

QStringList TreeView::pathlist() const
{
	return this->selectedPaths();
}

void TreeView::keyPressEvent(QKeyEvent* event)
{
	switch(event->key())
	{
		case Qt::Key_Enter:
		case Qt::Key_Return:
			emit sigEnterPressed();
			return;
		case Qt::Key_Escape:
			this->clearSelection();
			return;
		default: break;
	}

	Parent::keyPressEvent(event);
}

void TreeView::contextMenuEvent(QContextMenuEvent* event)
{
	if(!m->contextMenu){
		initContextMenu();
	}

	m->contextMenu->refresh(selectedPaths().size());

	QPoint pos = QWidget::mapToGlobal(event->pos());
	m->contextMenu->exec(pos);
}


void TreeView::skinChanged()
{
	const QFontMetrics fm = this->fontMetrics();
	this->setIconSize(QSize(fm.height(), fm.height()));
	this->setIndentation(fm.height());
}
