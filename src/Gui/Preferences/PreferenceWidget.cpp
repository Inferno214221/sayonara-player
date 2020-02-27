/* PreferenceWidgetInterface.cpp */

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

#include "PreferenceWidget.h"
#include "PreferenceAction.h"

using Preferences::Base;
using Preferences::Action;

struct Base::Private
{
	QString				identifier;
	Action*				action=nullptr;
	QByteArray			geometry;
	bool				is_initialized;

	Private(const QString& identifier) :
		identifier(identifier),
		is_initialized(false)
	{}
};

Base::Base(const QString& identifier) :
	Gui::Widget(nullptr)
{
	m = Pimpl::make<Private>(identifier);
}

Base::~Base() = default;

QString Base::identifier() const
{
	return m->identifier;
}

void Base::setInitialized()
{
	m->is_initialized = true;
}

void Base::languageChanged()
{
	translationAction();

	if(!isUiInitialized()){
		return;
	}

	QString new_name = actionName();
	this->setWindowTitle(new_name);

	retranslate();
}

void Base::translationAction()
{
	QString new_name = this->actionName();
	action()->setText(new_name + "...");
}

void Base::showEvent(QShowEvent* e)
{
	if(!isUiInitialized()){
		initUi();
	}

	Gui::Widget::showEvent(e);

	if(!m->geometry.isEmpty()){
		this->restoreGeometry(m->geometry);
	}
}

void Base::closeEvent(QCloseEvent* e)
{
	m->geometry = this->saveGeometry();
	Gui::Widget::closeEvent(e);
}

bool Base::isUiInitialized() const
{
	return m->is_initialized;
}

QAction* Base::action()
{
	// action has to be initialized here, because pure
	// virtual get_action_name should not be called from ctor
	QString name = actionName();
	if(!m->action){
		m->action = new Action(name, this);
	}

	m->action->setText(name + "...");
	return m->action;
}

bool Base::hasError() const
{
	return false;
}

QString Base::errorString() const
{
	return QString();
}
