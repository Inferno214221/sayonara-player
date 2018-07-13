/* DiscPopupMenu.cpp */

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

#include "DiscPopupMenu.h"
#include "GUI/Utils/GuiUtils.h"
#include "Utils/Utils.h"
#include "Utils/Language.h"

#include <QMouseEvent>
#include <algorithm>

DiscAction::DiscAction(QWidget* parent, Disc disc) :
	QAction(parent)
{
	if(disc == std::numeric_limits<Disc>::max())
	{
		this->setText(Lang::get(Lang::All));
		this->setIcon(Gui::Util::icon("cds.png"));
	}

	else {
		this->setText(tr("Disc") + " " + QString::number(disc));
		this->setIcon(Gui::Util::icon("cd.png"));
	}

	this->setData(disc);

	connect(this, &QAction::triggered, this, [=]()
	{
		bool ok = false;
		int discnumber = data().toInt(&ok);
		if(ok){
			emit sig_disc_pressed(discnumber);
		}
	});
}

DiscAction::~DiscAction() {}

DiscPopupMenu::DiscPopupMenu(QWidget* parent, QList<Disc> discs): QMenu(parent)
{
	Util::sort(discs, [](Disc disc1, Disc disc2){
		return (disc1 < disc2);
	});

	for(int i=-1; i<discs.size(); i++)
	{
		DiscAction* action;
		if(i == -1){
			action = new DiscAction(this, std::numeric_limits<Disc>::max());
		}

		else {
			action = new DiscAction(this, discs[i]);
		}

		this->addAction(action);

		connect(action, &DiscAction::sig_disc_pressed, this, &DiscPopupMenu::sig_disc_pressed);
	}
}

DiscPopupMenu::~DiscPopupMenu() {}
