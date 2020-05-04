/* LibraryGenreView.cpp */

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
using Library::GenreView;

static bool is_invalid_genre(const QModelIndex& index)
{
	return (index.data(Qt::UserRole) == 5000);
}

struct GenreView::Private
{
	QStringList				expandedItems;
	GenreFetcher*			genreFetcher=nullptr;
	GenreViewContextMenu*	contextMenu=nullptr;
	GenreNode*				genres=nullptr;

	int						defaultIndent;
	bool					filled;
	bool					isDragging;

	Private(QWidget* parent) :
		genreFetcher(new GenreFetcher(parent)),
		genres(new GenreNode("root")),
		filled(false),
		isDragging(false)
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
	m->defaultIndent = this->indentation();

	setAcceptDrops(true);
	setDragDropMode(GenreView::DragDrop);
	setAlternatingRowColors(true);
	setItemDelegate(new Gui::StyledItemDelegate(this));

	connect(this, &QTreeWidget::itemCollapsed, this, &GenreView::itemCollapsed);
	connect(this, &QTreeWidget::itemExpanded, this, &GenreView::itemExpanded);

	connect(m->genreFetcher, &GenreFetcher::sigFinished, this, &GenreView::updateFinished);
	connect(m->genreFetcher, &GenreFetcher::sigProgress, this, &GenreView::progressChanged);
	connect(m->genreFetcher, &GenreFetcher::sigGenresFetched, this, &GenreView::reloadGenres);

	ListenSettingNoCall(Set::Lib_GenreTree, GenreView::switchTreeList);

	new QShortcut(QKeySequence(Qt::Key_Enter), this, SLOT(expandCurrentItem()), nullptr, Qt::WidgetShortcut);
	new QShortcut(QKeySequence(Qt::Key_Return), this, SLOT(expandCurrentItem()), nullptr, Qt::WidgetShortcut);
	new QShortcut(QKeySequence(Qt::Key_F2), this, SLOT(renamePressed()), nullptr, Qt::WidgetShortcut);
}

GenreView::~GenreView() = default;

void GenreView::init(LocalLibrary* library)
{
	m->genreFetcher->set_local_library(library);
}

void GenreView::progressChanged(int progress)
{
	emit sigProgress(tr("Updating genres"), progress);
}

void GenreView::updateFinished()
{
	this->setAcceptDrops(true);
	emit sigProgress("", -1);
}

void GenreView::itemExpanded(QTreeWidgetItem* item)
{
	m->expandedItems << item->text(0);
}

void GenreView::itemCollapsed(QTreeWidgetItem* item)
{
	m->expandedItems.removeAll(item->text(0));
}

void GenreView::expandCurrentItem()
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

void GenreView::newPressed()
{
	Gui::LineInputDialog dialog
	(
		Lang::get(Lang::Genre),
		Lang::get(Lang::Genre) + ": " + Lang::get(Lang::New),
		this
	);

	dialog.exec();

	QString new_name = dialog.text();
	if(dialog.wasAccepted() && !new_name.isEmpty())
	{
		m->genreFetcher->createGenre(Genre(new_name));
	}
}

void GenreView::renamePressed()
{
	QList<QTreeWidgetItem*> selected_items = this->selectedItems();
	if(selected_items.isEmpty()){
		return;
	}

	QStringList genreList;
	{ // prepare genres for completer
		const Util::Set<Genre> genres = m->genreFetcher->genres();

		for(Genre genre : genres)
		{
			genreList << genre.name().trimmed();
		}

		std::sort(genreList.begin(), genreList.end());
	}

	// run rename dialog for each genre to rename
	for(const QTreeWidgetItem* item : selected_items)
	{
		QString itemText = item->text(0);
		Gui::LineInputDialog dialog
		(
			Lang::get(Lang::Genre),
			Lang::get(Lang::Rename) + ": " + itemText,
			itemText,
			this
		);

		dialog.setCompleterText(genreList);
		dialog.exec();

		QString newName = dialog.text();
		if(dialog.wasAccepted() && !newName.isEmpty())
		{
			m->genreFetcher->renameGenre(Genre(itemText), Genre(newName));
		}
	}
}


void GenreView::deletePressed()
{
	QList<QTreeWidgetItem*> selectedItems = this->selectedItems();
	if(selectedItems.isEmpty()){
		return;
	}

	Util::Set<Genre> genres;
	QStringList genre_names;

	for(QTreeWidgetItem* twi : selectedItems)
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
		m->genreFetcher->deleteGenres(genres);
	}
}

void GenreView::switchTreeList()
{
	bool showTree = GetSetting(Set::Lib_GenreTree);
	reloadGenres();

	if(!showTree) {
		this->setIndentation(0);
	}

	else {
		this->setIndentation(m->defaultIndent);
	}
}

void GenreView::selectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
	Gui::WidgetTemplate<QTreeWidget>::selectionChanged(selected, deselected);

	if(m->isDragging){
		return;
	}

	QStringList genres;
	const QModelIndexList indexes = this->selectionModel()->selectedRows();
	for(const QModelIndex& index : indexes)
	{
		genres << index.data().toString();
		if(is_invalid_genre(index))
		{
			emit sigInvalidGenreSelected();
			return;
		}
	}

	emit sigSelectedChanged(genres);
}


void GenreView::reloadGenres()
{
	for(GenreNode* n : Algorithm::AsConst(m->genres->children))
	{
		m->genres->removeChild(n);
		delete n; n=nullptr;
	}

	this->clear();

	// fill it on next show event
	m->filled = false;

	Util::Set<Genre> genres = m->genreFetcher->genres();
	setGenres(genres);
}

