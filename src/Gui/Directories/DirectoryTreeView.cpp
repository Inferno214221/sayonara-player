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
#include "DirectoryIconProvider.h"
#include "DirectoryModel.h"
#include "DirectoryContextMenu.h"

#include "Gui/Utils/Delegates/StyledItemDelegate.h"
#include "Gui/Utils/PreferenceAction.h"
#include "Gui/Utils/CustomMimeData.h"
#include "Gui/Utils/MimeDataUtils.h"
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
#include <QMouseEvent>
#include <QDrag>
#include <QTimer>
#include <QAction>

#include <algorithm>
#include <utility> // std::pair

struct DirectoryTreeView::Private
{
	QString				lastSearchTerm;

	DirectoryContextMenu*	contextMenu=nullptr;
	DirectoryModel*		model=nullptr;
	IconProvider*		iconProvider=nullptr;
	Gui::ProgressBar*	progressBar=nullptr;
	int					lastFoundIndex;

	QTimer*				dragTimer=nullptr;
	QModelIndex			dragTargetIndex;

	Private(QObject* parent) :
		lastFoundIndex(-1)
	{
		iconProvider = new IconProvider();

		dragTimer = new QTimer(parent);
		dragTimer->setSingleShot(true);
		dragTimer->setInterval(750);
	}

	~Private()
	{
		delete iconProvider; iconProvider = nullptr;
	}

	void resetDrag()
	{
		dragTimer->stop();
		dragTargetIndex = QModelIndex();
	}
};


DirectoryTreeView::DirectoryTreeView(QWidget* parent) :
	SearchableTreeView(parent),
	Gui::Dragable(this)
{
	m = Pimpl::make<Private>(this);

	m->model = new DirectoryModel(this);
	m->model->setFilter(QDir::NoDotAndDotDot | QDir::Dirs);
	m->model->setIconProvider(m->iconProvider);
	this->setSearchableModel(m->model);

	connect(m->dragTimer, &QTimer::timeout, this, &DirectoryTreeView::dragTimerTimeout);
	this->setItemDelegate(new Gui::StyledItemDelegate(this));

	auto* action = new QAction(this);
	action->setShortcut(QKeySequence("F2"));
	action->setShortcutContext(Qt::WidgetShortcut);
	connect(action, &QAction::triggered, this, &DirectoryTreeView::renameDirectoryClicked);
	this->addAction(action);
}

DirectoryTreeView::~DirectoryTreeView() = default;


void DirectoryTreeView::initContextMenu()
{
	if(m->contextMenu){
		return;
	}

	m->contextMenu = new DirectoryContextMenu(DirectoryContextMenu::Mode::Dir, this);

	connect(m->contextMenu, &DirectoryContextMenu::sigInfoClicked, this, &DirectoryTreeView::sigInfoClicked);
	connect(m->contextMenu, &DirectoryContextMenu::sigLyricsClicked, this, &DirectoryTreeView::sigInfoClicked);
	connect(m->contextMenu, &DirectoryContextMenu::sigEditClicked, this, &DirectoryTreeView::sigInfoClicked);
	connect(m->contextMenu, &DirectoryContextMenu::sigDeleteClicked, this, &DirectoryTreeView::sigDeleteClicked);
	connect(m->contextMenu, &DirectoryContextMenu::sigPlayClicked, this, &DirectoryTreeView::sigPlayClicked);
	connect(m->contextMenu, &DirectoryContextMenu::sigPlayNewTabClicked, this, &DirectoryTreeView::sigPlayNewTabClicked);
	connect(m->contextMenu, &DirectoryContextMenu::sigPlayNextClicked, this, &DirectoryTreeView::sigPlayNextClicked);
	connect(m->contextMenu, &DirectoryContextMenu::sigAppendClicked, this, &DirectoryTreeView::sigAppendClicked);
	connect(m->contextMenu, &DirectoryContextMenu::sigCreateDirectoryClicked, this, &DirectoryTreeView::createDirectoryClicked);
	connect(m->contextMenu, &DirectoryContextMenu::sigRenameClicked, this, &DirectoryTreeView::renameDirectoryClicked);
	connect(m->contextMenu, &DirectoryContextMenu::sigCollapseAllClicked, this, &DirectoryTreeView::collapseAll);
	connect(m->contextMenu, &DirectoryContextMenu::sigCopyToLibrary, this, &DirectoryTreeView::sigCopyToLibraryRequested);
	connect(m->contextMenu, &DirectoryContextMenu::sigMoveToLibrary, this, &DirectoryTreeView::sigMoveToLibraryRequested);
}

