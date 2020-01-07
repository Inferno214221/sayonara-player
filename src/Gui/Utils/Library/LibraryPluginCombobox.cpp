/* LibraryPluginCombobox.cpp */

/* Copyright (C) 2011-2020  Lucio Carreras
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

#include "Interfaces/Library/LibraryContainer.h"
#include "Components/LibraryManagement/LibraryPluginHandler.h"

#include "Utils/Algorithm.h"
#include "Utils/Logger/Logger.h"

#include <QList>
#include <QAction>
#include <QSize>
#include <QFontMetrics>

using Library::Container;
using Library::PluginHandler;
using Library::PluginCombobox;
using Library::PluginComboBoxDelegate;

namespace Algorithm=Util::Algorithm;

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
	connect(lph, &PluginHandler::sig_libraries_changed, this, &PluginCombobox::setup_actions);
	connect(lph, &PluginHandler::sig_current_library_changed, this, &PluginCombobox::current_library_changed);

	connect(this, combo_activated_int, this, &PluginCombobox::current_index_changed);

	setup_actions();
	setCurrentText(text);
}

PluginCombobox::~PluginCombobox() = default;

void PluginCombobox::setup_actions()
{
	QFontMetrics fm(this->font());

	this->clear();

	const QList<Container*> libraries = PluginHandler::instance()->get_libraries(true);
	for(const Container* container : libraries)
	{
		QPixmap pm = container->icon().scaled(
					this->iconSize(),
					Qt::KeepAspectRatio,
					Qt::SmoothTransformation
		);

		QString display_name = fm.elidedText(container->display_name(), Qt::TextElideMode::ElideRight, 200);
		this->addItem(QIcon(pm), display_name, container->name());
	}

	this->insertSeparator(1);
	this->setItemIcon(1, QIcon());

	current_library_changed();
}

void PluginCombobox::action_triggered(bool b)
{
	if(!b){
		return;
	}

	auto* action = static_cast<QAction*>(sender());
	QString name = action->data().toString();

	PluginHandler::instance()->set_current_library(name);
	for(QAction* library_action : Algorithm::AsConst(m->actions))
	{
		if(library_action == action){
			continue;
		}

		library_action->setChecked(false);
	}
}

void PluginCombobox::current_library_changed()
{
	Container* current_library = PluginHandler::instance()->current_library();
	if(!current_library) {
		return;
	}

	QString name = current_library->name();
	for(int i=0; i<this->count(); i++)
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

void PluginCombobox::current_index_changed(int index)
{
	PluginHandler::instance()->set_current_library(index - 2);
}

void PluginCombobox::language_changed()
{
	if(!m){
		return;
	}

	setup_actions();
}

void PluginCombobox::skin_changed()
{
	if(!m){
		return;
	}

	const QList<Container*> libraries = PluginHandler::instance()->get_libraries(true);
	int i=0;

	for(const Container* container : libraries)
	{
		QPixmap pm = container->icon().scaled(
					this->iconSize(),
					Qt::KeepAspectRatio,
					Qt::SmoothTransformation
		);

		if(this->itemData(i, Qt::DisplayRole).toString().isEmpty())
		{
			i++;
		}

		this->setItemIcon(i, QIcon(pm));
		i++;
	}
}
