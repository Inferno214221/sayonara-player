/* LibraryView.cpp */

/* Copyright (C) 2011-2024 Michael Lugmair (Lucio Carreras)
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

/*
 *  Created on: Jun 26, 2011
 *      Author: Michael Lugmair (Lucio Carreras)
 */

#include "ItemView.h"
#include "ItemModel.h"
#include "Gui/Library/Utils/MergeMenu.h"

#include "Components/Library/AbstractLibrary.h"
#include "Components/Covers/CoverLocation.h"

#include "Utils/Library/MergeData.h"
#include "Utils/Settings/Settings.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Set.h"
#include "Utils/FileUtils.h"
#include "Utils/Language/Language.h"
#include "Utils/ExtensionSet.h"

#include "Gui/Library/Header/LibraryHeaderView.h"
#include "Gui/Utils/ContextMenu/LibraryContextMenu.h"
#include "Gui/Utils/PreferenceAction.h"
#include "Gui/Utils/Shortcuts/ShortcutHandler.h"
#include "Gui/Utils/Shortcuts/Shortcut.h"
#include "Gui/Utils/MimeData/CustomMimeData.h"
#include "Gui/Utils/Icons.h"

#include <QKeySequence>
#include <QDrag>
#include <QPushButton>
#include <QItemSelectionModel>

using Library::ItemView;
using Library::ItemModel;
using Library::ContextMenu;

struct ItemView::Private
{
	Gui::MergeMenu* mergeMenu = nullptr;
	ItemModel* model = nullptr;
	QPushButton* buttonClearSelection = nullptr;
	ContextMenu* contextMenu = nullptr;

	MD::Interpretation type {MD::Interpretation::None};
	bool currentlyFilling {false};
	bool useClearButton {false};
};

ItemView::ItemView(QWidget* parent) :
	SearchableTableView(parent),
	InfoDialogContainer(),
	Gui::Dragable(this)
{
	m = Pimpl::make<Private>();

	this->setAcceptDrops(true);
	this->setSelectionBehavior(QAbstractItemView::SelectRows);
	this->setAlternatingRowColors(true);

	clearSelection();

	const auto slotMap = std::array {
		std::pair {ShortcutIdentifier::PlayNewTab, SLOT(playNewTabClicked())},
		std::pair {ShortcutIdentifier::PlayNext, SLOT(playNextClicked())},
		std::pair {ShortcutIdentifier::Append, SLOT(appendClicked())},
		std::pair {ShortcutIdentifier::AlbumArtists, SLOT(albumArtistsToggled())},
		std::pair {ShortcutIdentifier::ReloadLibrary, SLOT(reloadClicked())}
	};

	auto* shortcutHandler = ShortcutHandler::instance();
	for(const auto& entry: slotMap)
	{
		shortcutHandler->shortcut(entry.first).connect(this, this, entry.second, Qt::WidgetWithChildrenShortcut);
	}

	new QShortcut(QKeySequence(Qt::Key_Return), this, SLOT(playClicked()), nullptr, Qt::WidgetShortcut);
	new QShortcut(QKeySequence(Qt::Key_Enter), this, SLOT(playClicked()), nullptr, Qt::WidgetShortcut);
	new QShortcut(QKeySequence(Qt::Key_Backspace), this, SLOT(clearSelection()), nullptr, Qt::WidgetShortcut);

	connect(this, &QAbstractItemView::doubleClicked, this, &ItemView::playClicked);
}

ItemView::~ItemView() = default;

AbstractLibrary* ItemView::library() const { return nullptr; }

ItemModel* ItemView::itemModel() const { return m->model; }

void ItemView::setItemModel(ItemModel* model)
{
	m->model = model;

	SearchableTableView::setSearchableModel(model);
}

ContextMenu::Entries ItemView::contextMenuEntries() const
{
	return (ContextMenu::EntryPlay |
	        ContextMenu::EntryPlayNewTab |
	        ContextMenu::EntryInfo |
	        ContextMenu::EntryEdit |
	        ContextMenu::EntryDelete |
	        ContextMenu::EntryPlayNext |
	        ContextMenu::EntryAppend |
	        ContextMenu::EntryReload |
	        ContextMenu::EntryViewType);
}

void ItemView::selectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
	SearchableTableView::selectionChanged(selected, deselected);

	showClearButton(!selected.empty());

	if(!m->currentlyFilling)
	{
		selectedItemsChanged(selectedItems());
	}
}

