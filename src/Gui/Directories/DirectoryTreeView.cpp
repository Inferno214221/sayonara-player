/* DirectoryTreeView.cpp */

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

#include "DirectoryTreeView.h"
#include "DirectoryDelegate.h"
#include "DirectoryIconProvider.h"
#include "DirectoryModel.h"
#include "DirectoryContextMenu.h"

#include "Components/Directories/DirectoryReader.h"
#include "Components/Directories/FileOperations.h"

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
#include "Utils/Language/Language.h"
#include "Utils/Logger/Logger.h"

#include <QDir>
#include <QMouseEvent>
#include <QDrag>
#include <QTimer>
#include <QAction>

#include <algorithm>

struct DirectoryTreeView::Private
{
	QString				last_search_term;
	FileOperations*		file_operations=nullptr;

	DirectoryContextMenu*		context_menu=nullptr;
	DirectoryModel*		model = nullptr;
	IconProvider*		icon_provider = nullptr;
	QModelIndex			drag_move_index;
	QTimer*				drag_move_timer=nullptr;
	int					last_found_index;

	Private(QObject* parent) :
		last_found_index(-1)
	{
		file_operations = new FileOperations(parent);
		icon_provider = new IconProvider();
		drag_move_timer = new QTimer(parent);
		drag_move_timer->setInterval(750);
		drag_move_timer->setSingleShot(true);
	}

	~Private()
	{
		delete icon_provider; icon_provider = nullptr;
	}
};


DirectoryTreeView::DirectoryTreeView(QWidget *parent) :
	SearchableTreeView(parent),
	Gui::Dragable(this)
{
	m = Pimpl::make<Private>(this);

	QString root_path = Util::sayonara_path("Libraries");

	m->model = new DirectoryModel(this);
	m->model->setFilter(QDir::NoDotAndDotDot | QDir::Dirs);
	m->model->setIconProvider(m->icon_provider);
	m->model->setRootPath(root_path);

	connect(m->file_operations, &FileOperations::sig_copy_finished, this, &DirectoryTreeView::copy_finished);
	connect(m->file_operations, &FileOperations::sig_copy_started, this, &DirectoryTreeView::copy_started);

	this->set_model(m->model);
	this->setItemDelegate(new DirectoryDelegate(this));

	QAction* action = new QAction(this);
	connect(action, &QAction::triggered, this, &DirectoryTreeView::rename_dir_clicked);
	action->setShortcut(QKeySequence("F2"));
	action->setShortcutContext(Qt::WidgetShortcut);
	this->addAction(action);

	for(int i=1; i<4; i++){
		this->hideColumn(i);
	}

	this->setRootIndex(m->model->index(root_path));

	connect(m->drag_move_timer, &QTimer::timeout, this, &DirectoryTreeView::drag_move_timer_finished);

	connect(selectionModel(), &QItemSelectionModel::selectionChanged, this, &DirectoryTreeView::selection_changed);
}

DirectoryTreeView::~DirectoryTreeView() = default;

LibraryId DirectoryTreeView::library_id(const QModelIndex& index) const
{
	return m->model->library_id(index);
}

void DirectoryTreeView::language_changed() {}

void DirectoryTreeView::skin_changed()
{
	if(m && m->model){
		m->model->setIconProvider(m->icon_provider);
	}
}

void DirectoryTreeView::keyPressEvent(QKeyEvent* event)
{
	event->setAccepted(false);

	switch(event->key())
	{
		case Qt::Key_Enter:
		case Qt::Key_Return:
			emit sig_enter_pressed();
			return;

		default: break;
	}

	m->model->search_only_dirs(true);

	SearchableTreeView::keyPressEvent(event);
}

