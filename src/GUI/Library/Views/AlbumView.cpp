/* LibraryViewAlbum.cpp */

/* Copyright (C) 2011-2017  Lucio Carreras
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

#include "AlbumView.h"
#include "GUI/Library/Helper/DiscPopupMenu.h"
#include "GUI/Library/Helper/ColumnIndex.h"

#include <QHeaderView>

using namespace Library;

struct AlbumView::Private
{
	QList< QList<uint8_t> >	discnumbers;
	DiscPopupMenu*			discmenu=nullptr;
	QPoint					discmenu_point;
};

AlbumView::AlbumView(QWidget *parent) :
	TableView(parent)
{
	m = Pimpl::make<Private>();
	connect(this, &QTableView::clicked, this, &AlbumView::index_clicked);
}

AlbumView::~AlbumView() {}

void AlbumView::rc_menu_show(const QPoint & p)
{
	delete_discmenu();

	TableView::rc_menu_show(p);
}


void AlbumView::index_clicked(const QModelIndex &idx)
{
	if(idx.column() != static_cast<int>(ColumnIndex::Album::MultiDisc))
	{
		return;
	}

	QModelIndexList selections = this->selectionModel()->selectedRows();
	if(selections.size() == 1){
		init_discmenu(idx);
		show_discmenu();
	}
}


/* where to show the popup */
void AlbumView::calc_discmenu_point(QModelIndex idx)
{
	m->discmenu_point = QCursor::pos();

	QRect box = this->geometry();
	box.moveTopLeft(this->parentWidget()->mapToGlobal(box.topLeft()));

	if(!box.contains(m->discmenu_point)){
		m->discmenu_point.setX(box.x() + (box.width() * 2) / 3);
		m->discmenu_point.setY(box.y());

		QPoint dmp_tmp = parentWidget()->pos();
		dmp_tmp.setY(dmp_tmp.y() - this->verticalHeader()->sizeHint().height());

		while(idx.row() != indexAt(dmp_tmp).row()){
			  dmp_tmp.setY(dmp_tmp.y() + 10);
			  m->discmenu_point.setY(m->discmenu_point.y() + 10);
		}
	}
}

void AlbumView::init_discmenu(QModelIndex idx)
{
	int row = idx.row();
	QList<uint8_t> discnumbers;
	delete_discmenu();

	if( !idx.isValid() ||
		(row > m->discnumbers.size()) ||
		(row < 0) )
	{
		return;
	}

	discnumbers = m->discnumbers[row];
	if(discnumbers.size() < 2) {
		return;
	}

	calc_discmenu_point(idx);

	m->discmenu = new DiscPopupMenu(this, discnumbers);

	connect(m->discmenu, &DiscPopupMenu::sig_disc_pressed, this, &AlbumView::sig_disc_pressed);
}


void AlbumView::delete_discmenu()
{
	if(!m->discmenu) {
		return;
	}

	m->discmenu->hide();
	m->discmenu->close();

	disconnect(m->discmenu, &DiscPopupMenu::sig_disc_pressed, this, &AlbumView::sig_disc_pressed);

	m->discmenu->deleteLater();
	m->discmenu = nullptr;
}


void AlbumView::show_discmenu()
{
	if(!m->discmenu) return;

	m->discmenu->popup(m->discmenu_point);
}


void AlbumView::clear_discnumbers()
{
	m->discnumbers.clear();
}

void AlbumView::add_discnumbers(const QList<uint8_t>& dns)
{
	m->discnumbers << dns;
}