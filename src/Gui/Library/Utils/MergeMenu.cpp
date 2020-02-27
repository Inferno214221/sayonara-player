/* Mergable.cpp */

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

#include "MergeMenu.h"
#include "Utils/Library/MergeData.h"
#include "Utils/Set.h"

#include <QMenu>
#include <QStringList>
#include <QMap>

using Gui::MergeMenu;

struct MergeMenu::Private
{
	QAction*			action=nullptr;
	QMenu*				parent=nullptr;

	QMap<Id, QString>	data;
	Id					targetId;

	explicit Private(QMenu* parent) :
		parent(parent),
		targetId(-1)
	{}
};

MergeMenu::MergeMenu(QMenu* parent) :
	Gui::WidgetTemplate<QMenu>(parent)
{
	m = Pimpl::make<Private>(parent);

	m->action = m->parent->addMenu(this);
	m->action->setVisible(false);
	m->action->setText(tr("Merge"));
}

MergeMenu::~MergeMenu() = default;

void MergeMenu::setData(const QMap<Id, QString>& data)
{
	this->clear();
	m->data = data;

	if(data.size() < 2){
		return;
	}

	for(Id key : data.keys())
	{
		QString val	= data[key];

		auto* action = new QAction(this);
		action->setText(val);
		action->setData(key);
		this->addAction(action);

		connect(action, &QAction::triggered, this, [=]()
		{
			m->targetId = key;
			emit sigMergeTriggered();
		});
	}
}

QAction* MergeMenu::action() const
{
	return m->action;
}

bool MergeMenu::isDataValid() const
{
	return (m->data.size() > 1);
}

Library::MergeData MergeMenu::mergedata() const
{
	Util::Set<Id> source_ids;
	for(Id key : m->data.keys()){
		source_ids << key;
	}

	return Library::MergeData(source_ids, m->targetId, -1);
}

void MergeMenu::languageChanged()
{
	m->action->setText(tr("Merge"));
}
