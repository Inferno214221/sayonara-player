/* LibraryView.cpp */

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

/*
 *  Created on: Jun 26, 2011
 *      Author: Michael Lugmair (Lucio Carreras)
 */

#include "ItemView.h"
#include "ItemModel.h"

#include "Components/Library/AbstractLibrary.h"
#include "Components/Covers/CoverLocation.h"

#include "Utils/Library/MergeData.h"
#include "Utils/globals.h"
#include "Utils/Settings/Settings.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Set.h"
#include "Utils/FileUtils.h"
#include "Utils/Language/Language.h"
#include "Utils/Logger/Logger.h"
#include "Utils/ExtensionSet.h"

#include "Gui/Library/Header/HeaderView.h"
#include "Gui/Utils/ContextMenu/LibraryContextMenu.h"
#include "Gui/Utils/PreferenceAction.h"
#include "Gui/Utils/Shortcuts/ShortcutHandler.h"
#include "Gui/Utils/Shortcuts/Shortcut.h"
#include "Gui/Utils/CustomMimeData.h"
#include "Gui/Utils/Library/MergeMenu.h"
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
	Gui::MergeMenu*		mergeMenu=nullptr;
	ItemModel*			model=nullptr;
	QPushButton*		buttonClearSelection=nullptr;
	ContextMenu*		contextMenu=nullptr;

	MD::Interpretation	type;
	bool				currentlyFilling;
	bool				useClearButton;

	Private() :
		type(MD::Interpretation::None),
		currentlyFilling(false),
		useClearButton(false)
	{}
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

	QHeaderView* vertical_header = this->verticalHeader();
	if(vertical_header) {
		vertical_header->setResizeContentsPrecision(2);
	}

	clearSelection();

	Qt::ShortcutContext ctx = Qt::WidgetWithChildrenShortcut;

	auto* sch = ShortcutHandler::instance();
	sch->shortcut(ShortcutIdentifier::PlayNewTab).connect(this, this, SLOT(playNewTabClicked()), ctx);
	sch->shortcut(ShortcutIdentifier::PlayNext).connect(this, this, SLOT(playNextClicked()), ctx);
	sch->shortcut(ShortcutIdentifier::Append).connect(this, this, SLOT(appendClicked()), ctx);
	sch->shortcut(ShortcutIdentifier::CoverView).connect(this, this, SLOT(viewTypeTriggered()), ctx);
	sch->shortcut(ShortcutIdentifier::AlbumArtists).connect(this, this, SLOT(albumArtistsToggled()), ctx);
	sch->shortcut(ShortcutIdentifier::ReloadLibrary).connect(this, this, SLOT(reloadClicked()), ctx);

	new QShortcut(QKeySequence(Qt::Key_Return), this, SLOT(playClicked()), nullptr, Qt::WidgetShortcut);
	new QShortcut(QKeySequence(Qt::Key_Enter), this, SLOT(playClicked()), nullptr, Qt::WidgetShortcut);
	new QShortcut(QKeySequence(Qt::Key_Backspace), this, SLOT(clearSelection()), nullptr, Qt::WidgetShortcut);
}

ItemView::~ItemView() = default;

AbstractLibrary* ItemView::library() const { return nullptr; }

ItemModel* ItemView::itemModel() const
{
	return m->model;
}

void ItemView::setItemModel(ItemModel* model)
{
	m->model = model;

	SearchableTableView::setSearchableModel(model);
}

ContextMenu::Entries ItemView::contextMenuEntries() const
{
	ContextMenu::Entries entries =
	(
		ContextMenu::EntryPlay |
		ContextMenu::EntryPlayNewTab |
		ContextMenu::EntryInfo |
		ContextMenu::EntryEdit |
		ContextMenu::EntryDelete |
		ContextMenu::EntryPlayNext |
		ContextMenu::EntryAppend |
//		ContextMenu::EntryStandardView |
//		ContextMenu::EntryCoverView |
//		ContextMenu::EntryDirectoryView |
//		ContextMenu::EntryFilterExtension |
		ContextMenu::EntryReload
	);

	return entries;
}

void ItemView::selectionChanged(const QItemSelection& selected, const QItemSelection& deselected )
{
	SearchableTableView::selectionChanged(selected, deselected);

	showClearButton(!selected.empty());

	if(m->currentlyFilling) {
		return;
	}

	selectedItemsChanged(selectedItems());
}

void ItemView::initContextMenu()
{
	initCustomContextMenu(nullptr);
}

