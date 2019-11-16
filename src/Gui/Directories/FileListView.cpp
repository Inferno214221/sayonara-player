/* FileListView.cpp */

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

#include "FileListView.h"
#include "FileListModel.h"
#include "DirectoryIconProvider.h"
#include "DirectoryContextMenu.h"
#include "GUI_FileExpressionDialog.h"

#include "Components/Directories/DirectoryReader.h"
#include "Components/Directories/FileOperations.h"

#include "Utils/globals.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Library/SearchMode.h"
#include "Utils/FileUtils.h"
#include "Utils/Language/Language.h"
#include "Utils/Logger/Logger.h"

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

struct FileListView::Private
{
	DirectoryContextMenu*	context_menu=nullptr;
	FileListModel*			model=nullptr;
	FileOperations*			file_operations=nullptr;

	Private(FileListView* parent)
	{
		model = new FileListModel(parent);
		file_operations = new FileOperations(parent);
	}
};

FileListView::FileListView(QWidget* parent) :
	SearchableListView(parent),
	Gui::Dragable(this)
{
	m = Pimpl::make<Private>(this);

	this->set_model(m->model);
	this->setItemDelegate(new Gui::StyledItemDelegate(this));
	this->setSelectionMode(QAbstractItemView::ExtendedSelection);
	this->setDragEnabled(true);
	this->setIconSize(QSize(16, 16));

	{ // rename by pressing F2
		QAction* action = new QAction(this);
		connect(action, &QAction::triggered, this, &FileListView::rename_file_clicked);
		action->setShortcut(QKeySequence("F2"));
		action->setShortcutContext(Qt::WidgetShortcut);
		this->addAction(action);
	}

	new QShortcut(QKeySequence(Qt::Key_Return), this, SIGNAL(sig_enter_pressed()), nullptr, Qt::WidgetShortcut);
	new QShortcut(QKeySequence(Qt::Key_Enter), this, SIGNAL(sig_enter_pressed()), nullptr, Qt::WidgetShortcut);
}

FileListView::~FileListView() = default;

void FileListView::mousePressEvent(QMouseEvent* event)
{
	SearchableListView::mousePressEvent(event);

	if(event->button() & Qt::LeftButton)
	{
		this->drag_pressed(event->pos());
	}
}

void FileListView::mouseMoveEvent(QMouseEvent* event)
{
	QDrag* drag = Dragable::drag_moving(event->pos());
	if(drag)
	{
		connect(drag, &QObject::destroyed, this, [=]()
		{
			this->drag_released(Dragable::ReleaseReason::Destroyed);
		});
	}
}

void FileListView::contextMenuEvent(QContextMenuEvent* event)
{
	if(!m->context_menu){
		init_context_menu();
	}

	const QModelIndexList indexes = selected_rows();
	auto num_audio_files = std::count_if(indexes.begin(), indexes.end(), [](const QModelIndex& index)
	{
		QString filename = index.data(Qt::UserRole).toString();
		return Util::File::is_soundfile(filename);
	});

	m->context_menu->show_action
	(
		Library::ContextMenu::EntryLyrics,
		(num_audio_files==1)
	);

	m->context_menu->set_rename_visible
	(
		(num_audio_files==1)
	);

	if(num_audio_files > 0)
	{
		QPoint pos = QWidget::mapToGlobal(event->pos());
		m->context_menu->exec(pos);
	}
}

void FileListView::dragEnterEvent(QDragEnterEvent *event)
{
	event->accept();
}

void FileListView::dragMoveEvent(QDragMoveEvent *event)
{
	const QMimeData* mime_data = event->mimeData();
	const auto* cmd = Gui::MimeData::custom_mimedata(mime_data);
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

	if(m->model->library_id() < 0){
		return;
	}

	const QMimeData* mime_data = event->mimeData();
	if(!mime_data){
		sp_log(Log::Debug, this) << "Drop: No Mimedata";
		return;
	}

	if(Gui::MimeData::is_player_drag(mime_data)){
		sp_log(Log::Debug, this) << "Drop: Internal player drag";
		return;
	}

	if(!mime_data->hasUrls())
	{
		sp_log(Log::Debug, this) << "Drop: No Urls";
		return;
	}

	const QList<QUrl> urls = mime_data->urls();

	QStringList files; files.reserve(urls.size());
	for(const QUrl& url : urls)
	{
		QString local_file = url.toLocalFile();
		if(!local_file.isEmpty()){
			files << local_file;
		}
	}

	sp_log(Log::Debug, this) << "Drop: " << files.size() << " files into library " << m->model->library_id();

	emit sig_import_requested(m->model->library_id(), files, m->model->parent_directory());
}

void FileListView::language_changed() {}
void FileListView::skin_changed()
{
	QFontMetrics fm = this->fontMetrics();
	this->setIconSize(QSize(fm.height(), fm.height()));
}

