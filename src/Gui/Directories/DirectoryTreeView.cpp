/* DirectoryTreeView.cpp */

/* Copyright (C) 2011-2020  Lucio Carreras
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
	QString				last_search_term;

	DirectoryContextMenu*	context_menu=nullptr;
	DirectoryModel*		model=nullptr;
	IconProvider*		icon_provider=nullptr;
	Gui::ProgressBar*	progress_bar=nullptr;
	int					last_found_index;

	QTimer*				drag_timer=nullptr;
	QModelIndex			drag_target_index;

	Private(QObject* parent) :
		last_found_index(-1)
	{
		icon_provider = new IconProvider();

		drag_timer = new QTimer(parent);
		drag_timer->setSingleShot(true);
		drag_timer->setInterval(750);
	}

	~Private()
	{
		delete icon_provider; icon_provider = nullptr;
	}

	void reset_drag()
	{
		drag_timer->stop();
		drag_target_index = QModelIndex();
	}
};


DirectoryTreeView::DirectoryTreeView(QWidget* parent) :
	SearchableTreeView(parent),
	Gui::Dragable(this)
{
	m = Pimpl::make<Private>(this);

	m->model = new DirectoryModel(this);
	m->model->setFilter(QDir::NoDotAndDotDot | QDir::Dirs);
	m->model->setIconProvider(m->icon_provider);
	this->set_model(m->model);

	connect(m->drag_timer, &QTimer::timeout, this, &DirectoryTreeView::drag_timer_timeout);
	this->setItemDelegate(new Gui::StyledItemDelegate(this));

	auto* action = new QAction(this);
	action->setShortcut(QKeySequence("F2"));
	action->setShortcutContext(Qt::WidgetShortcut);
	connect(action, &QAction::triggered, this, &DirectoryTreeView::rename_dir_clicked);
	this->addAction(action);
}

DirectoryTreeView::~DirectoryTreeView() = default;

void DirectoryTreeView::language_changed() {}

void DirectoryTreeView::skin_changed()
{
	if(m) {
		m->model->setIconProvider(m->icon_provider);
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
	connect(m->context_menu, &DirectoryContextMenu::sig_append_clicked, this, &DirectoryTreeView::sig_append_clicked);
	connect(m->context_menu, &DirectoryContextMenu::sig_create_dir_clicked, this, &DirectoryTreeView::create_dir_clicked);
	connect(m->context_menu, &DirectoryContextMenu::sig_rename_clicked, this, &DirectoryTreeView::rename_dir_clicked);
	connect(m->context_menu, &DirectoryContextMenu::sig_collapse_all_clicked, this, &DirectoryTreeView::collapseAll);
	connect(m->context_menu, &DirectoryContextMenu::sig_copy_to_lib, this, &DirectoryTreeView::sig_copy_to_library_requested);
	connect(m->context_menu, &DirectoryContextMenu::sig_move_to_lib, this, &DirectoryTreeView::sig_move_to_library_requested);
}

QString DirectoryTreeView::directory_name(const QModelIndex& index)
{
	return m->model->filePath(index);
}

QModelIndexList DirectoryTreeView::selected_indexes() const
{
	QItemSelectionModel* selection_model = this->selectionModel();
	return selection_model->selectedRows();
}

QStringList DirectoryTreeView::selected_paths() const
{
	QStringList paths;

	const QModelIndexList selections = this->selected_indexes();
	for(const QModelIndex& idx : selections)
	{
		paths << m->model->filePath(idx);
	}

	return paths;
}

QModelIndex DirectoryTreeView::search(const QString& search_term)
{
	m->model->search_only_dirs(false);

	const QModelIndexList found_indexes = m->model->search_results(search_term);
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

	selectionModel()->select(idx, QItemSelectionModel::ClearAndSelect);
	setCurrentIndex(idx);
	expand(idx);
	scrollTo(idx, QAbstractItemView::PositionAtCenter);
}



void DirectoryTreeView::mousePressEvent(QMouseEvent* event)
{
	QTreeView::mousePressEvent(event);

	if(event->button() & Qt::RightButton)
	{
		if(!m->context_menu){
			init_context_menu();
		}

		int selection_size = this->selected_indexes().size();

		m->context_menu->show_directory_action(DirectoryContextMenu::EntryRename, (selection_size == 1));
		m->context_menu->show_directory_action(DirectoryContextMenu::EntryCreateDir, (selection_size == 1));
		m->context_menu->show_action(Library::ContextMenu::EntryDelete, true);

		QPoint pos = QWidget::mapToGlobal(event->pos());
		m->context_menu->exec(pos);
	}
}

static std::pair<Gui::LineInputDialog::ReturnValue, QString>
show_rename_dialog(const QString& old_name, QWidget* parent)
{
	Gui::LineInputDialog dialog(Lang::get(Lang::Rename), parent->tr("Enter new name"), old_name, parent);
	dialog.exec();

	return std::make_pair(dialog.return_value(), dialog.text());
}


void DirectoryTreeView::create_dir_clicked()
{
	QModelIndexList indexes = this->selected_indexes();
	if(indexes.size() != 1){
		return;
	}

	auto [ret, new_name] = show_rename_dialog("", this);
	if(!new_name.isEmpty() && !new_name.contains("/") && !new_name.contains("\\") && (ret == Gui::LineInputDialog::Ok))
	{
		m->model->mkdir(indexes.first(), new_name);
		this->expand(indexes.first());
	}
}

void DirectoryTreeView::rename_dir_clicked()
{
	const QModelIndexList indexes = this->selected_indexes();
	if(indexes.size() != 1){
		return;
	}

	const QModelIndex index = indexes.first();

	const QString dir = m->model->filePath(index);
	QDir d(dir);

	auto [ret, new_name] = show_rename_dialog(d.dirName(), this);
	if(!new_name.isEmpty() && (ret == Gui::LineInputDialog::Ok))
	{
		d.cdUp();
		emit sig_rename_requested(dir, d.filePath(new_name));
	}
}

void DirectoryTreeView::set_busy(bool b)
{
	if(b)
	{
		this->setDragDropMode(DragDropMode::NoDragDrop);

		if(!m->progress_bar)
		{
			m->progress_bar = new Gui::ProgressBar(this);
			m->progress_bar->show();
		}
	}

	else
	{
		this->setDragDropMode(DragDropMode::DragDrop);

		if(m->progress_bar)
		{
			m->progress_bar->hide();
		}
	}
}

void DirectoryTreeView::drag_timer_timeout()
{
	if(m->drag_target_index.isValid())
	{
		this->expand(m->drag_target_index);
	}

	m->reset_drag();
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

void DirectoryTreeView::dragEnterEvent(QDragEnterEvent* event)
{
	m->reset_drag();

	const QMimeData* mime_data = event->mimeData();
	if(!mime_data || !mime_data->hasUrls())
	{
		event->ignore();
		return;
	}

	event->accept();
}

void DirectoryTreeView::dragMoveEvent(QDragMoveEvent* event)
{
	const QMimeData* mime_data = event->mimeData();
	if(!mime_data){
		event->ignore();
		return;
	}

	const QModelIndex index = this->indexAt(event->pos());
	if(index != m->drag_target_index)
	{
		m->drag_target_index = index;

		if(index.isValid()) {
			m->drag_timer->start();
		}
	}

	const auto* cmd = Gui::MimeData::custom_mimedata(mime_data);

	if(mime_data->hasUrls()) {
		event->acceptProposedAction();
	}

	else if(cmd && cmd->has_source(this)) {
		event->acceptProposedAction();
	}

	else {
		event->ignore();
	}

	if(event->isAccepted() && !selectionModel()->isSelected(index))
	{
		selectionModel()->select(index, QItemSelectionModel::ClearAndSelect);
	}

	SearchableTreeView::dragMoveEvent(event);
}

void DirectoryTreeView::dragLeaveEvent(QDragLeaveEvent* event)
{
	m->reset_drag();
	event->accept();

	SearchableTreeView::dragLeaveEvent(event);
}

void DirectoryTreeView::dropEvent(QDropEvent* event)
{
	event->accept();

	m->drag_timer->stop();
	m->drag_target_index = QModelIndex();

	QModelIndex index = this->indexAt(event->pos());
	if(!index.isValid()){
		return;
	}

	const QMimeData* mimedata = event->mimeData();
	if(!mimedata){
		return;
	}

	const QString target_dir = m->model->filePath(index);
	const auto* cmd = Gui::MimeData::custom_mimedata(mimedata);
	if(cmd)
	{
		handle_sayonara_drop(cmd, target_dir);
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

		LibraryId id = m->model->library_info().id();
		emit sig_import_requested(id, files, target_dir);
	}
}

void DirectoryTreeView::handle_sayonara_drop(const Gui::CustomMimeData* cmd, const QString& target_dir)
{
	QList<QUrl> urls = cmd->urls();
	QStringList source_files, source_dirs;

	for(const QUrl& url : urls)
	{
		const QString source = url.toLocalFile();

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

	if(!source_dirs.isEmpty())
	{
		switch(drop_action)
		{
			case DirectoryTreeView::DropAction::Copy:
				emit sig_copy_requested(source_dirs, target_dir);
				break;
			case DirectoryTreeView::DropAction::Move:
				emit sig_move_requested(source_dirs, target_dir);
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
				emit sig_copy_requested(source_files, target_dir);
				break;
			case DirectoryTreeView::DropAction::Move:
				emit sig_move_requested(source_files, target_dir);
				break;
			default:
				break;
		}
	}
}

DirectoryTreeView::DropAction DirectoryTreeView::show_drop_menu(const QPoint& pos)
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
	const QModelIndexList selected_items = this->selected_indexes();

	QList<QUrl> urls;
	for(const QModelIndex& index : selected_items)
	{
		const QString path = m->model->filePath(index);
		urls << QUrl::fromLocalFile(path);
		sp_log(Log::Debug, this) << "Dragging " << path;
	}

	auto* cmd = new Gui::CustomMimeData(this);
	cmd->setUrls(urls);

	return cmd;
}

void DirectoryTreeView::set_library(const Library::Info& info)
{
	m->model->set_library(info);
	for(int i=1; i<m->model->columnCount(); i++) {
		this->hideColumn(i);
	}

	const QModelIndex index = m->model->index(info.path());
	this->setRootIndex(index);

	connect(selectionModel(), &QItemSelectionModel::selectionChanged, this, &DirectoryTreeView::selection_changed);
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
