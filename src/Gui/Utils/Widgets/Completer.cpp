/* SayonaraCompleter.cpp */

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

#include "Completer.h"
#include "Utils/Logger/Logger.h"
#include "Gui/Utils/Style.h"
#include "Gui/Utils/Delegates/ComboBoxDelegate.h"
#include "Utils/Utils.h"

#include <QStringList>
#include <QAbstractItemView>
#include <QLineEdit>
#include <QStringListModel>

using Gui::Completer;

Completer::Completer(const QStringList& strings, QObject* parent) :
	QCompleter(parent)
{
	auto* model = new QStringListModel(this);
	this->setModel(model);
	this->setStringList(strings);

	setCaseSensitivity(Qt::CaseInsensitive);
	setCompletionMode(QCompleter::UnfilteredPopupCompletion);

	popup()->setItemDelegate(new ComboBoxDelegate(this));
	popup()->setStyleSheet(Style::currentStyle());
	popup()->setObjectName("CompleterPopup");
}

Completer::~Completer() = default;

void Completer::setStringList(QStringList strings)
{
	auto* model = static_cast<QStringListModel*>(this->model());
	if(model)
	{
		strings.sort();
		model->setStringList(strings);
	}
}

QStringList Completer::splitPath(const QString& path) const
{
	QStringList splitted = path.split(",");
	if(splitted.isEmpty()){
		return QStringList();
	}

	else {
		return QStringList{splitted.last().trimmed().toLower()};
	}
}


QString Completer::pathFromIndex(const QModelIndex& index) const
{
	auto* line_edit = dynamic_cast<QLineEdit*>(this->widget());
	if(!line_edit){
		return QCompleter::pathFromIndex(index);
	}

	QString text = line_edit->text();
	QStringList items = text.split(",");
	if(items.isEmpty()){
		return QCompleter::pathFromIndex(index);
	}

	items.removeLast();

	for(auto it=items.begin(); it != items.end(); it++)
	{
		*it = it->trimmed();
	}

	QString completed = QCompleter::pathFromIndex(index);
	if(!completed.isEmpty())
	{
		QChar c = completed.at(0);
		completed.remove(0, 1);
		completed.push_front(c.toUpper());
	}

	items << completed;

	return items.join(", ");
}
