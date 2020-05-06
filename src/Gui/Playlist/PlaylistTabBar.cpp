/* PlaylistTabBar.cpp */

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

#include "PlaylistTabBar.h"
#include "PlaylistTabMenu.h"

#include "Gui/Utils/Shortcuts/ShortcutHandler.h"
#include "Gui/Utils/Shortcuts/Shortcut.h"
#include "Gui/Utils/InputDialog/LineInputDialog.h"
#include "Gui/Utils/MimeDataUtils.h"

#include "Utils/MetaData/MetaDataList.h"
#include "Utils/FileUtils.h"
#include "Utils/Language/Language.h"
#include "Utils/Algorithm.h"

#include <QShortcut>
#include <QInputDialog>
#include <QFileDialog>
#include <QMimeData>
#include <QMouseEvent>
#include <QDir>

using Playlist::TabBar;
using Playlist::TabMenu;

struct TabBar::Private
{
	QString		lastDir;
	TabMenu*	menu=nullptr;
	int			tabBeforeDragDrop;
	int			dragOriginTab;
	bool		dragFromPlaylist;

	Private(QWidget* parent) :
		menu(new TabMenu(parent)),
		tabBeforeDragDrop(-1),
		dragOriginTab(-1),
		dragFromPlaylist(false)
	{
		lastDir = QDir::homePath();
	}
};

TabBar::TabBar(QWidget* parent) :
	QTabBar(parent)
{
	m = Pimpl::make<Private>(this);

	this->setDrawBase(false);
	this->setAcceptDrops(true);
	this->setFocusPolicy(Qt::NoFocus);

	initShortcuts();

	connect(m->menu, &TabMenu::sigOpenFileClicked, this, &TabBar::openFilePressed);
	connect(m->menu, &TabMenu::sigOpenDirClicked, this, &TabBar::openDirPressed);
	connect(m->menu, &TabMenu::sigRenameClicked, this, &TabBar::renamePressed);
	connect(m->menu, &TabMenu::sigResetClicked, this, &TabBar::resetPressed);
	connect(m->menu, &TabMenu::sigSaveClicked, this, &TabBar::savePressed);
	connect(m->menu, &TabMenu::sigSaveAsClicked, this, &TabBar::saveAsPressed);
	connect(m->menu, &TabMenu::sigSaveToFileClicked, this, &TabBar::saveToFilePressed);
	connect(m->menu, &TabMenu::sigClearClicked, this, &TabBar::clearPressed);
	connect(m->menu, &TabMenu::sigDeleteClicked, this, &TabBar::deletePressed);
	connect(m->menu, &TabMenu::sigCloseClicked, this, &TabBar::closePressed);
	connect(m->menu, &TabMenu::sigCloseOthersClicked, this, &TabBar::closeOthersPressed);
}

TabBar::~TabBar() = default;

void TabBar::savePressed()
{
	emit sigTabSave(currentIndex());
}

void TabBar::saveAsPressed()
{
	int currentIndex = this->currentIndex();
	QString currentText = tabText(currentIndex);

	const QString name = QInputDialog::getText
	(
		this,
		Lang::get(Lang::SaveAs).triplePt(),
		currentText + ": " + Lang::get(Lang::SaveAs)
	);

	if(!name.isEmpty())
	{
		emit sigTabSaveAs(currentIndex, name);
	}
}

void TabBar::saveToFilePressed()
{
	const QString name = QFileDialog::getSaveFileName(this,
		Lang::get(Lang::SaveAs), m->lastDir, "*.m3u"
	);

	if(!name.isEmpty())
	{
		m->lastDir = Util::File::getParentDirectory(name);
		emit sigTabSaveToFile(this->currentIndex(), name);
	}
}

void TabBar::openFilePressed()
{
	emit sigOpenFile(currentIndex());
}

void TabBar::openDirPressed()
{
	emit sigOpenDir(currentIndex());
}

void TabBar::clearPressed()
{
	emit sigTabClear(currentIndex());
}

void TabBar::deletePressed()
{
	emit sigTabDelete(currentIndex());
}

void TabBar::closePressed()
{
	emit tabCloseRequested(this->currentIndex());
}

void TabBar::resetPressed()
{
	emit sigTabReset(currentIndex());
}

void TabBar::renamePressed()
{
	int currentIndex = this->currentIndex();
	QString currentText = tabText(currentIndex);

	Gui::LineInputDialog dialog
	(
		Lang::get(Lang::Rename),
		Lang::get(Lang::Rename),
		currentText,
		this
	);

	dialog.exec();
	if(dialog.returnValue() != Gui::LineInputDialog::ReturnValue::Ok) {
		return;
	}

	QString name = dialog.text();
	if(name.isEmpty()){
		return;
	}

	if(name.compare(currentText) == 0){
		return;
	}

	emit sigTabRename(this->currentIndex(), name);
}