void DirectoryTreeView::init_context_menu()
{
	if(m->context_menu){
		return;
	}

	m->context_menu = new DirectoryContextMenu(DirectoryContextMenu::Mode::Dir, this);

	connect(m->context_menu, &DirectoryContextMenu::sig_info_clicked, this, &DirectoryTreeView::sig_info_clicked);
	connect(m->context_menu, &DirectoryContextMenu::sig_lyrics_clicked, this, &DirectoryTreeView::sig_info_clicked);
	connect(m->context_menu, &DirectoryContextMenu::sig_edit_clicked, this, &DirectoryTreeView::sig_info_clicked);
	connect(m->context_menu, &DirectoryContextMenu::sig_delete_clicked, this, &DirectoryTreeView::sig_delete_clicked);
	connect(m->context_menu, &DirectoryContextMenu::sig_play_clicked, this, &DirectoryTreeView::sig_play_clicked);
	connect(m->context_menu, &DirectoryContextMenu::sig_play_new_tab_clicked, this, &DirectoryTreeView::sig_play_new_tab_clicked);
	connect(m->context_menu, &DirectoryContextMenu::sig_play_next_clicked, this, &DirectoryTreeView::sig_play_next_clicked);
	connect(m->context_menu, &DirectoryContextMenu::sig_create_dir_clicked, this, &DirectoryTreeView::create_dir_clicked);
	connect(m->context_menu, &DirectoryContextMenu::sig_rename_clicked, this, &DirectoryTreeView::rename_dir_clicked);
	connect(m->context_menu, &DirectoryContextMenu::sig_collapse_all_clicked, this, &DirectoryTreeView::collapseAll);
}

QString DirectoryTreeView::directory_name(const QModelIndex &index)
{
	return m->model->filePath(index);
}

QString DirectoryTreeView::directory_name_origin(const QModelIndex& index)
{
	return m->model->filepath_origin(index);
}

QModelIndexList DirectoryTreeView::selected_indexes() const
{
	QItemSelectionModel* selection_model = this->selectionModel();

	return selection_model->selectedRows();
}

QStringList DirectoryTreeView::selected_paths() const
{
	QModelIndexList selections = this->selected_indexes();
	if(selections.isEmpty()){
		return QStringList();
	}

	QStringList paths;
	for(const QModelIndex& idx : selections)
	{
		paths << m->model->filepath_origin(idx);
	}

	return paths;
}


QModelIndex DirectoryTreeView::search(const QString& search_term)
{
	m->model->search_only_dirs(false);

	QModelIndexList found_indexes = m->model->search_results(search_term);
	if(found_indexes.isEmpty())
	{
		m->last_found_index = 0;
		return QModelIndex();
	}

	if(m->last_search_term == search_term)
	{
		m->last_found_index++;
		if(m->last_found_index >= found_indexes.count()){
			m->last_found_index = 0;
		}
	}

	else {
		m->last_found_index = 0;
		m->last_search_term = search_term;
	}

	QModelIndex found_idx = found_indexes[m->last_found_index];

	scrollTo(found_idx, QAbstractItemView::PositionAtCenter);
	selectionModel()->select(found_idx, QItemSelectionModel::ClearAndSelect);
	expand(found_idx);

	emit sig_directory_loaded(found_idx);

	return found_idx;
}

void DirectoryTreeView::select_match(const QString& str, SearchDirection direction)
{
	QModelIndex idx = match_index(str, direction);
	if(!idx.isValid()){
		return;
	}

	expand(idx);
	scrollTo(idx, QAbstractItemView::PositionAtCenter);
	this->clearSelection();
	selectionModel()->clearSelection();
	selectionModel()->select(idx, QItemSelectionModel::ClearAndSelect);
	setCurrentIndex(idx);

	if(m->model->canFetchMore(idx)){
		m->model->fetchMore(idx);
	}
}

bool DirectoryTreeView::has_drag_label() const
{
	return (!this->selected_paths().isEmpty());
}

QString DirectoryTreeView::drag_label() const
{
	QStringList paths = selected_paths();
	for(QString& path : paths)
	{
		QString d, f;
		Util::File::split_filename(path, d, f);
		path = f;
	}
	return paths.join(", ");
}

void DirectoryTreeView::mousePressEvent(QMouseEvent* event)
{
	QTreeView::mousePressEvent(event);

	if(event->buttons() & Qt::LeftButton)
	{
		Dragable::drag_pressed( event->pos() );
	}

	if(event->button() & Qt::RightButton){
		QPoint pos = QWidget::mapToGlobal( event->pos() );

		if(!m->context_menu){
			init_context_menu();
		}

		QModelIndex index = indexAt(event->pos());
		bool is_root = m->model->is_root(index);

		m->context_menu->set_rename_visible(
			(this->selected_indexes().size()==1) && (!is_root)
		);

		m->context_menu->set_create_dir_visible(
			(this->selected_indexes().size()==1)
		);

		m->context_menu->show_action(Library::ContextMenu::EntryDelete, !is_root);
		m->context_menu->exec(pos);
	}
}

