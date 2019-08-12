/* LibraryGenreView.cpp */

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

#include "GenreView.h"

#include "Components/Library/GenreFetcher.h"

#include "Gui/Library/Utils/GenreViewContextMenu.h"
#include "Gui/Utils/CustomMimeData.h"
#include "Gui/Utils/Delegates/StyledItemDelegate.h"
#include "Gui/Utils/InputDialog/LineInputDialog.h"

#include "Utils/Utils.h"
#include "Utils/Algorithm.h"
#include "Utils/Tree.h"
#include "Utils/Set.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/MetaData/Genre.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Language/Language.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Message/Message.h"

#include <QDropEvent>
#include <QContextMenuEvent>
#include <QStyledItemDelegate>
#include <QTreeWidget>
#include <QShortcut>

namespace Algorithm=Util::Algorithm;
using StringSet=Util::Set<QString>;
using namespace Library;

static bool is_invalid_genre(const QModelIndex& index)
{
	return (index.data(Qt::UserRole) == 5000);
}

struct GenreView::Private
{
	QStringList				expanded_items;
	GenreFetcher*			genre_fetcher=nullptr;
	GenreViewContextMenu*	context_menu=nullptr;
	GenreNode*				genres=nullptr;

	int						default_indent;
	bool					filled;
	bool					is_dragging;

	Private(QWidget* parent) :
		genre_fetcher(new GenreFetcher(parent)),
		genres(new GenreNode("root")),
		filled(false),
		is_dragging(false)
	{}

	~Private()
	{
		delete genres; genres=nullptr;
	}
};

GenreView::GenreView(QWidget* parent) :
	WidgetTemplate<QTreeWidget>(parent)
{
	m = Pimpl::make<Private>(this);
	m->default_indent = this->indentation();

	setAcceptDrops(true);
	setDragDropMode(GenreView::DragDrop);
	setAlternatingRowColors(true);
	setItemDelegate(new Gui::StyledItemDelegate(this));

	QItemSelectionModel* ism = this->selectionModel();
	connect(ism, &QItemSelectionModel::selectionChanged, this, &GenreView::selection_changed);

	connect(this, &QTreeWidget::itemCollapsed, this, &GenreView::item_collapsed);
	connect(this, &QTreeWidget::itemExpanded, this, &GenreView::item_expanded);

	connect(m->genre_fetcher, &GenreFetcher::sig_finished, this, &GenreView::update_finished);
	connect(m->genre_fetcher, &GenreFetcher::sig_progress, this, &GenreView::progress_changed);
	connect(m->genre_fetcher, &GenreFetcher::sig_genres_fetched, this, &GenreView::reload_genres);

	ListenSettingNoCall(Set::Lib_GenreTree, GenreView::switch_tree_list);

	new QShortcut(QKeySequence(Qt::Key_Enter), this, SLOT(expand_current_item()), nullptr, Qt::WidgetShortcut);
	new QShortcut(QKeySequence(Qt::Key_Return), this, SLOT(expand_current_item()), nullptr, Qt::WidgetShortcut);
	new QShortcut(QKeySequence(Qt::Key_F2), this, SLOT(rename_pressed()), nullptr, Qt::WidgetShortcut);
}

GenreView::~GenreView() = default;

void GenreView::init(LocalLibrary* library)
{
	m->genre_fetcher->set_local_library(library);
}

void GenreView::progress_changed(int progress)
{
	emit sig_progress(tr("Updating genres"), progress);
}

void GenreView::update_finished()
{
	this->setAcceptDrops(true);
	emit sig_progress("", -1);
}

void GenreView::item_expanded(QTreeWidgetItem* item)
{
	m->expanded_items << item->text(0);
}

void GenreView::item_collapsed(QTreeWidgetItem* item)
{
	m->expanded_items.removeAll(item->text(0));
}

void GenreView::expand_current_item()
{
	QTreeWidgetItem* item = this->currentItem();
	if(item)
	{
		if(item->isExpanded() || item->childCount() == 0){
			emit activated(this->currentIndex());
		}
		else {
			item->setExpanded(true);
		}
	}
}

void GenreView::new_pressed()
{
	Gui::LineInputDialog dialog
	(
		Lang::get(Lang::Genre),
		Lang::get(Lang::Genre) + ": " + Lang::get(Lang::New),
		this
	);

	dialog.exec();

	QString new_name = dialog.text();
	if(dialog.was_accepted() && !new_name.isEmpty())
	{
		m->genre_fetcher->create_genre(Genre(new_name));
	}
}


