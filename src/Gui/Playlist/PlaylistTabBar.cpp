/* PlaylistTabBar.cpp */

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

#include "PlaylistTabBar.h"
#include "PlaylistTabMenu.h"

#include "Gui/Utils/CustomMimeData.h"
#include "Gui/Utils/Shortcuts/ShortcutHandler.h"
#include "Gui/Utils/Shortcuts/Shortcut.h"
#include "Gui/Utils/InputDialog/LineInputDialog.h"

#include "Components/Directories/DirectoryReader.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/FileUtils.h"
#include "Utils/Language/Language.h"

#include <QShortcut>
#include <QInputDialog>
#include <QFileDialog>
#include <QMouseEvent>
#include <QDir>

using Playlist::TabBar;

struct TabBar::Private
{
	QString             last_dir;
	TabMenu*	menu=nullptr;
	int					tab_before_dd;
	int					drag_origin_tab;
	bool				drag_from_playlist;

	Private(QWidget* parent) :
		menu(new TabMenu(parent)),
		tab_before_dd(-1),
		drag_origin_tab(-1),
		drag_from_playlist(false)
	{
		last_dir = QDir::homePath();
	}
};

TabBar::TabBar(QWidget *parent) :
	QTabBar(parent)
{
	m = Pimpl::make<Private>(this);

	this->setDrawBase(false);
	this->setAcceptDrops(true);
	this->setFocusPolicy(Qt::NoFocus);

	init_shortcuts();

	connect(m->menu, &TabMenu::sig_open_file_clicked, this, &TabBar::open_file_pressed);
	connect(m->menu, &TabMenu::sig_open_dir_clicked, this, &TabBar::open_dir_pressed);
	connect(m->menu, &TabMenu::sig_rename_clicked, this, &TabBar::rename_pressed);
	connect(m->menu, &TabMenu::sig_reset_clicked, this, &TabBar::reset_pressed);
	connect(m->menu, &TabMenu::sig_save_clicked, this, &TabBar::save_pressed);
	connect(m->menu, &TabMenu::sig_save_as_clicked, this, &TabBar::save_as_pressed);
	connect(m->menu, &TabMenu::sig_save_to_file_clicked, this, &TabBar::save_to_file_pressed);
	connect(m->menu, &TabMenu::sig_clear_clicked, this, &TabBar::clear_pressed);
	connect(m->menu, &TabMenu::sig_delete_clicked, this, &TabBar::delete_pressed);
	connect(m->menu, &TabMenu::sig_close_clicked, this, &TabBar::close_pressed);
	connect(m->menu, &TabMenu::sig_close_others_clicked, this, &TabBar::close_others_pressed);
}

TabBar::~TabBar() = default;

void TabBar::save_pressed()
{
	emit sig_tab_save(currentIndex());
}

void TabBar::save_as_pressed()
{
	int cur_idx = currentIndex();
	QString cur_text = tabText(cur_idx);

	QString name = QInputDialog::getText(
				this,
				Lang::get(Lang::SaveAs).triplePt(),
				cur_text + ": " + Lang::get(Lang::SaveAs));

	if(!name.isEmpty())
	{
		emit sig_tab_save_as(cur_idx, name);
	}
}

void TabBar::save_to_file_pressed()
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

	m->last_dir = Util::File::get_parent_directory(name);
	emit sig_tab_save_to_file(cur_idx, name);
}

void TabBar::open_file_pressed()
{
	emit sig_open_file(currentIndex());
}

void TabBar::open_dir_pressed()
{
	emit sig_open_dir(currentIndex());
}

void TabBar::clear_pressed()
{
	emit sig_tab_clear(currentIndex());
}

void TabBar::delete_pressed()
{
	emit sig_tab_delete(currentIndex());
}

void TabBar::close_pressed()
{
	emit tabCloseRequested(this->currentIndex());
}

void TabBar::reset_pressed()
{
	emit sig_tab_reset(currentIndex());
}

void TabBar::rename_pressed()
{
	int cur_idx = currentIndex();
	QString cur_text = tabText(cur_idx);

	Gui::LineInputDialog dialog(
		Lang::get(Lang::Rename),
		Lang::get(Lang::Rename),
		cur_text,
		this
	);

	dialog.exec();
	if(dialog.return_value() != Gui::LineInputDialog::ReturnValue::Ok) {
		return;
	}

	QString name = dialog.text();
	if(name.isEmpty()){
		return;
	}

	if(name.compare(cur_text) == 0){
		return;
	}

	emit sig_tab_rename(currentIndex(), name);
}


void TabBar::close_others_pressed()
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
		emit sig_add_tab_clicked();
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


void TabBar::init_shortcuts()
{
	ShortcutHandler* sch = ShortcutHandler::instance();
	sch->shortcut(ShortcutIdentifier::AddTab).connect(this, this, SIGNAL(sig_add_tab_clicked()));
	sch->shortcut(ShortcutIdentifier::CloseTab).connect(this, this, SLOT(close_pressed()));

	QShortcut* sc1 = init_shortcut(this->parentWidget(), QKeySequence::Save);
	connect(sc1, &QShortcut::activated, this, &TabBar::save_pressed);

	QShortcut* sc2 = init_shortcut(this->parentWidget(), QKeySequence::SaveAs);
	connect(sc2, &QShortcut::activated, this, &TabBar::save_as_pressed);

	QShortcut* sc3 = init_shortcut(this->parentWidget(), QKeySequence("F2"));
	connect(sc3, &QShortcut::activated, this, &TabBar::rename_pressed);

	QShortcut* sc4 = init_shortcut(this->parentWidget(), QKeySequence::Open);
	connect(sc4, &QShortcut::activated, this, &TabBar::open_file_pressed);

	QKeySequence ks(QKeySequence::Open);
	QShortcut* sc5 = init_shortcut(this->parentWidget(), QKeySequence("Shift+" + ks.toString()));
	connect(sc5, &QShortcut::activated, this, &TabBar::open_dir_pressed);
}

void TabBar::show_menu_items(Playlist::MenuEntries entries)
{
	m->menu->show_menu_items(entries);
}

void TabBar::setTabsClosable(bool b)
{
	QTabBar::setTabsClosable(b);
	m->menu->show_close(b);
}

bool TabBar::was_drag_from_playlist() const
{
	return m->drag_from_playlist;
}

int TabBar::get_drag_origin_tab() const
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

	if(m->tab_before_dd >= 0 && currentIndex() == count() - 1){
		this->setCurrentIndex(m->tab_before_dd);
	}

	m->tab_before_dd = -1;

	auto* mime_data = e->mimeData();
	if(!mime_data){
		return;
	}

	auto* cmd = dynamic_cast<const Gui::CustomMimeData*>(mime_data);
	if(!cmd)
	{
		if(!mime_data->hasUrls()){
			return;
		}

		MetaDataList v_md;
		DirectoryReader dir_reader;
		QList<QUrl> urls = mime_data->urls();

		QStringList files;
		for(const QUrl& url : urls){
			files << url.toLocalFile();
		}

		v_md = dir_reader.scan_metadata(files);
		emit sig_metadata_dropped(tab, v_md);

		return;
	}

	if(!cmd->has_metadata()){
		return;
	}

	emit sig_metadata_dropped(tab, cmd->metadata());
}
