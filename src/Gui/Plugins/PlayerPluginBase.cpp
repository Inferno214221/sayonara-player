/* PlayerPlugin.cpp */

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

#include "PlayerPluginBase.h"
#include "Gui/Utils/Shortcuts/Shortcut.h"
#include "Gui/Utils/Shortcuts/ShortcutHandler.h"
#include "Utils/Settings/Settings.h"

#include <QAction>
#include <QLayout>
#include <QCloseEvent>

using PlayerPlugin::Base;

struct Base::Private
{
	QAction*	pluginAction=nullptr;
	bool		isInitialized;

	Private() :
		pluginAction(new QAction(nullptr)),
		isInitialized(false)
	{
		pluginAction->setCheckable(true);
	}

	~Private()
	{
		delete pluginAction; pluginAction = nullptr;
	}
};

Base::Base(QWidget* parent) :
	Widget(parent)
{
	m = Pimpl::make<Private>();

	setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
	connect(m->pluginAction, &QAction::triggered, this, &Base::actionTriggered);

	hide();
}

Base::~Base() = default;

bool Base::hasTitle() const
{
	return true;
}

bool Base::hasLoadingBar() const
{
	return false;
}

QAction* Base::pluginAction() const
{
	m->pluginAction->setText( this->displayName() );
	return m->pluginAction;
}

void Base::finalizeInitialization()
{
	QLayout* widget_layout = layout();
	if(widget_layout)
	{
		int bottom = hasLoadingBar() ? 10 : 3;
		widget_layout->setContentsMargins(3, 3, 3, bottom);
	}

	if(parentWidget())
	{
		ShortcutHandler* sch = ShortcutHandler::instance();
		sch->shortcut(ShortcutIdentifier::ClosePlugin).connect(this, parentWidget(), SLOT(close()), Qt::WidgetWithChildrenShortcut);
	}

	setUiInitialized();
	retranslate();
}

void Base::assignUiVariables() {}

void Base::languageChanged()
{
	if(isUiInitialized()){
		retranslate();
	}
}

bool Base::isUiInitialized() const
{
	return m->isInitialized;
}

void Base::setUiInitialized()
{
	m->isInitialized = true;
}

void Base::showEvent(QShowEvent* e)
{
	if(!isUiInitialized()){
		initUi();
	}

	Widget::showEvent(e);

	m->pluginAction->setChecked(true);

	emit sigOpened();
}

void Base::closeEvent(QCloseEvent* e)
{
	Widget::closeEvent(e);
	actionTriggered(false);

	emit sigClosed();
}

void Base::actionTriggered(bool b)
{
	m->pluginAction->setChecked(b);

	emit sigActionTriggered(b);

	skinChanged();
}
