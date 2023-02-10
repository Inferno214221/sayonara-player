/* FileListView.cpp */

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

#include "FileListView.h"
#include "FileListModel.h"
#include "DirectoryContextMenu.h"
#include "GUI_FileExpressionDialog.h"

#include "Components/LibraryManagement/LibraryManager.h"
#include "Gui/Utils/Delegates/StyledItemDelegate.h"
#include "Gui/Utils/GuiUtils.h"
#include "Gui/Utils/Icons.h"
#include "Gui/Utils/InputDialog/LineInputDialog.h"
#include "Gui/Utils/MimeData/CustomMimeData.h"
#include "Gui/Utils/MimeData/MimeDataUtils.h"
#include "Gui/Utils/PreferenceAction.h"
#include "Utils/Algorithm.h"
#include "Utils/FileUtils.h"
#include "Utils/Language/Language.h"
#include "Utils/Library/LibraryInfo.h"
#include "Utils/Logger/Logger.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Set.h"

#include <QApplication>
#include <QDir>
#include <QDrag>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QHeaderView>
#include <QMimeData>
#include <QPainter>
#include <QShortcut>

using Directory::FileListView;

struct FileListView::Private
{
	Library::InfoAccessor* libraryInfoAccessor;
	FileListModel* model;
	ContextMenu* contextMenu = nullptr;

	Private(Library::InfoAccessor* libraryInfoAccessor, LibraryId libraryId, FileListView* parent) :
		libraryInfoAccessor {libraryInfoAccessor},
		model {new FileListModel {libraryInfoAccessor->libraryInstance(libraryId), parent}} {}
};

FileListView::FileListView(QWidget* parent) :
	SearchableTableView(parent),
	Gui::Dragable(this) {}

FileListView::~FileListView() = default;

void FileListView::init(Library::InfoAccessor* libraryInfoAccessor, const Library::Info& info)
{
	m = Pimpl::make<Private>(libraryInfoAccessor, info.id(), this);

	this->setSearchableModel(m->model);
	this->setItemDelegate(new Gui::StyledItemDelegate(this));
	this->setSelectionMode(QAbstractItemView::ExtendedSelection);
	this->setDragDropMode(QAbstractItemView::DragOnly);

	this->horizontalHeader()->resizeSection(0, this->fontMetrics().height());
	this->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
	this->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);

	this->verticalHeader()->setDefaultSectionSize(Gui::Util::viewRowHeight(fontMetrics()));

	{ // rename by pressing F2
		auto* action = new QAction(this);
		connect(action, &QAction::triggered, this, &FileListView::renameFileClicked);
		action->setShortcut(QKeySequence("F2"));
		action->setShortcutContext(Qt::WidgetShortcut);
		this->addAction(action);
	}

	new QShortcut(QKeySequence(Qt::Key_Return), this, SIGNAL(sigEnterPressed()), nullptr, Qt::WidgetShortcut);
	new QShortcut(QKeySequence(Qt::Key_Enter), this, SIGNAL(sigEnterPressed()), nullptr, Qt::WidgetShortcut);
}

void FileListView::initContextMenu()
{
	if(m->contextMenu)
	{
		return;
	}

	m->contextMenu = new ContextMenu(ContextMenu::Mode::File, m->libraryInfoAccessor, this);

	connect(m->contextMenu->action(ContextMenu::EntryInfo), &QAction::triggered, this, [&]() { this->showInfo(); });
	connect(m->contextMenu->action(ContextMenu::EntryLyrics), &QAction::triggered, this, [&]() { this->showLyrics(); });
	connect(m->contextMenu->action(ContextMenu::EntryEdit), &QAction::triggered, this, [&]() { this->showEdit(); });
	connect(m->contextMenu->action(ContextMenu::EntryDelete), &QAction::triggered,
	        this, &FileListView::sigDeleteClicked);
	connect(m->contextMenu->action(ContextMenu::EntryPlay), &QAction::triggered, this, &FileListView::sigPlayClicked);
	connect(m->contextMenu->action(ContextMenu::EntryPlayNewTab), &QAction::triggered,
	        this, &FileListView::sigPlayNewTabClicked);
	connect(m->contextMenu->action(ContextMenu::EntryPlayNext), &QAction::triggered,
	        this, &FileListView::sigPlayNextClicked);
	connect(m->contextMenu->action(ContextMenu::EntryAppend), &QAction::triggered,
	        this, &FileListView::sigAppendClicked);
	connect(m->contextMenu, &ContextMenu::sigRenameClicked, this, &FileListView::renameFileClicked);
	connect(m->contextMenu, &ContextMenu::sigRenameByTagClicked, this, &FileListView::renameFileByTagClicked);
	connect(m->contextMenu, &ContextMenu::sigCopyToLibrary, this, &FileListView::sigCopyToLibraryRequested);
	connect(m->contextMenu, &ContextMenu::sigMoveToLibrary, this, &FileListView::sigMoveToLibraryRequested);
}

