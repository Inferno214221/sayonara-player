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
#include "Components/Library/GenreTreeBuilder.h"

#include "Gui/Utils/GuiUtils.h"
#include "Gui/Library/Utils/GenreViewContextMenu.h"
#include "Gui/Utils/MimeData/CustomMimeData.h"
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
#include <QTreeWidget>
#include <QShortcut>

using Library::GenreView;
using Library::GenreTreeItem;

namespace
{
	using StringSet = Util::Set<QString>;
	using GenreTreeBuilder::GenreNode;

	enum DataRole
	{
		InvalidGenreRole = Qt::UserRole
	};

	bool isInvalidGenre(const QModelIndex& index)
	{
		return index.data(InvalidGenreRole).toBool();
	}

	QStringList getGenreNames(const Util::Set<Genre>& genres)
	{
		QStringList result;
		Util::Algorithm::transform(genres, result, [](const auto& genre) {
			return genre.name().trimmed();
		});

		result.sort();

		return result;
	}

	// NOLINTNEXTLINE(*-no-recursion)
	void populateWidget(QTreeWidget* tree, QTreeWidgetItem* parentItem, GenreNode* parentNode,
	                    const QStringList& expandedItems)
	{
		for(auto* node: parentNode->children)
		{
			const auto isInvalidGenre = node->data.isEmpty();
			const auto text = isInvalidGenre
			                  ? ::Library::GenreView::invalidGenreName()
			                  : node->data;

			auto* item = (parentItem != nullptr)
			             ? new GenreTreeItem(parentItem, {text})
			             : new GenreTreeItem(tree, {text}, isInvalidGenre);

			populateWidget(tree, item, node, expandedItems);

			if(expandedItems.contains(node->data))
			{
				item->setExpanded(true);
			}
		}
	}
}

struct GenreView::Private
{
	QStringList expandedItems;
	GenreFetcher* genreFetcher;
	GenreViewContextMenu* contextMenu = nullptr;
	GenreNode* genres {GenreTreeBuilder::buildGenreDataTree({}, true)};

	int defaultIndent;
	bool filled {false};
	bool isDragging {false};

	explicit Private(QTreeWidget* parent) :
		genreFetcher(new GenreFetcher(parent)),
		defaultIndent {parent->indentation()} {}
};

GenreView::GenreView(QWidget* parent) :
	WidgetTemplate<QTreeWidget>(parent),
	m {Pimpl::make<Private>(this)}
{
	setAcceptDrops(true);
	setDragDropMode(GenreView::DragDrop);
	setAlternatingRowColors(true);
	setItemDelegate(new Gui::StyledItemDelegate(this));

	connect(this, &QTreeWidget::itemCollapsed, this, [this](auto* item) { m->expandedItems.removeAll(item->text(0)); });
	connect(this, &QTreeWidget::itemExpanded, this, [this](auto* item) { m->expandedItems << item->text(0); });

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
	m->genreFetcher->setLocalLibrary(library);
}

void GenreView::progressChanged(int progress)
{
	emit sigProgress(tr("Updating genres"), progress);
}

void GenreView::updateFinished()
{
	setAcceptDrops(true);
	emit sigProgress("", -1);
}

void GenreView::expandCurrentItem()
{
	if(auto* item = currentItem(); item)
	{
		if(item->isExpanded() || item->childCount() == 0)
		{
			emit activated(currentIndex());
		}
		else
		{
			item->setExpanded(true);
		}
	}
}

void GenreView::newPressed()
{
	auto dialog = Gui::LineInputDialog(Lang::get(Lang::Genre),
	                                   Lang::get(Lang::Genre) + ": " + Lang::get(Lang::New),
	                                   this);

	dialog.exec();

	const auto newName = dialog.text();
	if(dialog.wasAccepted() && !newName.isEmpty())
	{
		m->genreFetcher->createGenre(Genre {newName});
	}
}

void GenreView::renamePressed()
{
	const auto selection = selectedItems();
	if(selection.isEmpty())
	{
		return;
	}

	const auto genreNames = getGenreNames(m->genreFetcher->genres());
	for(const auto* item: selection)
	{
		const auto itemText = item->text(0);
		auto dialog = Gui::LineInputDialog(Lang::get(Lang::Genre),
		                                   Lang::get(Lang::Rename) + ": " + itemText,
		                                   itemText, this);

		dialog.setCompleterText(genreNames);
		dialog.exec();

		const auto newName = dialog.text();
		if(dialog.wasAccepted() && !newName.isEmpty())
		{
			m->genreFetcher->renameGenre(Genre(itemText), Genre(newName));
		}
	}
}

void GenreView::deletePressed()
{
	const auto selection = selectedItems();
	if(selection.isEmpty())
	{
		return;
	}

	auto genres = Util::Set<Genre> {};
	auto genreNames = QStringList {};

	for(auto* treeWidgetItem: selection)
	{
		const auto genre = Genre(treeWidgetItem->text(0));
		genres.insert(genre);
		genreNames << genre.name();
	}

	const auto answer = Message::question_yn(
		tr("Do you really want to remove %1 from all tracks?").arg(genreNames.join(", ")),
		Lang::get(Lang::Genres)
	);

	if(answer == Message::Answer::Yes)
	{
		m->genreFetcher->deleteGenres(genres);
	}
}

