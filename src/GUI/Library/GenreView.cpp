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

#include "GUI/Utils/CustomMimeData.h"
#include "GUI/Utils/Delegates/StyledItemDelegate.h"
#include "GUI/Utils/ContextMenu/ContextMenu.h"

#include "Utils/Utils.h"
#include "Utils/Tree.h"
#include "Utils/Set.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/MetaData/Genre.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Language.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Message/Message.h"

#include <QDropEvent>
#include <QContextMenuEvent>
#include <QInputDialog>
#include <QStyledItemDelegate>
#include <QTreeWidget>
#include <QShortcut>

using StringSet=Util::Set<QString>;
using namespace Library;

struct GenreView::Private
{
	QStringList				expanded_items;
	GenreFetcher*			genre_fetcher=nullptr;
	ContextMenu*			context_menu=nullptr;
	GenreNode*				genres=nullptr;
	QAction*				toggle_tree_action=nullptr;
	int						default_indent;
	bool					filled;

	Private(QWidget* parent) :
		genre_fetcher(new GenreFetcher(parent)),
		genres(new GenreNode("root")),
		filled(false)
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

	connect(this, &QTreeWidget::itemCollapsed, this, &GenreView::item_collapsed);
	connect(this, &QTreeWidget::itemExpanded, this, &GenreView::item_expanded);

	connect(m->genre_fetcher, &GenreFetcher::sig_finished, this, &GenreView::update_finished);
	connect(m->genre_fetcher, &GenreFetcher::sig_progress, this, &GenreView::progress_changed);
	connect(m->genre_fetcher, &GenreFetcher::sig_genres_fetched, this, &GenreView::reload_genres);

	ListenSettingNoCall(Set::Lib_GenreTree, GenreView::tree_action_changed);

	new QShortcut(QKeySequence(Qt::Key_Enter), this, SLOT(expand_current_item()), nullptr, Qt::WidgetShortcut);
	new QShortcut(QKeySequence(Qt::Key_Return), this, SLOT(expand_current_item()), nullptr, Qt::WidgetShortcut);
}


GenreView::~GenreView() {}


bool GenreView::has_items() const
{
	int n_rows = m->genres->children.size();
	if(n_rows == 1)
	{
		QString data = m->genres->children[0]->data;
		if(data.trimmed().isEmpty()){
			return false;
		}
	}

	return (n_rows > 0);
}

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
	bool ok;
	QString new_name = QInputDialog::getText(this,
					Lang::get(Lang::Genre),
					Lang::get(Lang::New),
					QLineEdit::Normal, QString(), &ok);

	if(ok && !new_name.isEmpty()){
		m->genre_fetcher->create_genre(Genre(new_name));
	}
}