// Right click stuff
void ItemView::initCustomContextMenu(ContextMenu* menu)
{
	if(m->contextMenu){
		return;
	}

	if(menu) {
		m->contextMenu = menu;
	}

	else {
		m->contextMenu = new ContextMenu(this);
	}

	if(!m->mergeMenu)
	{
		m->mergeMenu = new Gui::MergeMenu(m->contextMenu);
		connect(m->mergeMenu, &Gui::MergeMenu::sigMergeTriggered, this, &ItemView::mergeActionTriggered);
	}

	QAction* afterEditAction = m->contextMenu->actionAfter(ContextMenu::EntryEdit);

	if(afterEditAction)
	{
		m->contextMenu->insertAction(afterEditAction, m->mergeMenu->action());
	}

	connect(m->contextMenu, &ContextMenu::sigEditClicked, this, [=](){ showEdit(); });
	connect(m->contextMenu, &ContextMenu::sigInfoClicked, this, [=](){ showInfo(); });
	connect(m->contextMenu, &ContextMenu::sigLyricsClicked, this, [=](){ showLyrics(); });
	connect(m->contextMenu, &ContextMenu::sigDeleteClicked, this, &ItemView::deleteClicked);
	connect(m->contextMenu, &ContextMenu::sigPlayClicked, this, &ItemView::playClicked);
	connect(m->contextMenu, &ContextMenu::sigPlayNextClicked, this, &ItemView::playNextClicked);
	connect(m->contextMenu, &ContextMenu::sigPlayNewTabClicked, this, &ItemView::playNewTabClicked);
	connect(m->contextMenu, &ContextMenu::sigAppendClicked, this, &ItemView::appendClicked);
	connect(m->contextMenu, &ContextMenu::sigRefreshClicked, this, &ItemView::refreshClicked);
	connect(m->contextMenu, &ContextMenu::sigFilterTriggered, this, &ItemView::filterExtensionsTriggered);
	connect(m->contextMenu, &ContextMenu::sigReloadClicked, this, &ItemView::reloadClicked);

	this->showContextMenuActions(contextMenuEntries());

	m->contextMenu->addPreferenceAction(new Gui::LibraryPreferenceAction(m->contextMenu));
	m->contextMenu->setExtensions(library()->extensions());
}

ContextMenu* ItemView::contextMenu() const
{
	return m->contextMenu;
}

void ItemView::showContextMenu(const QPoint& p)
{
	m->contextMenu->exec(p);
}

void ItemView::showContextMenuActions(ContextMenu::Entries entries)
{
	m->contextMenu->showActions(entries);
}

QMimeData* ItemView::dragableMimedata() const
{
	return itemModel()->customMimedata();
}