QStringList FileListView::selectedPaths() const
{
	const auto paths = m->model->files();
	const auto indexes = selectionModel()->selectedIndexes();

	Util::Set<QString> selectedPaths;
	for(const auto& index: indexes)
	{
		const auto row = index.row();
		if(row >= 0 && row <= paths.count())
		{
			selectedPaths.insert(paths[index.row()]);
		}
	}

	return QStringList {selectedPaths.toList()};
}

void FileListView::setParentDirectory(const QString& dir)
{
	selectionModel()->clear();
	m->model->setParentDirectory(dir);
}

QString FileListView::parentDirectory() const { return m->model->parentDirectory(); }

int FileListView::mapModelIndexToIndex(const QModelIndex& idx) const { return idx.row(); }

ModelIndexRange FileListView::mapIndexToModelIndexes(int idx) const
{
	return {
		m->model->index(idx, 0),
		m->model->index(idx, m->model->columnCount())
	};
}

void FileListView::renameFileClicked()
{
	const auto paths = this->selectedPaths();
	if(paths.size() != 1)
	{
		return;
	}

	const auto& path = paths.first();
	const auto dir = Util::File::getParentDirectory(path);
	const auto extension = Util::File::getFileExtension(path);

	const auto inputText =
		Gui::LineInputDialog::getRenameFilename(this, Lang::get(Lang::EnterNewName));

	if(inputText.isEmpty())
	{
		return;
	}

	const auto newName = inputText.endsWith("." + extension)
	                     ? QDir(dir).filePath(inputText)
	                     : QDir(dir).filePath(inputText) + "." + extension;

	emit sigRenameRequested(path, newName);
}

void FileListView::renameFileByTagClicked()
{
	const auto files = this->selectedPaths();
	if(!files.isEmpty())
	{
		auto* dialog = new GUI_FileExpressionDialog(this);
		const auto ret = QDialog::DialogCode(dialog->exec());
		const auto expression = dialog->expression();

		if(ret == QDialog::Rejected || expression.isEmpty())
		{
			return;
		}

		for(const auto& file: files)
		{
			emit sigRenameByExpressionRequested(file, expression);
		}
	}
}

MD::Interpretation FileListView::metadataInterpretation() const { return MD::Interpretation::Tracks; }

MetaDataList FileListView::infoDialogData() const { return {}; }

bool FileListView::hasMetadata() const { return false; }

QStringList FileListView::pathlist() const { return this->selectedPaths(); }

QWidget* FileListView::getParentWidget() { return this; }

void FileListView::contextMenuEvent(QContextMenuEvent* event)
{
	initContextMenu();

	const auto files = selectedPaths();
	const auto audioFileCount = Util::Algorithm::count(files, [](const auto& filename) {
		return Util::File::isSoundFile(filename);
	});

	m->contextMenu->refresh(audioFileCount);

	const auto pos = QWidget::mapToGlobal(event->pos());
	m->contextMenu->exec(pos);
}

void FileListView::dragEnterEvent(QDragEnterEvent* event)
{
	event->accept();
}

void FileListView::dragMoveEvent(QDragMoveEvent* event)
{
	const auto* mimeData = event->mimeData();
	const auto* cmd = Gui::MimeData::customMimedata(mimeData);

	event->setAccepted(cmd == nullptr);
}

void FileListView::dropEvent(QDropEvent* event)
{
	event->accept();

	const auto* mimeData = event->mimeData();
	if(!mimeData)
	{
		spLog(Log::Debug, this) << "Drop: No Mimedata";
		return;
	}

	if(Gui::MimeData::isPlayerDrag(mimeData))
	{
		spLog(Log::Debug, this) << "Drop: Internal player drag";
		return;
	}

	if(!mimeData->hasUrls())
	{
		spLog(Log::Debug, this) << "Drop: No Urls";
		return;
	}

	const auto urls = mimeData->urls();

	QStringList files;
	for(const auto& url: urls)
	{
		const auto localFile = url.toLocalFile();
		if(!localFile.isEmpty())
		{
			files << localFile;
		}
	}

	emit sigImportRequested(m->model->libraryId(), files, m->model->parentDirectory());
}

void FileListView::skinChanged()
{
	const auto height = this->fontMetrics().height();
	setIconSize(QSize(height, height));
}