void GenreView::rename_pressed()
{
	QList<QTreeWidgetItem*> selected_items = this->selectedItems();
	if(selected_items.isEmpty()){
		return;
	}

	QStringList genre_list;
	{ // prepare genres for completer
		const Util::Set<Genre> genres = m->genre_fetcher->genres();

		for(Genre genre : genres)
		{
			genre_list << genre.name().trimmed();
		}

		std::sort(genre_list.begin(), genre_list.end());
	}

	// run rename dialog for each genre to rename
	for(const QTreeWidgetItem* item : selected_items)
	{
		QString item_text = item->text(0);
		Gui::LineInputDialog dialog
		(
			Lang::get(Lang::Genre),
			Lang::get(Lang::Rename) + ": " + item_text,
			item_text,
			this
		);

		dialog.set_completer_text(genre_list);
		dialog.exec();

		QString new_name = dialog.text();
		if(dialog.was_accepted() && !new_name.isEmpty())
		{
			m->genre_fetcher->rename_genre(Genre(item_text), Genre(new_name));
		}
	}
}


void GenreView::delete_pressed()
{
	QList<QTreeWidgetItem*> selected_items = this->selectedItems();
	if(selected_items.isEmpty()){
		return;
	}

	Util::Set<Genre> genres;
	QStringList genre_names;

	for(QTreeWidgetItem* twi : selected_items)
	{
		Genre g(twi->text(0));
		genres.insert(g);
		genre_names << g.name();
	}

	Message::Answer answer = Message::question_yn(
			tr("Do you really want to remove %1 from all tracks?").arg(genre_names.join(", ")),
			Lang::get(Lang::Genres)
	);

	if(answer == Message::Answer::Yes)
	{
		m->genre_fetcher->delete_genres(genres);
	}
}

void GenreView::switch_tree_list()
{
	bool show_tree = GetSetting(Set::Lib_GenreTree);
	reload_genres();

	if(!show_tree) {
		this->setIndentation(0);
	}

	else {
		this->setIndentation(m->default_indent);
	}
}

void GenreView::selection_changed(const QItemSelection& selected, const QItemSelection& deselected)
{
	Q_UNUSED(selected)
	Q_UNUSED(deselected);

	if(m->is_dragging){
		return;
	}

	QStringList genres;
	const QModelIndexList indexes = this->selectionModel()->selectedRows();
	for(const QModelIndex& index : indexes)
	{
		genres << index.data().toString();
		if(is_invalid_genre(index))
		{
			emit sig_invalid_genre_selected();
			return;
		}
	}

	emit sig_selected_changed(genres);
}


void GenreView::reload_genres()
{
	for(GenreNode* n : Algorithm::AsConst(m->genres->children))
	{
		m->genres->remove_child(n);
		delete n; n=nullptr;
	}

	this->clear();

	// fill it on next show event
	m->filled = false;

	Util::Set<Genre> genres = m->genre_fetcher->genres();
	set_genres(genres);
}

void GenreView::set_genres(const Util::Set<Genre>& genres)
{
	if(m->filled){
		return;
	}

	m->filled = true;

	this->build_genre_data_tree(genres);
	this->populate_widget(nullptr, m->genres);
}

static void build_genre_node(GenreNode* node, const QMap<QString, StringSet>& parent_nodes)
{
	QString value = node->data;
	if(!parent_nodes.contains(value)){
		return;
	}

	const StringSet& children = parent_nodes[value];
	if(children.isEmpty()){
		return;
	}

	for(const QString& str : children)
	{
		GenreNode* new_child = new GenreNode(str);
		build_genre_node( new_child, parent_nodes );
		node->add_child(new_child);
	}
}


void GenreView::build_genre_data_tree(const Util::Set<Genre>& genres)
{
	bool show_tree = GetSetting(Set::Lib_GenreTree);

	if(m->genres){
		delete m->genres;
	}

	m->genres = new GenreNode("");
	QMap<QString, StringSet> children;

	for(const Genre& genre : genres)
	{
		bool found_parent = false;

		if(show_tree)
		{
			for(const Genre& parent_genre : genres)
			{
				QString parent_name = parent_genre.name();

				if( parent_name.isEmpty() ||
					parent_genre == genre)
				{
					continue;
				}

				if( genre.name().contains(parent_name, Qt::CaseInsensitive)) {
					StringSet& child_genres = children[parent_name];
					child_genres.insert(genre.name());
					found_parent = true;
				}
			}
		}

		if(!found_parent) {
			m->genres->add_child(genre.name());
		}
	}

	for(GenreNode* base_genre : Algorithm::AsConst(m->genres->children))
	{
		build_genre_node(base_genre, children);
	}

	m->genres->sort(true);
}