void DirectoryTreeView::create_dir_clicked()
{
	QModelIndexList indexes = this->selected_indexes();
	if(indexes.size() != 1){
		return;
	}

	Gui::LineInputDialog dialog(Lang::get(Lang::Rename), tr("Enter new name"), this);
	dialog.exec();

	QString new_dir_name = dialog.text();

	if(!new_dir_name.isEmpty() && !new_dir_name.contains("/") && !new_dir_name.contains("\\"))
	{
		m->model->mkdir(indexes.first(), new_dir_name);
		this->expand(indexes.first());
	}
}

void DirectoryTreeView::rename_dir_clicked()
{
	QModelIndexList indexes = this->selected_indexes();
	if(indexes.size() != 1){
		return;
	}

	QModelIndex index = indexes.first();
	if(m->model->is_root(index)){
		return;
	}

	QString dir = m->model->filepath_origin(index);
	QDir d(dir);

	Gui::LineInputDialog dialog(Lang::get(Lang::Rename), tr("Enter new name"), d.dirName(), this);
	dialog.exec();

	QString dir_renamed = dialog.text();

	if(!dir_renamed.isEmpty() && (dialog.return_value() == Gui::LineInputDialog::Ok))
	{
		d.cdUp();
		m->file_operations->rename_dir(dir, d.filePath(dir_renamed));
	}
}

void DirectoryTreeView::copy_started()
{
	this->setDragDropMode(DragDropMode::DragOnly);
	Gui::ProgressBar* pb = new Gui::ProgressBar(this);
	pb->show();
	connect(m->file_operations, &FileOperations::sig_copy_finished, pb, &Gui::ProgressBar::deleteLater);

	emit sig_copy_started();
}

void DirectoryTreeView::copy_finished()
{
	this->setDragDropMode(DragDropMode::DragDrop);
	emit sig_copy_finished();
}



void DirectoryTreeView::mouseMoveEvent(QMouseEvent* e)
{
	QDrag* drag = Dragable::drag_moving(e->pos());
	if(drag)
	{
		connect(drag, &QDrag::destroyed, this, [=]()
		{
			this->drag_released(Dragable::ReleaseReason::Destroyed);
		});
	}
}

void DirectoryTreeView::dragEnterEvent(QDragEnterEvent* event)
{
	const QMimeData* mime_data = event->mimeData();
	if(!mime_data->hasUrls()){
		sp_log(Log::Warning, this) << "DragMove: No Mimedata";
		return;
	}

	event->accept();
}

void DirectoryTreeView::dragLeaveEvent(QDragLeaveEvent* event)
{
	event->accept();
}

void DirectoryTreeView::drag_move_timer_finished()
{
	if(m->drag_move_index.isValid())
	{
		this->expand(m->drag_move_index);

		m->drag_move_timer->stop();
		m->drag_move_index = QModelIndex();
	}
}


void DirectoryTreeView::dragMoveEvent(QDragMoveEvent* event)
{
	event->ignore();

	const QMimeData* mime_data = event->mimeData();
	if(!mime_data){
		return;
	}

	const auto* cmd = Gui::MimeData::custom_mimedata(mime_data);
	if(cmd)
	{
		if(cmd->has_source(this)){
			event->acceptProposedAction();
		}

		if(cmd->hasUrls())
		{
			event->acceptProposedAction();
		}
	}

	else {
		if(mime_data->hasUrls()) {
			event->acceptProposedAction();
		}
	}

	if(!event->isAccepted()){
		m->drag_move_timer->stop();
		return;
	}

	QModelIndex index = this->indexAt(event->pos());
	selectionModel()->select(index, QItemSelectionModel::ClearAndSelect);

	if(m->drag_move_index != index && index.isValid())
	{
		m->drag_move_timer->start();
	}

	m->drag_move_index = index;

	if(!index.isValid()){
		m->drag_move_timer->stop();
		sp_log(Log::Debug, this) << "Stop timer";
	}

	QTreeView::dragMoveEvent(event);
}

