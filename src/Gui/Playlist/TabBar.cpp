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

#include "TabBar.h"
#include "TabMenu.h"

#include "Gui/Utils/CustomMimeData.h"
#include "Gui/Utils/Shortcuts/ShortcutHandler.h"
#include "Gui/Utils/Shortcuts/Shortcut.h"

#include "Components/Directories/DirectoryReader.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/FileUtils.h"
#include "Utils/Language.h"

#include <QShortcut>
#include <QInputDialog>
#include <QFileDialog>
#include <QMouseEvent>
#include <QDir>

struct PlaylistTabBar::Private
{
	QString             last_dir;
	PlaylistTabMenu*	menu=nullptr;
	int					tab_before_dd;
	int					drag_origin_tab;
	bool				drag_from_playlist;


	Private(QWidget* parent) :
		menu(new PlaylistTabMenu(parent)),
		tab_before_dd(-1),
		drag_origin_tab(-1),
		drag_from_playlist(false)
	{
		last_dir = QDir::homePath();
	}
};

PlaylistTabBar::PlaylistTabBar(QWidget *parent) :
	QTabBar(parent),
	ShortcutWidget()
{
	m = Pimpl::make<Private>(this);

	this->setDrawBase(false);
	this->setAcceptDrops(true);

	init_shortcuts();

	connect(m->menu, &PlaylistTabMenu::sig_open_file_clicked, this, &PlaylistTabBar::open_file_pressed);
	connect(m->menu, &PlaylistTabMenu::sig_open_dir_clicked, this, &PlaylistTabBar::open_dir_pressed);
	connect(m->menu, &PlaylistTabMenu::sig_rename_clicked, this, &PlaylistTabBar::rename_pressed);
	connect(m->menu, &PlaylistTabMenu::sig_reset_clicked, this, &PlaylistTabBar::reset_pressed);
	connect(m->menu, &PlaylistTabMenu::sig_save_clicked, this, &PlaylistTabBar::save_pressed);
	connect(m->menu, &PlaylistTabMenu::sig_save_as_clicked, this, &PlaylistTabBar::save_as_pressed);
	connect(m->menu, &PlaylistTabMenu::sig_save_to_file_clicked, this, &PlaylistTabBar::save_to_file_pressed);
	connect(m->menu, &PlaylistTabMenu::sig_clear_clicked, this, &PlaylistTabBar::clear_pressed);
	connect(m->menu, &PlaylistTabMenu::sig_delete_clicked, this, &PlaylistTabBar::delete_pressed);
	connect(m->menu, &PlaylistTabMenu::sig_close_clicked, this, &PlaylistTabBar::close_pressed);
	connect(m->menu, &PlaylistTabMenu::sig_close_others_clicked, this, &PlaylistTabBar::close_others_pressed);
}

PlaylistTabBar::~PlaylistTabBar() {}

void PlaylistTabBar::save_pressed()
{
	emit sig_tab_save(currentIndex());
}

void PlaylistTabBar::save_as_pressed()
{
	int cur_idx = currentIndex();
	QString cur_text = tabText(cur_idx);

	QString name = QInputDialog::getText(
				this,
				Lang::get(Lang::SaveAs).triplePt(),
				cur_text + ": " + Lang::get(Lang::SaveAs));

	if(!name.isEmpty()){
		emit sig_tab_save_as(cur_idx, name);
	}
}

void PlaylistTabBar::save_to_file_pressed()
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

void PlaylistTabBar::open_file_pressed()
{
	emit sig_open_file(currentIndex());
}

void PlaylistTabBar::open_dir_pressed()
{
	emit sig_open_dir(currentIndex());
}

void PlaylistTabBar::clear_pressed()
{
	emit sig_tab_clear(currentIndex());
}

void PlaylistTabBar::delete_pressed()
{
	emit sig_tab_delete(currentIndex());
}

void PlaylistTabBar::close_pressed()
{
	emit tabCloseRequested(this->currentIndex());
}

void PlaylistTabBar::reset_pressed()
{
	emit sig_tab_reset(currentIndex());
}

void PlaylistTabBar::rename_pressed()
{
	int cur_idx = currentIndex();
	QString cur_text = tabText(cur_idx);

	QString name = QInputDialog::getText(
				this,
				Lang::get(Lang::Rename),
				cur_text + ": " + Lang::get(Lang::Rename));

	if(name.isEmpty()){
		return;
	}

	if(name.compare(cur_text) == 0){
		return;
	}

	emit sig_tab_rename(currentIndex(), name);
}


void PlaylistTabBar::close_others_pressed()
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

void PlaylistTabBar::mousePressEvent(QMouseEvent* e){
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

void PlaylistTabBar::wheelEvent(QWheelEvent* e)
{
	QTabBar::wheelEvent(e);
	if(this->currentIndex() == this->count() - 1 &&
			this->count() > 1)
	{
		this->setCurrentIndex(this->count() - 2);
	}
}

void PlaylistTabBar::init_shortcuts()
{
	ShortcutHandler* sch = ShortcutHandler::instance();
	sch->shortcut(ShortcutIdentifier::AddTab).connect(this, this, SIGNAL(sig_add_tab_clicked()));
	sch->shortcut(ShortcutIdentifier::CloseTab).connect(this, this, SLOT(close_pressed()));
}


void PlaylistTabBar::show_menu_items(PlaylistMenuEntries entries)
{
	m->menu->show_menu_items(entries);
}

void PlaylistTabBar::setTabsClosable(bool b)
{
	QTabBar::setTabsClosable(b);
	m->menu->show_close(b);
}

bool PlaylistTabBar::was_drag_from_playlist() const
{
	return m->drag_from_playlist;
}

int PlaylistTabBar::get_drag_origin_tab() const
{
	return m->drag_origin_tab;
}


void PlaylistTabBar::dragEnterEvent(QDragEnterEvent* e)
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

void PlaylistTabBar::dragMoveEvent(QDragMoveEvent* e)
{
	e->accept();

	int tab = tabAt(e->pos());
	this->setCurrentIndex(tab);
}

void PlaylistTabBar::dragLeaveEvent(QDragLeaveEvent* e)
{
	if((m->tab_before_dd >= 0) && (currentIndex() == count() - 1))
	{
		this->setCurrentIndex(m->tab_before_dd);
		m->tab_before_dd = -1;
	}

	e->accept();
}

void PlaylistTabBar::dropEvent(QDropEvent* e)
{
	e->accept();
	int tab = this->tabAt(e->pos());

	m->drag_origin_tab = m->tab_before_dd;

	if(m->tab_before_dd >= 0 && currentIndex() == count() - 1){
		this->setCurrentIndex(m->tab_before_dd);
	}

	m->tab_before_dd = -1;

	const QMimeData* mime_data = e->mimeData();
	if(!mime_data){
		return;
	}

	const CustomMimeData* cmd = dynamic_cast<const CustomMimeData*>(mime_data);
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