QPixmap ItemView::dragPixmap() const
{
	Cover::Location cl = itemModel()->cover(selectedItems());

	QString cover_path = cl.preferredPath();
	return QPixmap(cover_path);
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

		connect(m->buttonClearSelection, &QPushButton::clicked, this, [=](){
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
		if(!b) {
			m->buttonClearSelection->hide();
		}
		else {
			m->buttonClearSelection->setVisible(this->selectedItems().count() > 0);
		}
	}
}

bool ItemView::isValidDragPosition(const QPoint &p) const
{
	QModelIndex idx = this->indexAt(p);
	return (idx.isValid() && (this->model()->flags(idx) & Qt::ItemFlag::ItemIsSelectable));
}

MetaDataList ItemView::infoDialogData() const
{
	return itemModel()->selectedMetadata();
}

void ItemView::mergeActionTriggered()
{
	Library::MergeData mergedata = m->mergeMenu->mergedata();

	if(mergedata.isValid()){
		runMergeOperation(mergedata);
	}
}

void ItemView::runMergeOperation(const Library::MergeData& md) { Q_UNUSED(md) }

void ItemView::playClicked() { emit sigPlayClicked(); }
void ItemView::playNewTabClicked() {	emit sigPlayNewTabClicked(); }
void ItemView::playNextClicked() { emit sigPlayNextClicked(); }
void ItemView::deleteClicked() { emit sigDeleteClicked(); }
void ItemView::appendClicked() { emit sigAppendClicked(); }
void ItemView::refreshClicked() { emit sigRefreshClicked(); }
void ItemView::reloadClicked() { emit sigReloadClicked(); }

void ItemView::viewTypeTriggered()
{
	Library::ViewType viewType = GetSetting(Set::Lib_ViewType);
	int i = static_cast<int>(viewType) + 1 % 3;

	SetSetting(Set::Lib_ViewType, static_cast<Library::ViewType>(i));
}

void ItemView::albumArtistsToggled()
{
	bool b = GetSetting(Set::Lib_ShowAlbumArtists);
	SetSetting(Set::Lib_ShowAlbumArtists, !b);
}

void ItemView::filterExtensionsTriggered(const QString& extension, bool b)
{
	AbstractLibrary* library = this->library();
	if(!library) {
		return;
	}

	Gui::ExtensionSet extensions = library->extensions();
	extensions.setEnabled(extension, b);
	library->setExtensions(extensions);
}

void ItemView::fill()
{
	this->clearSelection();

	int oldSize, newSize;
	m->model->refreshData(&oldSize, &newSize);

	if(newSize > oldSize) {
		resizeRowsToContents(oldSize, newSize - oldSize);
	}
}

void ItemView::selectedItemsChanged(const IndexSet& indexes)
{
	emit sigSelectionChanged(indexes);
}

void ItemView::importRequested(const QStringList& files)
{
	AbstractLibrary* lib = this->library();
	if(lib){
		lib->importFiles(files);
	}
}


void ItemView::resizeRowsToContents()
{
	if(rowCount() == 0) {
		return;
	}

	QHeaderView* header = this->verticalHeader();
	if(header) {
		header->resizeSections(QHeaderView::ResizeToContents);
	}
}


void ItemView::resizeRowsToContents(int first_row, int count)
{
	if(rowCount() == 0) {
		return;
	}

	QHeaderView* header = this->verticalHeader();
	if(header)
	{
		for(int i=first_row; i<first_row + count; i++)
		{
			this->resizeRowToContents(i);
		}
	}
}

void ItemView::mousePressEvent(QMouseEvent* event)
{
	if(rowCount() == 0) {
		return;
	}

	SearchableTableView::mousePressEvent(event);

	if(event->button() == Qt::MidButton)
	{
		if(!this->selectedItems().isEmpty()){
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

	const QPoint pos = event->globalPos();
	const IndexSet selections = selectedItems();

	if(metadataInterpretation() == MD::Interpretation::Tracks && selections.size() == 1)
	{
		m->contextMenu->showAction(ContextMenu::EntryLyrics, true);
	}

	else
	{
		m->contextMenu->showAction(ContextMenu::EntryLyrics, false);
	}

	if(isMergeable())
	{
		QMap<Id, QString> data;
		ItemModel* model = itemModel();
		for(int selectedIndex : selections)
		{
			Id id = model->mapIndexToId(selectedIndex);
			QString name = model->searchableString(selectedIndex);
			name.replace("&", "&&");

			data.insert(id, name);
		}

		m->mergeMenu->setData(data);
		m->mergeMenu->action()->setVisible( m->mergeMenu->isDataValid() );
	}

	m->contextMenu->setExtensions(library()->extensions());
	m->contextMenu->setSelectionCount(selections.count());

	showContextMenu(pos);
	QTableView::contextMenuEvent(event);
}

void ItemView::dragEnterEvent(QDragEnterEvent* event) {	event->accept(); }
void ItemView::dragMoveEvent(QDragMoveEvent* event) { event->accept(); }
void ItemView::dropEvent(QDropEvent* event)
{
	event->accept();

	const QMimeData* mimedata = event->mimeData();
	if(!mimedata) {
		return;
	}

	QString text;

	if(mimedata->hasText()){
		text = mimedata->text();
	}

	// extern drops
	if( !mimedata->hasUrls() || text.compare("tracks", Qt::CaseInsensitive) == 0) {
		return;
	}

	QStringList filelist;
	const QList<QUrl> urls = mimedata->urls();
	for(const QUrl& url : urls)
	{
		QString path = url.path();

		if(::Util::File::exists(path)) {
			filelist << path;
		}
	}

	importRequested(filelist);
}


void ItemView::changeEvent(QEvent* event)
{
	SearchableTableView::changeEvent(event);

	if(event->type() == QEvent::FontChange)
	{
		resizeRowsToContents();
	}
}

void ItemView::keyPressEvent(QKeyEvent* event)
{
	SearchableTableView::keyPressEvent(event);
}


void ItemView::resizeEvent(QResizeEvent* event)
{
	SearchableTableView::resizeEvent(event);

	if(m->buttonClearSelection){
		showClearButton(m->buttonClearSelection->isVisible());
	}
}


int ItemView::viewportHeight() const
{
	int h = SearchableTableView::viewportHeight();

	if(m->buttonClearSelection && m->buttonClearSelection->isVisible()) {
		return h - (m->buttonClearSelection->height() + 5);
	}

	return h;
}