void ItemView::initContextMenu()
{
	initCustomContextMenu(nullptr);
}

// Right click stuff
void ItemView::initCustomContextMenu(ContextMenu* menu)
{
	if(m->contextMenu)
	{
		return;
	}

	m->contextMenu = (menu != nullptr)
	                 ? menu
	                 : new ContextMenu(this);

	if(!m->mergeMenu)
	{
		m->mergeMenu = new Gui::MergeMenu(m->contextMenu);
		connect(m->mergeMenu, &Gui::MergeMenu::sigMergeTriggered, this, &ItemView::mergeActionTriggered);
	}

	auto* afterEditAction = m->contextMenu->actionAfter(ContextMenu::EntryEdit);
	if(afterEditAction)
	{
		m->contextMenu->insertAction(afterEditAction, m->mergeMenu->action());
	}
	connect(m->contextMenu->action(ContextMenu::EntryEdit), &QAction::triggered, this, [&]() { showEdit(); });
	connect(m->contextMenu->action(ContextMenu::EntryInfo), &QAction::triggered, this, [&]() { showInfo(); });
	connect(m->contextMenu->action(ContextMenu::EntryLyrics), &QAction::triggered, this, [&]() { showLyrics(); });
	connect(m->contextMenu->action(ContextMenu::EntryDelete), &QAction::triggered, this, &ItemView::deleteClicked);
	connect(m->contextMenu->action(ContextMenu::EntryPlay), &QAction::triggered, this, &ItemView::playClicked);
	connect(m->contextMenu->action(ContextMenu::EntryPlayNext), &QAction::triggered, this, &ItemView::playNextClicked);
	connect(m->contextMenu->action(ContextMenu::EntryPlayNewTab), &QAction::triggered,
	        this, &ItemView::playNewTabClicked);
	connect(m->contextMenu->action(ContextMenu::EntryAppend), &QAction::triggered, this, &ItemView::appendClicked);
	connect(m->contextMenu->action(ContextMenu::EntryRefresh), &QAction::triggered, this, &ItemView::refreshClicked);
	connect(m->contextMenu->action(ContextMenu::EntryReload), &QAction::triggered, this, &ItemView::reloadClicked);
	connect(m->contextMenu, &ContextMenu::sigFilterTriggered, this, &ItemView::filterExtensionsTriggered);

	this->showContextMenuActions(contextMenuEntries());

	m->contextMenu->addPreferenceAction(new Gui::LibraryPreferenceAction(m->contextMenu));
	m->contextMenu->setExtensions(library()->extensions());
}

ContextMenu* ItemView::contextMenu() const { return m->contextMenu; }

void ItemView::showContextMenu(const QPoint& p)
{
	m->contextMenu->exec(p);
}

void ItemView::showContextMenuActions(ContextMenu::Entries entries)
{
	m->contextMenu->showActions(entries);
}

void ItemView::showClearButton(bool visible)
{
	if(!m->useClearButton)
	{
		return;
	}

	if(!m->buttonClearSelection)
	{
		m->buttonClearSelection = new QPushButton(this);

		connect(m->buttonClearSelection, &QPushButton::clicked, this, [=]() {
			this->clearSelection();
			m->buttonClearSelection->hide();
		});
	}

	m->buttonClearSelection->setText(Lang::get(Lang::ClearSelection));
	m->buttonClearSelection->setIcon(Gui::Icons::icon(Gui::Icons::Delete));

	{ // little hack to use vieport_height() and ..width() method
		m->buttonClearSelection->setVisible(false);
	}

	m->buttonClearSelection->setGeometry(
		1, viewportHeight() - m->buttonClearSelection->height() - 2,
		viewportWidth() - 2, m->buttonClearSelection->height()
	);

	m->buttonClearSelection->setVisible(visible);
}

void ItemView::useClearButton(bool b)
{
	m->useClearButton = b;
	if(m->buttonClearSelection)
	{
		m->buttonClearSelection->setVisible(b && !selectedItems().isEmpty());
	}
}

bool ItemView::isValidDragPosition(const QPoint& p) const
{
	const auto modelIndex = this->indexAt(p);
	return modelIndex.isValid() &&
	       (model()->flags(modelIndex) & Qt::ItemFlag::ItemIsSelectable);
}

MetaDataList ItemView::infoDialogData() const { return itemModel()->selectedMetadata(); }

QWidget* ItemView::getParentWidget() { return this; }

