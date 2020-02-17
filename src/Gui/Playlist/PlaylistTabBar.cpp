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
	QString		last_dir;
	TabMenu*	menu=nullptr;
	int			tab_before_dd;
	int			drag_origin_tab;
	bool		drag_from_playlist;

	Private(QWidget* parent) :
		menu(new TabMenu(parent)),
		tab_before_dd(-1),
		drag_origin_tab(-1),
		drag_from_playlist(false)
	{
		last_dir = QDir::homePath();
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
	int cur_idx = currentIndex();
	QString cur_text = tabText(cur_idx);

	QString name = QInputDialog::getText(
				this,
				Lang::get(Lang::SaveAs).triplePt(),
				cur_text + ": " + Lang::get(Lang::SaveAs));

	if(!name.isEmpty())
	{
		emit sigTabSaveAs(cur_idx, name);
	}
}

void TabBar::saveToFilePressed()
{
	int cur_idx = currentIndex();

	QString name = QFileDialog::getSaveFileName(
				this,
				Lang::get(Lang::SaveAs).triplePt(),
				m->last_dir,
				"*.m3u");

	if(name.isEmpty()){
		return;
	}

	m->last_dir = Util::File::getParentDirectory(name);
	emit sigTabSaveToFile(cur_idx, name);
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

	Gui::LineInputDialog dialog(
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

static QShortcut* init_shortcut(QWidget* parent, QKeySequence key)
{
	QShortcut* sc;
	sc = new QShortcut(parent);
	sc->setKey(key);
	sc->setContext(Qt::WidgetWithChildrenShortcut);
	return sc;
}


void TabBar::initShortcuts()
{
	ShortcutHandler* sch = ShortcutHandler::instance();
	sch->shortcut(ShortcutIdentifier::AddTab).connect(this, this, SIGNAL(sigAddTabClicked()));
	sch->shortcut(ShortcutIdentifier::CloseTab).connect(this, this, SLOT(closePressed()));

	QShortcut* sc1 = init_shortcut(this->parentWidget(), QKeySequence::Save);
	connect(sc1, &QShortcut::activated, this, &TabBar::savePressed);

	QShortcut* sc2 = init_shortcut(this->parentWidget(), QKeySequence::SaveAs);
	connect(sc2, &QShortcut::activated, this, &TabBar::saveAsPressed);

	QShortcut* sc3 = init_shortcut(this->parentWidget(), QKeySequence("F2"));
	connect(sc3, &QShortcut::activated, this, &TabBar::renamePressed);

	QShortcut* sc4 = init_shortcut(this->parentWidget(), QKeySequence::Open);
	connect(sc4, &QShortcut::activated, this, &TabBar::openFilePressed);

	QKeySequence ks(QKeySequence::Open);
	QShortcut* sc5 = init_shortcut(this->parentWidget(), QKeySequence("Shift+" + ks.toString()));
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
	return m->drag_from_playlist;
}

int TabBar::getDragOriginTab() const
{
	return m->drag_origin_tab;
}


void TabBar::dragEnterEvent(QDragEnterEvent* e)
{
	QString object_name;
	if(e->source()){
		object_name = e->source()->objectName();
	}

	m->drag_origin_tab = -1;
	m->drag_from_playlist = object_name.contains("playlist_view");

	if(!m->drag_from_playlist){
		m->tab_before_dd = -1;
	}

	else if(m->tab_before_dd < 0){
		m->tab_before_dd = currentIndex();
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
	if((m->tab_before_dd >= 0) && (currentIndex() == count() - 1))
	{
		this->setCurrentIndex(m->tab_before_dd);
		m->tab_before_dd = -1;
	}

	e->accept();
}

void TabBar::dropEvent(QDropEvent* e)
{
	e->accept();
	int tab = this->tabAt(e->pos());

	m->drag_origin_tab = m->tab_before_dd;

	if(m->tab_before_dd >= 0 && currentIndex() == count() - 1)
	{
		this->setCurrentIndex(m->tab_before_dd);
	}

	m->tab_before_dd = -1;

	auto* mime_data = e->mimeData();
	if(!mime_data){
		return;
	}

	MetaDataList v_md = Gui::MimeData::metadata(mime_data);
	if(!v_md.isEmpty())
	{
		emit sigMetadataDropped(tab, v_md);
	}

	else
	{
		const QList<QUrl> urls = mime_data->urls();
		QStringList files;
		for(const QUrl& url : urls)
		{
			files << url.toLocalFile();
		}

		if(!files.isEmpty())
		{
			emit sigFilesDropped(tab, files);
		}
	}
}