void GenreView::populate_widget(QTreeWidgetItem* parent_item, GenreNode* node)
{
	QStringList text = { ::Util::cvt_str_to_first_upper(node->data) };

	bool invalid_genre = (text.size() > 0 && text.first().isEmpty());
	if(invalid_genre) {
		text = QStringList{ invalid_genre_name() };
	}

	QTreeWidgetItem* item;
	if(node->parent == m->genres) {
		item = new QTreeWidgetItem(this, text);
	}

	else {
		item = new QTreeWidgetItem(parent_item, text);
	}

	if(invalid_genre)
	{
		item->setData(0, Qt::UserRole, 5000);
		item->setFlags(item->flags() & ~Qt::ItemIsDragEnabled & ~Qt::ItemIsDropEnabled);
	}

	for(GenreNode* child : Algorithm::AsConst(node->children))
	{
		populate_widget(item, child);
	}

	if(m->expanded_items.contains(node->data, Qt::CaseInsensitive)){
		item->setExpanded(true);
	}
}

QTreeWidgetItem* GenreView::find_genre(const QString& genre)
{
	QList<QTreeWidgetItem*> items = this->findItems(genre, Qt::MatchRecursive);

	if(items.isEmpty()) {
		sp_log(Log::Warning, this) << "Could not find item " << genre;
		return nullptr;
	}

	return items.first();
}

void GenreView::init_context_menu()
{
	using Gui::ContextMenu;

	if(m->context_menu){
		return;
	}

	m->context_menu = new GenreViewContextMenu(this);
	m->context_menu->show_actions(
				ContextMenu::EntryDelete |
				ContextMenu::EntryNew |
				ContextMenu::EntryRename );

	connect( m->context_menu, &ContextMenu::sig_delete, this, &GenreView::delete_pressed);
	connect( m->context_menu, &ContextMenu::sig_rename, this, &GenreView::rename_pressed);
	connect( m->context_menu, &ContextMenu::sig_new, this, &GenreView::new_pressed);
}

void GenreView::contextMenuEvent(QContextMenuEvent* e)
{
	QModelIndexList indexes = this->selectionModel()->selectedIndexes();
	for(const QModelIndex& idx : indexes)
	{
		if(is_invalid_genre(idx))
		{
			e->ignore();
			return;
		}
	}

	init_context_menu();
	m->context_menu->exec(e->globalPos());
	QTreeView::contextMenuEvent(e);
}

void GenreView::dragEnterEvent(QDragEnterEvent* e)
{
	e->accept();
}

void GenreView::dragMoveEvent(QDragMoveEvent* e)
{
	QModelIndex idx = this->indexAt(e->pos());
	if(is_invalid_genre(idx)){
		e->ignore();
		return;
	}

	if(!idx.isValid()){
		return;
	}

	m->is_dragging=true;

	QItemSelectionModel* ism = this->selectionModel();
	ism->select(idx, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);

	e->accept();
}

void GenreView::dragLeaveEvent(QDragLeaveEvent* e)
{
	m->is_dragging = false;

	this->clearSelection();
	e->accept();
}

void GenreView::dropEvent(QDropEvent* e)
{
	m->is_dragging = false;

	QModelIndex idx = this->indexAt(e->pos());
	if(is_invalid_genre(idx)){
		e->ignore();
		return;
	}

	this->clearSelection();

	e->accept();

	auto* cmd = static_cast<const Gui::CustomMimeData*>(e->mimeData());
	if(!cmd) {
		sp_log(Log::Debug, this) << "Cannot apply genre to data";
		return;
	}

	if(!idx.isValid()){
		sp_log(Log::Debug, this) << "drop: Invalid index";
		return;
	}

	this->setAcceptDrops(false);

	Genre genre(idx.data().toString());
	MetaDataList v_md(std::move(cmd->metadata()));

	m->genre_fetcher->add_genre_to_md(v_md, genre);
}


QString GenreView::invalid_genre_name()
{
	return "<" + Lang::get(Lang::UnknownGenre) + ">";
}


void GenreView::language_changed()
{
	QAbstractItemModel* model = this->model();
	int rc = model->rowCount();
	for(int i=0; i<rc; i++)
	{
		QModelIndex idx = model->index(i, 0);
		if(is_invalid_genre(idx))
		{
			model->setData(idx, invalid_genre_name(), Qt::DisplayRole);
			break;
		}
	}
}