void ItemView::mergeActionTriggered()
{
	if(const auto mergedata = m->mergeMenu->mergedata(); mergedata.isValid())
	{
		runMergeOperation(mergedata);
	}
}

void ItemView::runMergeOperation(const Library::MergeData& /*md*/) {}

void ItemView::playClicked() { emit sigPlayClicked(); }

void ItemView::playNewTabClicked() { emit sigPlayNewTabClicked(); }

void ItemView::playNextClicked() { emit sigPlayNextClicked(); }

void ItemView::deleteClicked() { emit sigDeleteClicked(); }

void ItemView::appendClicked() { emit sigAppendClicked(); }

void ItemView::refreshClicked() { emit sigRefreshClicked(); }

void ItemView::reloadClicked() { emit sigReloadClicked(); }

void ItemView::albumArtistsToggled()
{
	const auto b = GetSetting(Set::Lib_ShowAlbumArtists);
	SetSetting(Set::Lib_ShowAlbumArtists, !b);
}

void ItemView::filterExtensionsTriggered(const QString& extension, const bool b)
{
	auto* library = this->library();
	if(library)
	{
		Gui::ExtensionSet extensions = library->extensions();
		extensions.setEnabled(extension, b);
		library->setExtensions(extensions);
	}
}

void ItemView::fill()
{
	this->clearSelection();

	int oldSize, newSize;
	m->model->refreshData(&oldSize, &newSize);
}

void ItemView::selectedItemsChanged(const IndexSet& indexes)
{
	emit sigSelectionChanged(indexes);
}

void ItemView::importRequested(const QStringList& files)
{
	if(library())
	{
		library()->importFiles(files);
	}
}

void ItemView::mousePressEvent(QMouseEvent* event)
{
	if(rowCount() == 0)
	{
		return;
	}

	SearchableTableView::mousePressEvent(event);

	if(event->button() == Qt::MiddleButton)
	{
		if(!selectedItems().isEmpty())
		{
			playNewTabClicked();
		}
	}
}

void ItemView::contextMenuEvent(QContextMenuEvent* event)
{
	if(!m->contextMenu)
	{
		initContextMenu();
	}

	const auto globalPos = event->globalPos();
	const auto selections = selectedItems();

	const auto lyricsVisible = (metadataInterpretation() == MD::Interpretation::Tracks) && (selections.size() == 1);
	m->contextMenu->showAction(ContextMenu::EntryLyrics, lyricsVisible);

	if(isMergeable())
	{
		QMap<Id, QString> data;
		auto* model = itemModel();
		for(const auto& selectedIndex: selections)
		{
			const auto id = model->mapIndexToId(selectedIndex);
			auto name = model->searchableString(selectedIndex);
			name.replace("&", "&&");

			data.insert(id, name);
		}

		m->mergeMenu->setData(data);
		m->mergeMenu->action()->setVisible(m->mergeMenu->isDataValid());
	}

	m->contextMenu->setExtensions(library()->extensions());
	m->contextMenu->setSelectionCount(selections.count());

	showContextMenu(globalPos);
	QTableView::contextMenuEvent(event);
}

void ItemView::dragEnterEvent(QDragEnterEvent* event) { event->accept(); }

void ItemView::dragMoveEvent(QDragMoveEvent* event) { event->accept(); }

void ItemView::dropEvent(QDropEvent* event)
{
	event->accept();

	const auto* mimedata = event->mimeData();
	if(!mimedata)
	{
		return;
	}

	const auto text = mimedata->hasText()
	                  ? mimedata->text()
	                  : QString {};

	// extern drops
	if(!mimedata->hasUrls() || (text.toLower() == "tracks"))
	{
		return;
	}

	QStringList filelist;
	const auto urls = mimedata->urls();
	for(const auto& url: urls)
	{
		const auto path = url.path();

		if(::Util::File::exists(path))
		{
			filelist << path;
		}
	}

	importRequested(filelist);
}

void ItemView::resizeEvent(QResizeEvent* event)
{
	SearchableTableView::resizeEvent(event);

	if(m->buttonClearSelection)
	{
		showClearButton(m->buttonClearSelection->isVisible());
	}
}

int ItemView::viewportHeight() const
{
	const auto viewportHeight = SearchableTableView::viewportHeight();

	if(m->buttonClearSelection && m->buttonClearSelection->isVisible())
	{
		return viewportHeight - (m->buttonClearSelection->height() + 5);
	}

	return viewportHeight;
}
