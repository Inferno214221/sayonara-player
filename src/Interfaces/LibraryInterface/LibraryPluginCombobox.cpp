/* LibraryPluginCombobox.cpp */

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

#include "LibraryPluginCombobox.h"
#include "LibraryPluginHandler.h"
#include "LibraryContainer/LibraryContainer.h"

#include "Utils/Utils.h"

#include <QList>
#include <QAction>
#include <QSize>

using Library::Container;
using Library::PluginHandler;
using Library::PluginCombobox;

struct PluginCombobox::Private
{
	PluginHandler* lph=nullptr;
	QList<QAction*> actions;

	Private()
	{
		lph = PluginHandler::instance();
	}
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

	connect(m->lph, &PluginHandler::sig_libraries_changed, this, &PluginCombobox::setup_actions);
	connect(m->lph, &PluginHandler::sig_current_library_changed, this, &PluginCombobox::current_library_changed);

	setup_actions();
	setCurrentText(text);
}

PluginCombobox::~PluginCombobox() {}

void PluginCombobox::setup_actions()
{
	this->clear();

	QList<Container*> libraries = m->lph->get_libraries();

	for(const Container* container : libraries)
	{
		QPixmap pm = container->icon().scaled(
					this->iconSize(),
					Qt::KeepAspectRatio,
					Qt::SmoothTransformation
		);

		this->addItem(QIcon(pm), container->display_name(), container->name());
	}

	current_library_changed(m->lph->current_library()->name());
}

void PluginCombobox::action_triggered(bool b)
{
	if(!b){
		return;
	}

	QAction* action = static_cast<QAction*>(sender());
	QString name = action->data().toString();

	PluginHandler::instance()->set_current_library(name);
	for(QAction* library_action : Util::AsConst(m->actions))
	{
		if(library_action == action){
			continue;
		}

		library_action->setChecked(false);
	}
}

void PluginCombobox::current_library_changed(const QString& name)
{
	for(int i=0; i<this->count(); i++) {
		if(this->itemData(i).toString().compare(name) == 0){
			this->setCurrentIndex(i);
			break;
		}
	}
}

void PluginCombobox::language_changed()
{
	if(!m){
		return;
	}

	QList<Container*> libraries = m->lph->get_libraries();
	int i=0;

	for(const Container* container : libraries)
	{
		this->setItemText(i, container->display_name());
		i++;
	}
}

void PluginCombobox::skin_changed()
{
	if(!m){
		return;
	}

	QList<Container*> libraries = m->lph->get_libraries();
	int i=0;

	for(const Container* container : libraries)
	{
		QPixmap pm = container->icon().scaled(
					this->iconSize(),
					Qt::KeepAspectRatio,
					Qt::SmoothTransformation
		);

		this->setItemIcon(i, QIcon(pm));
		i++;
	}
}