void GenreView::rename_pressed()
{
	QList<QTreeWidgetItem*> selected_items = this->selectedItems();
	if(selected_items.isEmpty()){
		return;
	}

	for(const QTreeWidgetItem* item : selected_items)
	{
		bool ok;
		QString text = item->text(0);

		QString new_name = QInputDialog::getText(this,
						Lang::get(Lang::Genre),
						Lang::get(Lang::Rename) + " " + text + ": ",
						QLineEdit::Normal, text, &ok);
		if(ok && !new_name.isEmpty()){
			m->genre_fetcher->rename_genre(Genre(text), Genre(new_name));
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

	for(QTreeWidgetItem* twi : selected_items){
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
		for(const Genre& genre : genres)
		{
			m->genre_fetcher->delete_genre(Genre(genre));
		}
	}
}

void GenreView::tree_action_changed()
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

void GenreView::tree_action_toggled(bool b)
{
	SetSetting(Set::Lib_GenreTree, b);
}

void GenreView::language_changed()
{
	if(m->toggle_tree_action) {
		m->toggle_tree_action->setText(Lang::get(Lang::Tree));
	}
}

void GenreView::reload_genres()
{
	for(GenreNode* n : ::Util::AsConst(m->genres->children))
	{
		m->genres->remove_child(n);
		delete n; n=nullptr;
	}

	this->clear();

	// fill it on next show event
	m->filled = false;

	Util::Set<Genre> genres = m->genre_fetcher->genres();
	set_genres(genres);

	emit sig_genres_reloaded();
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

		if(genre.name().isEmpty()){
			continue;
		}

		if(show_tree)
		{
			for(const Genre& parent_genre : genres)
			{
				if( parent_genre.name().isEmpty() ||
					parent_genre == genre)
				{
					continue;
				}

				if( genre.name().contains(parent_genre.name(), Qt::CaseInsensitive) ) {
					StringSet& child_genres = children[parent_genre.name()];
					child_genres.insert(genre.name());
					found_parent = true;
				}
			}
		}

		if(!found_parent) {
			m->genres->add_child(genre.name());
		}
	}

	for(GenreNode* base_genre : ::Util::AsConst(m->genres->children))
	{
		build_genre_node(base_genre, children);
	}

	m->genres->sort(true);
}



void GenreView::populate_widget(QTreeWidgetItem* parent_item, GenreNode* node)
{
	QTreeWidgetItem* item;
	QStringList text = { ::Util::cvt_str_to_first_upper(node->data) };

	if(node->parent == m->genres){
		item = new QTreeWidgetItem(this, text);
	}

	else {
		item = new QTreeWidgetItem(parent_item, text);
	}

	for(GenreNode* child : ::Util::AsConst(node->children))
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

	if(items.isEmpty()){
		sp_log(Log::Warning, this) << "Could not find item " << genre;
		return nullptr;
	}

	return items.first();
}



void GenreView::init_context_menu()
{
	if(m->context_menu){
		return;
	}

	bool show_tree = GetSetting(Set::Lib_GenreTree);

	m->context_menu = new ContextMenu(this);
	m->toggle_tree_action = new QAction(m->context_menu);
	m->toggle_tree_action->setCheckable(true);
	m->toggle_tree_action->setChecked(show_tree);
	m->toggle_tree_action->setText(Lang::get(Lang::Tree));

	m->context_menu->show_actions(
				ContextMenu::EntryDelete |
				ContextMenu::EntryNew |
				ContextMenu::EntryRename );

	m->context_menu->register_action(m->toggle_tree_action);

	connect( m->context_menu, &ContextMenu::sig_delete, this, &GenreView::delete_pressed);
	connect( m->context_menu, &ContextMenu::sig_rename, this, &GenreView::rename_pressed);
	connect( m->context_menu, &ContextMenu::sig_new, this, &GenreView::new_pressed);

	connect( m->toggle_tree_action, &QAction::triggered, this, &GenreView::tree_action_toggled);
}


void GenreView::contextMenuEvent(QContextMenuEvent* e)
{
	init_context_menu();

	m->context_menu->exec(e->globalPos());

	QTreeView::contextMenuEvent(e);
}

void GenreView::dragEnterEvent(QDragEnterEvent *e)
{
	e->accept();
}

void GenreView::dragMoveEvent(QDragMoveEvent *e)
{
	QItemSelectionModel* ism;
	QModelIndex idx;

	idx = this->indexAt(e->pos());
	if(!idx.isValid()){
		sp_log(Log::Debug, this) << "drag: Invalid index";
		return;
	}

	ism = this->selectionModel();

	ism->select(idx, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);

	e->accept();
}

void GenreView::dragLeaveEvent(QDragLeaveEvent *e)
{
	this->clearSelection();
	e->accept();
}

void GenreView::dropEvent(QDropEvent *e)
{
	e->accept();

	this->clearSelection();

	const CustomMimeData* cmd = static_cast<const CustomMimeData*>(e->mimeData());

	if(!cmd){
		sp_log(Log::Debug, this) << "Cannot apply genre to data";
		return;
	}

	QModelIndex idx = this->indexAt(e->pos());
	if(!idx.isValid()){
		sp_log(Log::Debug, this) << "drop: Invalid index";
		return;
	}

	this->setAcceptDrops(false);

	Genre genre(idx.data().toString());
	MetaDataList v_md(std::move(cmd->metadata()));

	m->genre_fetcher->add_genre_to_md(v_md, genre);
}