void FileListView::init_context_menu()
{
	if(m->context_menu){
		return;
	}

	m->context_menu = new DirectoryContextMenu(DirectoryContextMenu::Mode::File, this);

	connect(m->context_menu, &DirectoryContextMenu::sig_info_clicked, this, &FileListView::sig_info_clicked);
	connect(m->context_menu, &DirectoryContextMenu::sig_lyrics_clicked, this, &FileListView::sig_lyrics_clicked);
	connect(m->context_menu, &DirectoryContextMenu::sig_edit_clicked, this, &FileListView::sig_edit_clicked);
	connect(m->context_menu, &DirectoryContextMenu::sig_delete_clicked, this, &FileListView::sig_delete_clicked);
	connect(m->context_menu, &DirectoryContextMenu::sig_play_clicked, this, &FileListView::sig_play_clicked);
	connect(m->context_menu, &DirectoryContextMenu::sig_play_new_tab_clicked, this, &FileListView::sig_play_new_tab_clicked);
	connect(m->context_menu, &DirectoryContextMenu::sig_play_next_clicked, this, &FileListView::sig_play_next_clicked);
	connect(m->context_menu, &DirectoryContextMenu::sig_append_clicked, this, &FileListView::sig_append_clicked);
	connect(m->context_menu, &DirectoryContextMenu::sig_rename_clicked, this, &FileListView::rename_file_clicked);
	connect(m->context_menu, &DirectoryContextMenu::sig_rename_by_tag_clicked, this, &FileListView::rename_file_by_tag_clicked);
}


QModelIndexList FileListView::selected_rows() const
{
	QItemSelectionModel* selection_model = this->selectionModel();

	if(selection_model) {
		return selection_model->selectedIndexes();
	}

	return QModelIndexList();
}

MetaDataList FileListView::selected_metadata() const
{
	const QStringList paths = selected_paths();

	DirectoryReader reader;
	return reader.scan_metadata(paths);
}

QStringList FileListView::selected_paths() const
{
	const QStringList paths = m->model->files();
	const QModelIndexList selections = this->selected_rows();

	QStringList ret;
	for(const QModelIndex& idx : selections)
	{
		int row = idx.row();
		if(Util::between(row, paths)){
			ret << paths[row];
		}
	}

	return ret;
}


void FileListView::set_parent_directory(LibraryId library_id, const QString& dir)
{
	this->selectionModel()->clear();

	m->model->set_parent_directory(library_id, dir);
}

QString FileListView::parent_directory() const
{
	return m->model->parent_directory();
}

void FileListView::set_search_filter(const QString& search_string)
{
	if(search_string.isEmpty()){
		return;
	}

	const Library::SearchModeMask smm = GetSetting(Set::Lib_SearchMode);
	const QString search_text = Library::Utils::convert_search_string(search_string, smm);

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

		data = Library::Utils::convert_search_string(data, smm);
		if(data.contains(search_text, Qt::CaseInsensitive)){
			this->selectionModel()->select(idx, (QItemSelectionModel::Select | QItemSelectionModel::Rows));
		}
	}
}


QMimeData* FileListView::dragable_mimedata() const
{
	auto* mimedata = new Gui::CustomMimeData(this);
	mimedata->set_metadata(selected_metadata());

	QStringList paths = selected_paths();
	std::remove_if(paths.begin(), paths.end(), [](const QString& path){
		return (Util::File::is_soundfile(path));
	});

	QList<QUrl> urls; urls.reserve(paths.size());
	std::transform(paths.begin(), paths.end(), std::back_inserter(urls), [](const QString& path)
	{
		return QUrl::fromLocalFile(path);
	});

	mimedata->setUrls(urls);

	return mimedata;
}


int FileListView::index_by_model_index(const QModelIndex& idx) const
{
	return idx.row();
}

ModelIndexRange FileListView::model_indexrange_by_index(int idx) const
{
	return ModelIndexRange(m->model->index(idx), m->model->index(idx));
}

void FileListView::keyPressEvent(QKeyEvent *event)
{
	event->setAccepted(false);
	SearchableListView::keyPressEvent(event);
}


void FileListView::rename_file(const QString& old_name, const QString& new_name)
{
	QDir d(old_name);
	d.cdUp();

	QString new_full_name = new_name;
	QString ext = Util::File::get_file_extension(old_name);
	if(!new_full_name.toLower().endsWith("." + ext.toLower()))
	{
		new_full_name = d.filePath(new_name) + "." + ext;
	}

	sp_log(Log::Debug, this) << "Will rename " << old_name << " to " << new_full_name;

	m->file_operations->rename_file(old_name, new_full_name);
	m->model->set_parent_directory(m->model->library_id(), m->model->parent_directory());

	const QStringList files = m->model->files();
	int new_file_index = files.indexOf(new_full_name);
	if(Util::between(new_file_index, files))
	{
		this->select_row(new_file_index);
	}
}

void FileListView::rename_file_clicked()
{
	const QModelIndexList indexes = this->selected_rows();
	if(indexes.size() != 1){
		return;
	}

	const QModelIndex index = indexes.first();
	int row = index.row();

	QStringList files = m->model->files();
	if(!Util::between(row, files)){
		return;
	}

	QString file = Util::File::get_filename_of_path(files[row]);

	int last_dot = file.lastIndexOf(".");
	file = file.left(last_dot);

	QString new_name;
	{
		Gui::LineInputDialog dialog(Lang::get(Lang::Rename), tr("Enter new name"), file, this);
		dialog.exec();
		new_name = dialog.text();
		if(dialog.return_value() != Gui::LineInputDialog::Ok || new_name.isEmpty())
		{
			return;
		}
	}

	this->rename_file(files[row], new_name);
}


void FileListView::rename_file_by_tag_clicked()
{
	const QModelIndexList indexes = this->selected_rows();
	const QStringList files = m->model->files();

	if(indexes.isEmpty() || files.isEmpty()) {
		return;
	}

	auto* dialog = new GUI_FileExpressionDialog(this);
	QDialog::DialogCode ret = QDialog::DialogCode(dialog->exec());
	if(ret == QDialog::Rejected)
	{
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

		m->file_operations->rename_file_by_expression(files[row], expression);
	}

	m->model->set_parent_directory(m->model->library_id(), m->model->parent_directory());
}

