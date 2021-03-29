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

#include "Interfaces/LibraryInfoAccessor.h"

#include "Gui/Utils/Delegates/StyledItemDelegate.h"
#include "Gui/Utils/PreferenceAction.h"
#include "Gui/Utils/MimeData/CustomMimeData.h"
#include "Gui/Utils/MimeData/MimeDataUtils.h"
#include "Gui/Utils/InputDialog/LineInputDialog.h"
#include "Gui/Utils/Icons.h"
#include "Gui/Utils/Widgets/ProgressBar.h"

#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Library/LibraryInfo.h"
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

using Directory::TreeView;

struct TreeView::Private
{
	LibraryInfoAccessor* libraryInfoAccessor;
	Directory::Model* model;
	Directory::ContextMenu* contextMenu = nullptr;
	Gui::ProgressBar* progressBar = nullptr;

	QTimer* dragTimer;
	QModelIndex dragTargetIndex;

	Private(LibraryInfoAccessor* libraryInfoAccessor, TreeView* parent) :
		libraryInfoAccessor {libraryInfoAccessor},
		model {new Directory::Model(libraryInfoAccessor, parent)},
		dragTimer {new QTimer {parent}}
	{
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
	Gui::Dragable(this) {}

TreeView::~TreeView() = default;

void TreeView::init(LibraryInfoAccessor* libraryInfoAccessor, const Library::Info& info)
{
	m = Pimpl::make<Private>(libraryInfoAccessor, this);

	setModel(m->model);
	connect(m->model, &Model::sigBusy, this, &TreeView::setBusy);
	connect(m->dragTimer, &QTimer::timeout, this, &TreeView::dragTimerTimeout);

	this->setItemDelegate(new Gui::StyledItemDelegate(this));
	this->setDragDropMode(QAbstractItemView::DragDrop);

	auto* action = new QAction(this);
	action->setShortcut(QKeySequence("F2"));
	action->setShortcutContext(Qt::WidgetShortcut);
	connect(action, &QAction::triggered, this, &TreeView::renameDirectoryClicked);
	this->addAction(action);

	const auto index = m->model->setDataSource(info.id());
	if(index.isValid())
	{
		this->setRootIndex(index);
	}
}

void TreeView::initContextMenu()
{
	if(m->contextMenu)
	{
		return;
	}

	m->contextMenu = new ContextMenu(ContextMenu::Mode::Dir, m->libraryInfoAccessor, this);

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

	connect(m->contextMenu, &ContextMenu::sigInfoClicked, this, [this]() { this->showInfo(); });
	connect(m->contextMenu, &ContextMenu::sigEditClicked, this, [this]() { this->showEdit(); });
}

QString TreeView::directoryName(const QModelIndex& index)
{
	return m->model->filePath(index);
}

QModelIndexList TreeView::selectedRows() const
{
	return selectionModel()->selectedRows();
}

QStringList TreeView::selectedPaths() const
{
	QStringList paths;

	Util::Algorithm::transform(selectedRows(), paths, [&](const auto& index) {
		return m->model->filePath(index);
	});

	return paths;
}

void TreeView::createDirectoryClicked()
{
	const auto paths = selectedPaths();
	if(paths.size() != 1)
	{
		return;
	}

	const auto newName =
		Gui::LineInputDialog::getNewFilename(this, Lang::get(Lang::CreateDirectory), paths[0]);

	if(!newName.isEmpty())
	{
		Util::File::createDir(paths[0] + "/" + newName);
		this->expand(m->model->indexOfPath(paths[0]));
	}
}

void TreeView::renameDirectoryClicked()
{
	const auto paths = selectedPaths();
	if(paths.size() != 1)
	{
		return;
	}

	const auto originalDir = QDir(paths[0]);
	auto parentDir = originalDir;

	if(parentDir.cdUp())
	{
		const auto newName =
			Gui::LineInputDialog::getRenameFilename(this, originalDir.dirName(), parentDir.absolutePath());

		if(!newName.isEmpty())
		{
			emit sigRenameRequested(originalDir.absolutePath(), parentDir.absoluteFilePath(newName));
		}
	}
}

void TreeView::viewInFileManagerClicked()
{
	const auto paths = this->selectedPaths();
	for(const auto& path : paths)
	{
		const auto url = QUrl::fromLocalFile(path);
		QDesktopServices::openUrl(url);
	}
}

void TreeView::setBusy(bool b)
{
	if(b)
	{
		this->setDragDropMode(DragDropMode::NoDragDrop);

		if(!m->progressBar)
		{
			m->progressBar = new Gui::ProgressBar(this);
		}

		m->progressBar->show();
	}

	else
	{
		this->setDragDropMode(DragDropMode::DragDrop);

		if(m->progressBar)
		{
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

	const auto* mimeData = event->mimeData();
	event->setAccepted(mimeData && mimeData->hasUrls());
}

void TreeView::dragMoveEvent(QDragMoveEvent* event)
{
	Parent::dragMoveEvent(event);

	const auto* mimeData = event->mimeData();
	if(!mimeData)
	{
		event->ignore();
		return;
	}

	const auto index = this->indexAt(event->pos());
	if(index != m->dragTargetIndex)
	{
		m->dragTargetIndex = index;

		if(index.isValid())
		{
			m->dragTimer->start();
		}
	}

	const auto* cmd = Gui::MimeData::customMimedata(mimeData);

	if(mimeData->hasUrls() || (cmd && cmd->hasSource(this)))
	{
		event->acceptProposedAction();
	}

	else
	{
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

	const auto index = this->indexAt(event->pos());
	if(!index.isValid())
	{
		return;
	}

	const auto* mimedata = event->mimeData();
	if(!mimedata)
	{
		return;
	}

	const auto targetDirectory = m->model->filePath(index);
	const auto* cmd = Gui::MimeData::customMimedata(mimedata);
	if(cmd)
	{
		handleSayonaraDrop(cmd, targetDirectory);
	}

	else if(mimedata->hasUrls())
	{
		QStringList files;

		const auto urls = mimedata->urls();
		for(const auto& url : urls)
		{
			const auto localFile = url.toLocalFile();
			if(!localFile.isEmpty())
			{
				files << localFile;
			}
		}

		const auto libraryId = m->model->libraryDataSource();
		if(libraryId >= 0)
		{
			emit sigImportRequested(libraryId, files, targetDirectory);
		}
	}
}

void TreeView::handleSayonaraDrop(const Gui::CustomMimeData* cmd, const QString& targetDirectory)
{
	const auto urls = cmd->urls();
	QStringList sourceFiles, sourceDirectories;

	for(const auto& url : urls)
	{
		const auto localFilename = url.toLocalFile();
		const auto localFile = (!localFilename.isEmpty())
		                       ? localFilename
		                       : url.toString(QUrl::PreferLocalFile);

		if(Util::File::isDir(localFile) && Util::File::canCopyDir(localFile, targetDirectory))
		{
			sourceDirectories << localFile;
		}

		else if(Util::File::isFile(localFile))
		{
			sourceFiles << localFile;
		}
	}

	if(!sourceDirectories.isEmpty())
	{
		const auto dropAction = showDropMenu(QCursor::pos());
		switch(dropAction)
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
		const auto dropAction = showDropMenu(QCursor::pos());
		switch(dropAction)
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
	auto menu = QMenu(this);
	auto* copyAction = new QAction(tr("Copy here"), &menu);
	auto* moveAction = new QAction(tr("Move here"), &menu);
	auto* cancelAction = new QAction(Lang::get(Lang::Cancel), &menu);

	menu.addActions(
		{
			copyAction,
			moveAction,
			menu.addSeparator(),
			cancelAction,
		});

	auto* action = menu.exec(pos);
	if(action == copyAction)
	{
		return TreeView::DropAction::Copy;
	}

	else if(action == moveAction)
	{
		return TreeView::DropAction::Move;
	}

	return TreeView::DropAction::Cancel;
}

void TreeView::selectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
	if(!m->dragTimer->isActive())
	{
		QTreeView::selectionChanged(selected, deselected);

		const auto index = (!selected.indexes().isEmpty())
		                   ? selected.indexes().first()
		                   : QModelIndex();

		emit sigCurrentIndexChanged(index);
	}
}

void TreeView::setFilterTerm(const QString& filter) { m->model->setFilter(filter); }

MD::Interpretation TreeView::metadataInterpretation() const { return MD::Interpretation::Tracks; }

MetaDataList TreeView::infoDialogData() const {	return MetaDataList(); }

bool TreeView::hasMetadata() const { return false; }

QStringList TreeView::pathlist() const
{
    return this->selectedPaths();
}

QWidget *TreeView::getParentWidget()
{
    return this;
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
		default:
			Parent::keyPressEvent(event);
	}
}

void TreeView::contextMenuEvent(QContextMenuEvent* event)
{
	if(!m->contextMenu)
	{
		initContextMenu();
	}

	m->contextMenu->refresh(selectedPaths().size());

	const auto pos = QWidget::mapToGlobal(event->pos());
	m->contextMenu->exec(pos);
}

void TreeView::skinChanged()
{
	const auto height = this->fontMetrics().height();
	this->setIconSize(QSize(height, height));
	this->setIndentation(height);
}
