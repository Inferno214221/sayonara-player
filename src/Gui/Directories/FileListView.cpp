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

#include "Utils/globals.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Library/SearchMode.h"
#include "Utils/FileUtils.h"
#include "Utils/Language/Language.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Algorithm.h"

#include "Gui/Utils/Delegates/StyledItemDelegate.h"
#include "Gui/Utils/InputDialog/LineInputDialog.h"
#include "Gui/Utils/PreferenceAction.h"
#include "Gui/Utils/CustomMimeData.h"
#include "Gui/Utils/MimeDataUtils.h"
#include "Gui/Utils/Icons.h"

#include <QDir>
#include <QDrag>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QPainter>
#include <QMimeData>
#include <QApplication>
#include <QShortcut>
#include <QHeaderView>

struct FileListView::Private
{
	DirectoryContextMenu*	contextMenu=nullptr;
	FileListModel*			model=nullptr;

	Private(FileListView* parent)
	{
		model = new FileListModel(parent);
	}
};

FileListView::FileListView(QWidget* parent) :
	SearchableTableView(parent),
	Gui::Dragable(this)
{
	m = Pimpl::make<Private>(this);

	this->setSearchableModel(m->model);
	this->setItemDelegate(new Gui::StyledItemDelegate(this));
	this->setSelectionMode(QAbstractItemView::ExtendedSelection);
	this->setIconSize(QSize(16, 16));

	this->horizontalHeader()->resizeSection(0, this->fontMetrics().height());
	this->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
	this->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);

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

FileListView::~FileListView() = default;

void FileListView::contextMenuEvent(QContextMenuEvent* event)
{
	if(!m->contextMenu){
		initContextMenu();
	}

	const QModelIndexList indexes = selectedRows();
	auto audioFileCount = Util::Algorithm::count(indexes, [](const QModelIndex& index)
	{
		QString filename = index.data(Qt::UserRole).toString();
		return Util::File::isSoundFile(filename);
	});

	m->contextMenu->refresh(audioFileCount);

	QPoint pos = QWidget::mapToGlobal(event->pos());
	m->contextMenu->exec(pos);
}

void FileListView::dragEnterEvent(QDragEnterEvent *event)
{
	event->accept();
}

void FileListView::dragMoveEvent(QDragMoveEvent *event)
{
	const QMimeData* mime_data = event->mimeData();
	const auto* cmd = Gui::MimeData::customMimedata(mime_data);
	if(cmd){
		event->setAccepted(false);
	}

	else{
		event->setAccepted(true);
	}
}

void FileListView::dropEvent(QDropEvent *event)
{
	event->accept();

	const QMimeData* mime_data = event->mimeData();
	if(!mime_data){
		spLog(Log::Debug, this) << "Drop: No Mimedata";
		return;
	}

	if(Gui::MimeData::isPlayerDrag(mime_data)){
		spLog(Log::Debug, this) << "Drop: Internal player drag";
		return;
	}

	if(!mime_data->hasUrls())
	{
		spLog(Log::Debug, this) << "Drop: No Urls";
		return;
	}

	const QList<QUrl> urls = mime_data->urls();

	QStringList files;
	for(const QUrl& url : urls)
	{
		QString local_file = url.toLocalFile();
		if(!local_file.isEmpty())
		{
			files << local_file;
		}
	}

	emit sigImportRequested(m->model->libraryId(), files, m->model->parentDirectory());
}

void FileListView::languageChanged() {}
void FileListView::skinChanged()
{
	QFontMetrics fm = this->fontMetrics();
	this->setIconSize(QSize(fm.height(), fm.height()));
}

void FileListView::initContextMenu()
{
	if(m->contextMenu){
		return;
	}

	m->contextMenu = new DirectoryContextMenu(DirectoryContextMenu::Mode::File, this);

	connect(m->contextMenu, &DirectoryContextMenu::sigInfoClicked, this, &FileListView::sigInfoClicked);
	connect(m->contextMenu, &DirectoryContextMenu::sigLyricsClicked, this, &FileListView::sigLyricsClicked);
	connect(m->contextMenu, &DirectoryContextMenu::sigEditClicked, this, &FileListView::sigEditClicked);
	connect(m->contextMenu, &DirectoryContextMenu::sigDeleteClicked, this, &FileListView::sigDeleteClicked);
	connect(m->contextMenu, &DirectoryContextMenu::sigPlayClicked, this, &FileListView::sigPlayClicked);
	connect(m->contextMenu, &DirectoryContextMenu::sigPlayNewTabClicked, this, &FileListView::sigPlayNewTabClicked);
	connect(m->contextMenu, &DirectoryContextMenu::sigPlayNextClicked, this, &FileListView::sigPlayNextClicked);
	connect(m->contextMenu, &DirectoryContextMenu::sigAppendClicked, this, &FileListView::sigAppendClicked);
	connect(m->contextMenu, &DirectoryContextMenu::sigRenameClicked, this, &FileListView::renameFileClicked);
	connect(m->contextMenu, &DirectoryContextMenu::sigRenameByTagClicked, this, &FileListView::renameFileByTagClicked);
	connect(m->contextMenu, &DirectoryContextMenu::sigCopyToLibrary, this, &FileListView::sigCopyToLibraryRequested);
	connect(m->contextMenu, &DirectoryContextMenu::sigMoveToLibrary, this, &FileListView::sigMoveToLibraryRequested);
}