void GenreView::switchTreeList()
{
	const auto showTree = GetSetting(Set::Lib_GenreTree);
	reloadGenres();

	const auto indentation = showTree ? m->defaultIndent : 0;
	setIndentation(indentation);
}

void GenreView::selectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
	Gui::WidgetTemplate<QTreeWidget>::selectionChanged(selected, deselected);

	if(m->isDragging)
	{
		return;
	}

	auto genreNames = QStringList {};
	const auto indexes = selectionModel()->selectedRows();
	for(const auto& index: indexes)
	{
		genreNames << index.data().toString();
		if(isInvalidGenre(index))
		{
			emit sigInvalidGenreSelected();
			return;
		}
	}

	emit sigSelectedChanged(genreNames);
}

void GenreView::reloadGenres()
{
	for(const auto& genreNode: m->genres->children)
	{
		auto* node = m->genres->removeChild(genreNode);
		delete node;
	}

	clear();

	// fill it on next show event
	m->filled = false;
	setGenres(m->genreFetcher->genres());
}

void GenreView::setGenres(const Util::Set<Genre>& genres)
{
	if(m->filled)
	{
		return;
	}

	const auto it = Util::Algorithm::find(genres, [](const auto& genre) {
		return genre.name().toLower().contains("nu");
	});

	if(it != genres.end())
	{
		spLog(Log::Info, this) << "Found it";
	}

	m->filled = true;

	delete m->genres;
	m->genres = GenreTreeBuilder::buildGenreDataTree(genres, GetSetting(Set::Lib_GenreTree));

	populateWidget(this, nullptr, m->genres, m->expandedItems);
}

[[maybe_unused]] QTreeWidgetItem* GenreView::findGenre(const QString& genre)
{
	const auto items = this->findItems(genre, Qt::MatchRecursive);
	if(items.isEmpty())
	{
		spLog(Log::Warning, this) << "Could not find item " << genre;
		return nullptr;
	}

	return items.first();
}

void GenreView::initContextMenu()
{
	using Gui::ContextMenu;

	if(m->contextMenu)
	{
		return;
	}

	m->contextMenu = new GenreViewContextMenu(this);
	m->contextMenu->showActions(
		ContextMenu::EntryDelete |
		ContextMenu::EntryNew |
		ContextMenu::EntryRename);

	connect(m->contextMenu, &ContextMenu::sigDelete, this, &GenreView::deletePressed);
	connect(m->contextMenu, &ContextMenu::sigRename, this, &GenreView::renamePressed);
	connect(m->contextMenu, &ContextMenu::sigNew, this, &GenreView::newPressed);
}

void GenreView::contextMenuEvent(QContextMenuEvent* e)
{
	const auto indexes = selectionModel()->selectedIndexes();
	for(const auto& index: indexes)
	{
		if(isInvalidGenre(index))
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
	const auto index = this->indexAt(e->pos());
	if(isInvalidGenre(index))
	{
		e->ignore();
		return;
	}

	if(index.isValid())
	{
		m->isDragging = true;
		selectionModel()->select(index, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
		e->accept();
	}
}

void GenreView::dragLeaveEvent(QDragLeaveEvent* e)
{
	m->isDragging = false;

	clearSelection();
	e->accept();
}

void GenreView::dropEvent(QDropEvent* e)
{
	m->isDragging = false;

	const auto index = indexAt(e->pos());
	if(isInvalidGenre(index))
	{
		e->ignore();
		return;
	}

	clearSelection();
	e->accept();

	const auto* cmd = dynamic_cast<const Gui::CustomMimeData*>(e->mimeData());
	if(!cmd)
	{
		spLog(Log::Debug, this) << "Cannot apply genre to data";
		return;
	}

	if(!index.isValid())
	{
		spLog(Log::Debug, this) << "drop: Invalid index";
		return;
	}

	setAcceptDrops(false);

	const auto genre = Genre {index.data().toString()};
	m->genreFetcher->applyGenreToMetadata(cmd->metadata(), genre);
}

QString GenreView::invalidGenreName() { return "<" + Lang::get(Lang::UnknownGenre) + ">"; }

void GenreView::languageChanged()
{
	auto* model = this->model();
	const auto rows = model->rowCount();
	for(int i = 0; i < rows; i++)
	{
		const auto index = model->index(i, 0);
		if(isInvalidGenre(index))
		{
			model->setData(index, invalidGenreName(), Qt::DisplayRole);
			break;
		}
	}
}

GenreTreeItem::GenreTreeItem(QTreeWidgetItem* parent, const QStringList& text) :
	QTreeWidgetItem(parent, text)
{
	setInvalidGenre(false);
}

GenreTreeItem::GenreTreeItem(QTreeWidget* parent, const QStringList& text, bool invalidGenre) :
	QTreeWidgetItem(parent, text)
{
	setInvalidGenre(invalidGenre);
}

void GenreTreeItem::setInvalidGenre(const bool b)
{
	setData(0, InvalidGenreRole, b);

	if(b)
	{
		setFlags(flags() &
		         ~Qt::ItemIsDragEnabled &
		         ~Qt::ItemIsDropEnabled);
	}
}