QString DirectoryTreeView::directoryName(const QModelIndex& index)
{
	return m->model->filePath(index);
}

QModelIndexList DirectoryTreeView::selctedRows() const
{
	return this->selectionModel()->selectedRows();
}

QStringList DirectoryTreeView::selectedPaths() const
{
	QStringList paths;

	const QModelIndexList selections = this->selctedRows();
	for(const QModelIndex& idx : selections)
	{
		paths << m->model->filePath(idx);
	}

	return paths;
}

QModelIndex DirectoryTreeView::search(const QString& search_term)
{
	m->model->setSearchOnlyDirectories(false);

	const QModelIndexList found_indexes = m->model->searchResults(search_term);
	if(found_indexes.isEmpty())
	{
		m->lastFoundIndex = 0;
		return QModelIndex();
	}

	if(m->lastSearchTerm == search_term)
	{
		m->lastFoundIndex++;
		if(m->lastFoundIndex >= found_indexes.count()){
			m->lastFoundIndex = 0;
		}
	}

	else {
		m->lastFoundIndex = 0;
		m->lastSearchTerm = search_term;
	}

	QModelIndex found_idx = found_indexes[m->lastFoundIndex];

	scrollTo(found_idx, QAbstractItemView::PositionAtCenter);
	selectionModel()->select(found_idx, QItemSelectionModel::ClearAndSelect);
	expand(found_idx);

	emit sigDirectoryLoaded(found_idx);

	return found_idx;
}

void DirectoryTreeView::selectMatch(const QString& str, SearchDirection direction)
{
	QModelIndex idx = matchIndex(str, direction);
	if(!idx.isValid()){
		return;
	}

	selectionModel()->select(idx, QItemSelectionModel::ClearAndSelect);
	//setCurrentIndex(idx);
	expand(idx);
	scrollTo(idx, QAbstractItemView::PositionAtCenter);
}

void DirectoryTreeView::contextMenuEvent(QContextMenuEvent* event)
{
	if(!m->contextMenu){
		initContextMenu();
	}

	m->contextMenu->refresh(selectedPaths().size());

	QPoint pos = QWidget::mapToGlobal(event->pos());
	m->contextMenu->exec(pos);
}

void DirectoryTreeView::createDirectoryClicked()
{
	QModelIndexList indexes = this->selctedRows();
	if(indexes.size() != 1){
		return;
	}

	QString newName = Gui::LineInputDialog::getNewFilename(this, Lang::get(Lang::CreateDirectory), selectedPaths()[0]);
	if(!newName.isEmpty())
	{
		m->model->mkdir(indexes.first(), newName);
		this->expand(indexes.first());
	}
}

void DirectoryTreeView::renameDirectoryClicked()
{
	QStringList paths = selectedPaths();
	if(paths.size() != 1){
		return;
	}

	const QString dir = paths[0];
	QDir d(dir);
	QString newName = Gui::LineInputDialog::getRenameFilename(this, d.dirName());
	if(!newName.isEmpty())
	{
		if(d.cdUp())
		{
			emit sigRenameRequested(dir, d.filePath(newName));
		}
	}
}

void DirectoryTreeView::setBusy(bool b)
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

void DirectoryTreeView::dragTimerTimeout()
{
	if(m->dragTargetIndex.isValid()) {
		this->expand(m->dragTargetIndex);
	}

	m->resetDrag();
}

bool DirectoryTreeView::hasDragLabel() const
{
	return (!this->selectedPaths().isEmpty());
}

QString DirectoryTreeView::dragLabel() const
{
	QStringList paths;

	Util::Algorithm::transform(selectedPaths(), paths, [](const QString& path){
		return Util::File::getFilenameOfPath(path);
	});

	if(paths.size() > 3){
		return Lang::getWithNumber(Lang::NrDirectories, paths.size());
	}

	return paths.join(", ");
}

void DirectoryTreeView::dragEnterEvent(QDragEnterEvent* event)
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