QModelIndexList FileListView::selectedRows() const
{
	QItemSelectionModel* selection_model = this->selectionModel();

	if(selection_model) {
		return selection_model->selectedIndexes();
	}

	return QModelIndexList();
}

QStringList FileListView::selectedPaths() const
{
	const QStringList paths = m->model->files();
	const QModelIndexList selections = this->selectedRows();

	QStringList ret;
	for(const QModelIndex& idx : selections)
	{
		if(idx.column() != 0){
			continue;
		}

		int row = idx.row();
		if(Util::between(row, paths)){
			ret << paths[row];
		}
	}

	return ret;
}

void FileListView::setParentDirectory(LibraryId libraryId, const QString& dir)
{
	this->selectionModel()->clear();
	m->model->setParentDirectory(libraryId, dir);

	this->resizeRowsToContents();
}

QString FileListView::parentDirectory() const
{
	return m->model->parentDirectory();
}

void FileListView::setSearchFilter(const QString& search_string)
{
	if(search_string.isEmpty()){
		return;
	}

	const Library::SearchModeMask smm = GetSetting(Set::Lib_SearchMode);
	const QString search_text = Library::Utils::convertSearchstring(search_string, smm);

	for(int i=0; i<m->model->rowCount(); i++)
	{
		QModelIndex idx = m->model->index(i, 0);
		QString data = m->model->data(idx).toString();
		if(data.isEmpty()){
			continue;
		}

		if(!idx.isValid()){
			continue;
		}

		data = Library::Utils::convertSearchstring(data, smm);
		if(data.contains(search_text, Qt::CaseInsensitive)){
			this->selectionModel()->select(idx, (QItemSelectionModel::Select | QItemSelectionModel::Rows));
		}
	}
}

QMimeData* FileListView::dragableMimedata() const
{
	auto* mimedata = new Gui::CustomMimeData(this);
	m->model->mimeData(this->selectedIndexes());
	//mimedata->set_metadata(selected_metadata());

	QList<QUrl> urls;
	Util::Algorithm::transform(selectedPaths(), urls, [](const QString& path)
	{
		return QUrl::fromLocalFile(path);
	});

	mimedata->setUrls(urls);

	return mimedata;
}


int FileListView::mapModelIndexToIndex(const QModelIndex& idx) const
{
	return idx.row();
}

ModelIndexRange FileListView::mapIndexToModelIndexes(int idx) const
{
	return ModelIndexRange
	(
		m->model->index(idx, 0),
		m->model->index(idx, m->model->columnCount())
	);
}

void FileListView::keyPressEvent(QKeyEvent *event)
{
	event->setAccepted(false);
	SearchableTableView::keyPressEvent(event);
}

void FileListView::renameFileClicked()
{
	const QModelIndexList indexes = this->selectedRows();
	if(indexes.size() != 1){
		return;
	}

	const QModelIndex index = indexes.first();
	int row = index.row();

	QStringList files = m->model->files();
	if(!Util::between(row, files)){
		return;
	}

	auto [dir, file] = Util::File::splitFilename(files[row]);
	QString ext = Util::File::getFileExtension(files[row]);


	int lastDot = file.lastIndexOf(".");
	file = file.left(lastDot);

	QString inputText = Gui::LineInputDialog::getRenameFilename(this, tr("Enter new name"));
	if(inputText.isEmpty()) {
		return;
	}

	QString newName = QDir(dir).filePath(inputText);
	if(!newName.endsWith("." + ext)){
		newName += "." + ext;
	}

	emit sigRenameRequested(files[row], newName);
}


void FileListView::renameFileByTagClicked()
{
	const QModelIndexList indexes = this->selectedRows();
	const QStringList files = m->model->files();

	if(indexes.isEmpty() || files.isEmpty()) {
		return;
	}

	auto* dialog = new GUI_FileExpressionDialog(this);
	QDialog::DialogCode ret = QDialog::DialogCode(dialog->exec());
	if(ret == QDialog::Rejected) {
		return;
	}

	const QString expression = dialog->expression();
	if(expression.isEmpty()){
		return;
	}

	for(const QModelIndex& index : indexes)
	{
		int row = index.row();
		if(!Util::between(row, files)){
			return;
		}

		emit sigRenameByExpressionRequested(files[row], expression);
	}
}