void GenreView::setGenres(const Util::Set<Genre>& genres)
{
	if(m->filled){
		return;
	}

	m->filled = true;

	this->buildGenreDataTree(genres);
	this->populateWidget(nullptr, m->genres);
}

static void buildGenreNode(GenreNode* node, const QMap<QString, StringSet>& parentNodes)
{
	QString value = node->data;
	if(!parentNodes.contains(value)){
		return;
	}

	const StringSet& children = parentNodes[value];
	if(children.isEmpty()){
		return;
	}

	for(const QString& str : children)
	{
		GenreNode* newChild = new GenreNode(str);
		buildGenreNode(newChild, parentNodes);
		node->addChild(newChild);
	}
}


void GenreView::buildGenreDataTree(const Util::Set<Genre>& genres)
{
	bool showTree = GetSetting(Set::Lib_GenreTree);

	if(m->genres){
		delete m->genres;
	}

	m->genres = new GenreNode("");
	QMap<QString, StringSet> children;

	for(const Genre& genre : genres)
	{
		bool foundParent = false;

		if(showTree)
		{
			for(const Genre& parentGenre : genres)
			{
				const QString parentName = parentGenre.name();
				if(parentName.isEmpty() || parentGenre == genre)
				{
					continue;
				}

				if(genre.name().contains(parentName)) {
					StringSet& childGenres = children[parentName];
					childGenres.insert(genre.name());
					foundParent = true;
				}
			}
		}

		if(!foundParent) {
			m->genres->addChild(genre.name());
		}
	}

	for(GenreNode* baseGenre : Algorithm::AsConst(m->genres->children))
	{
		buildGenreNode(baseGenre, children);
	}

	m->genres->sort(true);
}


void GenreView::populateWidget(QTreeWidgetItem* parentItem, GenreNode* node)
{
	QStringList text = { node->data };

	bool invalidGenre = (!text.isEmpty() && text.first().isEmpty());
	if(invalidGenre) {
		text = QStringList{ invalidGenreName() };
	}

	QTreeWidgetItem* item;
	if(node->parent == m->genres) {
		item = new QTreeWidgetItem(this, text);
	}

	else {
		item = new QTreeWidgetItem(parentItem, text);
	}

	if(invalidGenre)
	{
		item->setData(0, Qt::UserRole, 5000);
		item->setFlags(item->flags() & ~Qt::ItemIsDragEnabled & ~Qt::ItemIsDropEnabled);
	}

	for(GenreNode* child : Algorithm::AsConst(node->children))
	{
		populateWidget(item, child);
	}

	if(m->expandedItems.contains(node->data)){
		item->setExpanded(true);
	}
}

QTreeWidgetItem* GenreView::findGenre(const QString& genre)
{
	QList<QTreeWidgetItem*> items = this->findItems(genre, Qt::MatchRecursive);

	if(items.isEmpty()) {
		spLog(Log::Warning, this) << "Could not find item " << genre;
		return nullptr;
	}

	return items.first();
}

void GenreView::initContextMenu()
{
	using Gui::ContextMenu;

	if(m->contextMenu){
		return;
	}

	m->contextMenu = new GenreViewContextMenu(this);
	m->contextMenu->showActions(
				ContextMenu::EntryDelete |
				ContextMenu::EntryNew |
				ContextMenu::EntryRename );

	connect( m->contextMenu, &ContextMenu::sigDelete, this, &GenreView::deletePressed);
	connect( m->contextMenu, &ContextMenu::sigRename, this, &GenreView::renamePressed);
	connect( m->contextMenu, &ContextMenu::sigNew, this, &GenreView::newPressed);
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

	initContextMenu();
	m->contextMenu->exec(e->globalPos());
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

	m->isDragging=true;

	QItemSelectionModel* ism = this->selectionModel();
	ism->select(idx, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);

	e->accept();
}

void GenreView::dragLeaveEvent(QDragLeaveEvent* e)
{
	m->isDragging = false;

	this->clearSelection();
	e->accept();
}

void GenreView::dropEvent(QDropEvent* e)
{
	m->isDragging = false;

	QModelIndex idx = this->indexAt(e->pos());
	if(is_invalid_genre(idx)){
		e->ignore();
		return;
	}

	this->clearSelection();

	e->accept();

	auto* cmd = static_cast<const Gui::CustomMimeData*>(e->mimeData());
	if(!cmd) {
		spLog(Log::Debug, this) << "Cannot apply genre to data";
		return;
	}

	if(!idx.isValid()){
		spLog(Log::Debug, this) << "drop: Invalid index";
		return;
	}

	this->setAcceptDrops(false);

	Genre genre(idx.data().toString());
	MetaDataList v_md(std::move(cmd->metadata()));

	m->genreFetcher->applyGenreToMetadata(v_md, genre);
}


QString GenreView::invalidGenreName()
{
	return "<" + Lang::get(Lang::UnknownGenre) + ">";
}

void GenreView::skinChanged()
{
	QFontMetrics fm = this->fontMetrics();
	this->setIconSize(QSize(fm.height(), fm.height()));
	this->setIndentation(fm.height());
}

void GenreView::languageChanged()
{
	QAbstractItemModel* model = this->model();
	int rc = model->rowCount();
	for(int i=0; i<rc; i++)
	{
		QModelIndex idx = model->index(i, 0);
		if(is_invalid_genre(idx))
		{
			model->setData(idx, invalidGenreName(), Qt::DisplayRole);
			break;
		}
	}
}