void DirectoryTreeView::dragMoveEvent(QDragMoveEvent* event)
{
	SearchableTreeView::dragMoveEvent(event);

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

void DirectoryTreeView::dragLeaveEvent(QDragLeaveEvent* event)
{
	m->resetDrag();
	event->accept();

	SearchableTreeView::dragLeaveEvent(event);
}

void DirectoryTreeView::dropEvent(QDropEvent* event)
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

		LibraryId id = m->model->libraryInfo().id();
		emit sigImportRequested(id, files, targetDirectory);
	}
}

void DirectoryTreeView::handleSayonaraDrop(const Gui::CustomMimeData* cmd, const QString& targetDirectory)
{
	QList<QUrl> urls = cmd->urls();
	QStringList sourceFiles, sourceDirectorys;

	for(const QUrl& url : urls)
	{
		const QString source = url.toLocalFile();

		if(Util::File::isDir(source))
		{
			if(Util::File::canCopyDir(source, targetDirectory)){
				sourceDirectorys << source;
			}
		}

		else if(Util::File::isFile(source))
		{
			sourceFiles << url.toLocalFile();
		}
	}

	if(sourceDirectorys.isEmpty() && sourceFiles.isEmpty()){
		return;
	}

	DirectoryTreeView::DropAction drop_action = showDropMenu(QCursor::pos());

	if(!sourceDirectorys.isEmpty())
	{
		switch(drop_action)
		{
			case DirectoryTreeView::DropAction::Copy:
				emit sigCopyRequested(sourceDirectorys, targetDirectory);
				break;
			case DirectoryTreeView::DropAction::Move:
				emit sigMoveRequested(sourceDirectorys, targetDirectory);
				break;
			default:
				break;
		}
	}

	if(!sourceFiles.isEmpty())
	{
		switch(drop_action)
		{
			case DirectoryTreeView::DropAction::Copy:
				emit sigCopyRequested(sourceFiles, targetDirectory);
				break;
			case DirectoryTreeView::DropAction::Move:
				emit sigMoveRequested(sourceFiles, targetDirectory);
				break;
			default:
				break;
		}
	}
}

DirectoryTreeView::DropAction DirectoryTreeView::showDropMenu(const QPoint& pos)
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
	DirectoryTreeView::DropAction dropAction = DirectoryTreeView::DropAction::Cancel;

	if(action == actions[0]){
		dropAction = DirectoryTreeView::DropAction::Copy;
	}

	else if(action == actions[1]){
		dropAction = DirectoryTreeView::DropAction::Move;
	}

	menu->deleteLater();

	return dropAction;
}

void DirectoryTreeView::selectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
	QTreeView::selectionChanged(selected, deselected);

	QModelIndex index;
	if(!selected.indexes().isEmpty()){
		index = selected.indexes().first();
	}

	emit sigCurrentIndexChanged(index);
}


QMimeData* DirectoryTreeView::dragableMimedata() const
{
	const QModelIndexList selected_items = this->selctedRows();

	QList<QUrl> urls;
	for(const QModelIndex& index : selected_items)
	{
		const QString path = m->model->filePath(index);
		urls << QUrl::fromLocalFile(path);
		spLog(Log::Debug, this) << "Dragging " << path;
	}

	auto* cmd = new Gui::CustomMimeData(this);
	cmd->setUrls(urls);

	return cmd;
}

void DirectoryTreeView::setLibraryInfo(const Library::Info& info)
{
	m->model->setLibraryInfo(info);
	for(int i=1; i<m->model->columnCount(); i++) {
		this->hideColumn(i);
	}

	const QModelIndex index = m->model->index(info.path());
	this->setRootIndex(index);
}

int DirectoryTreeView::mapModelIndexToIndex(const QModelIndex& idx) const
{
	Q_UNUSED(idx)
	return -1;
}

ModelIndexRange DirectoryTreeView::mapIndexToModelIndexes(int idx) const
{
	Q_UNUSED(idx)
	return ModelIndexRange(QModelIndex(), QModelIndex());
}


void DirectoryTreeView::skinChanged()
{
	if(m) {
		m->model->setIconProvider(m->iconProvider);
	}

	const QFontMetrics fm = this->fontMetrics();
	this->setIconSize(QSize(fm.height(), fm.height()));
	this->setIndentation(fm.height());
}

void DirectoryTreeView::keyPressEvent(QKeyEvent* event)
{
	event->setAccepted(false);

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

	m->model->setSearchOnlyDirectories(true);

	SearchableTreeView::keyPressEvent(event);
}
