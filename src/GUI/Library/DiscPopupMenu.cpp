/* DiscPopupMenu.cpp */

/* Copyright (C) 2011-2016  Lucio Carreras
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



#include "DiscPopupMenu.h"

DiscAction::DiscAction(QWidget* parent) :
	QAction(parent)
{
	connect(this, &QAction::triggered, this, &DiscAction::disc_hover);
}

void DiscAction::disc_hover(){
	bool ok = false;
	int discnumber = data().toInt(&ok);
	if(ok){
		emit sig_disc_pressed(discnumber);
	}
}



DiscPopupMenu::DiscPopupMenu(QWidget* parent, QList<quint8> discs): QMenu(parent){

	for(int i= -1; i<discs.size(); i++){
		QString text;
		int data;

		if(i == -1) {
			text = "All";
			data = -1;
		}

		else{
			quint8 disc = discs[i];
			text = QString("Disc ") + QString::number(disc);
			data = disc;
		}

		DiscAction* action = new DiscAction(this);
		connect(action, &DiscAction::sig_disc_pressed, this, &DiscPopupMenu::disc_pressed);

		action->setText(text);
		action->setData(data);

		addAction(action);
		_actions << action;


	}
}

DiscPopupMenu::~DiscPopupMenu(){
	clean_up();
}


void DiscPopupMenu::disc_pressed(int disc){
	emit sig_disc_pressed(disc);
}


void DiscPopupMenu::mouseReleaseEvent(QMouseEvent* e){
	QMenu::mouseReleaseEvent(e);
	hide();
	close();
}


void DiscPopupMenu::clean_up(){
	for(DiscAction* a : _actions){
		if(!a) continue;
		delete a;
		a = 0;
	}
}