void DirectoryTreeView::dropEvent(QDropEvent* event)
{
	m->drag_move_timer->stop();

	event->accept();

	QModelIndex index = this->indexAt(event->pos());
	if(!index.isValid()){
		sp_log(Log::Warning, this) << "Drop: Invalid index";
		return;
	}

	const QMimeData* mimedata = event->mimeData();
	if(!mimedata){
		sp_log(Log::Warning, this) << "Drop: No Mimedata";
		return;
	}

	QString target_dir = m->model->filepath_origin(index);
	const auto* cmd = Gui::MimeData::custom_mimedata(mimedata);
	if(cmd)
	{
		handle_sayonara_drop(cmd, target_dir);
	}

	else if(mimedata->hasUrls())
	{
		LibraryId lib_id = m->model->library_id(index);
		QStringList files;

		const QList<QUrl> urls = mimedata->urls();
		for(const QUrl& url : urls)
		{
			QString local_file = url.toLocalFile();
			if(!local_file.isEmpty()){
				files << local_file;
			}
		}

		sp_log(Log::Debug, this) << "Drop: " << files.size() << " files into library " << lib_id;

		if(lib_id < 0){
			return;
		}

		emit sig_import_requested(lib_id, files, target_dir);
	}
}

void DirectoryTreeView::handle_sayonara_drop(const Gui::CustomMimeData* cmd, const QString& target_dir)
{
	QList<QUrl> urls = cmd->urls();
	QStringList source_files, source_dirs;

	for(const QUrl& url : urls)
	{
		QString source = url.toLocalFile();

		if(Util::File::is_dir(source))
		{
			if(Util::File::can_copy_dir(source, target_dir)){
				source_dirs << source;
			}
		}

		else if(Util::File::is_file(source))
		{
			source_files << url.toLocalFile();
		}
	}

	if(source_dirs.isEmpty() && source_files.isEmpty()){
		return;
	}

	DirectoryTreeView::DropAction drop_action = show_drop_menu(QCursor::pos());

	if(!source_dirs.isEmpty()){
		switch(drop_action)
		{
			case DirectoryTreeView::DropAction::Copy:
				m->file_operations->copy_dirs(source_dirs, target_dir);
				break;
			case DirectoryTreeView::DropAction::Move:
				m->file_operations->move_dirs(source_dirs, target_dir);
				break;
			default:
				break;
		}
	}

	if(!source_files.isEmpty())
	{
		switch(drop_action)
		{
			case DirectoryTreeView::DropAction::Copy:
				m->file_operations->copy_files(source_files, target_dir);
				break;
			case DirectoryTreeView::DropAction::Move:
				m->file_operations->move_files(source_files, target_dir);
				break;
			default:
				break;
		}
	}
}

DirectoryTreeView::DropAction DirectoryTreeView::show_drop_menu(const QPoint& pos)
{
	QMenu* menu = new QMenu(this);

	QList<QAction*> actions;
	actions << new QAction(tr("Copy here"), menu);
	actions << new QAction(tr("Move here"), menu);
	actions << menu->addSeparator();
	actions << new QAction(Lang::get(Lang::Cancel), menu);
	menu->addActions(actions);

	QAction* action = menu->exec(pos);
	DirectoryTreeView::DropAction drop_action = DirectoryTreeView::DropAction::Cancel;

	if(action == actions[0]){
		drop_action = DirectoryTreeView::DropAction::Copy;
	}

	else if(action == actions[1]){
		drop_action = DirectoryTreeView::DropAction::Move;
	}

	menu->deleteLater();

	return drop_action;
}

void DirectoryTreeView::selection_changed(const QItemSelection& selected, const QItemSelection& deselected)
{
	Q_UNUSED(deselected)

	QModelIndex index;
	if(!selected.indexes().isEmpty()){
		index = selected.indexes().first();
	}

	emit sig_current_index_changed(index);
}


QMimeData* DirectoryTreeView::dragable_mimedata() const
{
	QModelIndexList selected_items = this->selected_indexes();

	for(const QModelIndex& index : selected_items)
	{
		if(m->model->is_root(index)){
			return nullptr;
		}
	}

	auto* cmd = new Gui::CustomMimeData(this);

	QList<QUrl> urls;
	for(const QModelIndex& index : selected_items)
	{
		if(m->model->is_root(index)){
			continue;
		}

		urls << QUrl::fromLocalFile(m->model->filepath_origin(index));
		sp_log(Log::Debug, this) << "Dragging " << m->model->filepath_origin(index);
	}

	cmd->setUrls(urls);

	return cmd;
}

int DirectoryTreeView::index_by_model_index(const QModelIndex& idx) const
{
	Q_UNUSED(idx)
	return -1;
}

ModelIndexRange DirectoryTreeView::model_indexrange_by_index(int idx) const
{
	Q_UNUSED(idx)
	return ModelIndexRange(QModelIndex(), QModelIndex());
}