void TabBar::closeOthersPressed()
{
	int my_tab = currentIndex();
	int i=0;

	while( count() > 2)
	{
		if(i < my_tab){
			emit tabCloseRequested(0);
		}

		else if(i == my_tab) {}

		else{
			emit tabCloseRequested(1);
		}

		i++;
	}
}

void TabBar::mousePressEvent(QMouseEvent* e){
	int idx = this->tabAt(e->pos());

	if(idx == this->count() - 1){
		emit sigAddTabClicked();
		return;
	}

	else{
		this->setCurrentIndex(idx);
	}

	if(e->button() == Qt::RightButton){
		m->menu->exec(e->globalPos());
	}

	else if(e->button() == Qt::MiddleButton)
	{
		if(this->count() > 2){
			emit tabCloseRequested(idx);
		}
	}
}

void TabBar::wheelEvent(QWheelEvent* e)
{
	QTabBar::wheelEvent(e);
	if(this->currentIndex() == this->count() - 1 &&
			this->count() > 1)
	{
		this->setCurrentIndex(this->count() - 2);
	}
}

static QShortcut* initShortcut(QWidget* parent, QKeySequence key)
{
	QShortcut* sc;
	sc = new QShortcut(parent);
	sc->setKey(key);
	sc->setContext(Qt::WidgetWithChildrenShortcut);
	return sc;
}

void TabBar::initShortcuts()
{
	auto* sch = ShortcutHandler::instance();
	sch->shortcut(ShortcutIdentifier::AddTab).connect(this, this, SIGNAL(sigAddTabClicked()));
	sch->shortcut(ShortcutIdentifier::CloseTab).connect(this, this, SLOT(closePressed()));

	auto* sc1 = initShortcut(this->parentWidget(), QKeySequence::Save);
	connect(sc1, &QShortcut::activated, this, &TabBar::savePressed);

	auto* sc2 = initShortcut(this->parentWidget(), QKeySequence::SaveAs);
	connect(sc2, &QShortcut::activated, this, &TabBar::saveAsPressed);

	auto* sc3 = initShortcut(this->parentWidget(), QKeySequence("F2"));
	connect(sc3, &QShortcut::activated, this, &TabBar::renamePressed);

	auto* sc4 = initShortcut(this->parentWidget(), QKeySequence::Open);
	connect(sc4, &QShortcut::activated, this, &TabBar::openFilePressed);

	QKeySequence ks(QKeySequence::Open);
	auto* sc5 = initShortcut(this->parentWidget(), QKeySequence("Shift+" + ks.toString()));
	connect(sc5, &QShortcut::activated, this, &TabBar::openDirPressed);
}

void TabBar::showMenuItems(Playlist::MenuEntries entries)
{
	m->menu->showMenuItems(entries);
}

void TabBar::setTabsClosable(bool b)
{
	QTabBar::setTabsClosable(b);
	m->menu->showClose(b);
}

bool TabBar::wasDragFromPlaylist() const
{
	return m->dragFromPlaylist;
}

int TabBar::getDragOriginTab() const
{
	return m->dragOriginTab;
}

void TabBar::dragEnterEvent(QDragEnterEvent* e)
{
	QString object_name;
	if(e->source()){
		object_name = e->source()->objectName();
	}

	m->dragOriginTab = -1;
	m->dragFromPlaylist = object_name.contains("playlist_view");

	if(!m->dragFromPlaylist){
		m->tabBeforeDragDrop = -1;
	}

	else if(m->tabBeforeDragDrop < 0){
		m->tabBeforeDragDrop = currentIndex();
	}

	e->accept();

	int tab = tabAt(e->pos());
	this->setCurrentIndex(tab);
}

void TabBar::dragMoveEvent(QDragMoveEvent* e)
{
	e->accept();

	int tab = tabAt(e->pos());
	this->setCurrentIndex(tab);
}

void TabBar::dragLeaveEvent(QDragLeaveEvent* e)
{
	if((m->tabBeforeDragDrop >= 0) && (currentIndex() == count() - 1))
	{
		this->setCurrentIndex(m->tabBeforeDragDrop);
		m->tabBeforeDragDrop = -1;
	}

	e->accept();
}

void TabBar::dropEvent(QDropEvent* e)
{
	e->accept();
	int tab = this->tabAt(e->pos());

	m->dragOriginTab = m->tabBeforeDragDrop;

	if(m->tabBeforeDragDrop >= 0 && currentIndex() == count() - 1)
	{
		this->setCurrentIndex(m->tabBeforeDragDrop);
	}

	m->tabBeforeDragDrop = -1;

	auto* mimeData = e->mimeData();
	if(!mimeData){
		return;
	}

	const MetaDataList tracks = Gui::MimeData::metadata(mimeData);
	if(!tracks.isEmpty())
	{
		emit sigMetadataDropped(tab, tracks);
	}

	else
	{
		const QList<QUrl> urls = mimeData->urls();

		QStringList files;
		Util::Algorithm::transform(urls, files, [](const QUrl& url){
			return  url.toLocalFile();
		});

		if(!files.isEmpty())
		{
			emit sigFilesDropped(tab, files);
		}
	}
}
