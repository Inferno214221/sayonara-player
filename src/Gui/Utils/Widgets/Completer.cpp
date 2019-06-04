/* SayonaraCompleter.cpp */

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

#include "Completer.h"
#include "Gui/Utils/Style.h"
#include "Gui/Utils/Delegates/ComboBoxDelegate.h"

#include <QStringList>
#include <QAbstractItemView>

using Gui::Completer;

Completer::Completer(const QStringList& lst, QObject* parent) :
	QCompleter(lst, parent)
{
	setCaseSensitivity(Qt::CaseInsensitive);
	setCompletionMode(QCompleter::UnfilteredPopupCompletion);

	popup()->setItemDelegate(new ComboBoxDelegate(this));
	popup()->setStyleSheet(Style::current_style());
}

Completer::~Completer() {}

void Completer::set_stringlist(const QStringList& lst)
{
	QAbstractItemModel* model = this->model();
	if(!model){
		return;
	}

	model->removeRows(0, this->model()->rowCount());
	model->insertRows(0, lst.size());

	int idx=0;
	for(const QString& str : lst){
		model->setData(model->index(idx++, 0), str);
	}
}

