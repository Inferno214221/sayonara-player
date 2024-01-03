/* DiscPopupMenu.cpp */

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

#include "DiscPopupMenu.h"

#include "Utils/Algorithm.h"
#include "Utils/Language/Language.h"

#include "Gui/Utils/GuiUtils.h"

#include <QMouseEvent>

namespace Algorithm = Util::Algorithm;
using Library::DiscAction;
using Library::DiscPopupMenu;

DiscAction::DiscAction(QWidget* parent, Disc disc) :
	QAction(parent)
{
	if(disc == std::numeric_limits<Disc>::max())
	{
		this->setText(Lang::get(Lang::All));
		this->setIcon(Gui::Util::icon("cds.png", Gui::Util::NoTheme));
	}

	else
	{
		this->setText(Lang::get(Lang::Disc) + " " + QString::number(disc));
		this->setIcon(Gui::Util::icon("cd.png", Gui::Util::NoTheme));
	}

	this->setData(disc);

	connect(this, &QAction::triggered, this, [=]() {
		bool ok = false;
		int discnumber = data().toInt(&ok);
		if(ok)
		{
			emit sigDiscPressed(discnumber);
		}
	});
}

DiscAction::~DiscAction() = default;

DiscPopupMenu::DiscPopupMenu(QWidget* parent, QList<Disc> discs) :
	QMenu(parent)
{
	Algorithm::sort(discs, [](Disc disc1, Disc disc2) {
		return (disc1 < disc2);
	});

	for(int i = -1; i < discs.size(); i++)
	{
		DiscAction* action;
		if(i == -1)
		{
			action = new DiscAction(this, std::numeric_limits<Disc>::max());
		}

		else
		{
			action = new DiscAction(this, discs[i]);
		}

		this->addAction(action);

		connect(action, &DiscAction::sigDiscPressed, this, &DiscPopupMenu::sigDiscPressed);
	}
}

DiscPopupMenu::~DiscPopupMenu() = default;
