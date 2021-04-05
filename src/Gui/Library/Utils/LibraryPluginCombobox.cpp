/* LibraryPluginCombobox.cpp */

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

#include "LibraryPluginCombobox.h"
#include "LibraryPluginComboBoxDelegate.h"

#include "Components/LibraryManagement/AbstractLibraryContainer.h"
#include "Components/LibraryManagement/LibraryPluginHandler.h"

#include "Utils/Algorithm.h"
#include "Utils/Logger/Logger.h"

#include <QList>
#include <QAction>
#include <QSize>
#include <QFontMetrics>

using Library::AbstractContainer;
using Library::PluginHandler;
using Library::PluginCombobox;
using Library::PluginComboBoxDelegate;

namespace Algorithm = Util::Algorithm;

struct PluginCombobox::Private
{
	QList<QAction*> actions;
};

PluginCombobox::PluginCombobox(const QString& text, QWidget* parent) :
	ComboBox(parent)
{
	m = Pimpl::make<Private>();

	this->setSizeAdjustPolicy(QComboBox::AdjustToContents);
	this->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	this->setFrame(false);
	this->setIconSize(QSize(16, 16));
	this->setFocusPolicy(Qt::ClickFocus);

	this->setItemDelegate(new PluginComboBoxDelegate(this));

	auto* lph = PluginHandler::instance();
	connect(lph, &PluginHandler::sigLibrariesChanged, this, &PluginCombobox::setupActions);
	connect(lph, &PluginHandler::sigCurrentLibraryChanged, this, &PluginCombobox::currentLibraryChanged);

	connect(this, combo_activated_int, this, &PluginCombobox::currentIndexChanged);

	setupActions();
	setCurrentText(text);
}

PluginCombobox::~PluginCombobox() = default;

void PluginCombobox::setupActions()
{
	QFontMetrics fm(this->font());

	this->clear();

	const QList<AbstractContainer*> libraries = PluginHandler::instance()->libraries(true);
	for(const AbstractContainer* container : libraries)
	{
		const auto icon = container->icon();

		QString display_name = fm.elidedText(container->displayName(), Qt::TextElideMode::ElideRight, 200);
		this->addItem(icon, display_name, container->name());
	}

	this->insertSeparator(1);
	this->setItemIcon(1, QIcon());

	currentLibraryChanged();
}

void PluginCombobox::actionTriggered(bool b)
{
	if(!b)
	{
		return;
	}

	auto* action = dynamic_cast<QAction*>(sender());
	QString name = action->data().toString();

	PluginHandler::instance()->setCurrentLibrary(name);
	for(QAction* libraryAction : Algorithm::AsConst(m->actions))
	{
		if(libraryAction == action)
		{
			continue;
		}

		libraryAction->setChecked(false);
	}
}

void PluginCombobox::currentLibraryChanged()
{
	AbstractContainer* currentLibrary = PluginHandler::instance()->currentLibrary();
	if(!currentLibrary)
	{
		return;
	}

	QString name = currentLibrary->name();
	for(int i = 0; i < this->count(); i++)
	{
		if(this->itemData(i).toString().compare(name) == 0)
		{
			if(i != this->currentIndex())
			{
				this->setCurrentIndex(i);
			}

			break;
		}
	}
}

void PluginCombobox::currentIndexChanged(int index)
{
	PluginHandler::instance()->setCurrentLibrary(index - 2);
}

void PluginCombobox::languageChanged()
{
	if(!m)
	{
		return;
	}

	setupActions();
}

void PluginCombobox::skinChanged()
{
	if(!m)
	{
		return;
	}

	const QList<AbstractContainer*> libraries = PluginHandler::instance()->libraries(true);
	int i = 0;

	for(const AbstractContainer* container : libraries)
	{
		const auto icon = container->icon();

		if(this->itemData(i, Qt::DisplayRole).toString().isEmpty())
		{
			i++;
		}

		this->setItemIcon(i, icon);
		i++;
	}
}
