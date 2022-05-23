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

#include <utility>

namespace Preferences
{
	struct Base::Private
	{
		QString identifier;
		Action* action {nullptr};
		QByteArray geometry;
		bool isInitialized {false};

		explicit Private(QString identifier) :
			identifier(std::move(identifier)) {}
	};

	Base::Base(const QString& identifier) :
		Gui::Widget(nullptr)
	{
		m = Pimpl::make<Private>(identifier);
	}

	Base::~Base() = default;

	QString Base::identifier() const { return m->identifier; }

	void Base::setInitialized() { m->isInitialized = true; }

	bool Base::isUiInitialized() const { return m->isInitialized; }

	bool Base::hasError() const { return false; }

	QString Base::errorString() const { return {}; }

	void Base::languageChanged()
	{
		translationAction();

		if(isUiInitialized())
		{
			const auto newName = actionName();
			setWindowTitle(newName);
			retranslate();
		}
	}

	void Base::translationAction()
	{
		const auto newName = actionName();
		action()->setText(newName + "...");
	}

	void Base::closeEvent(QCloseEvent* e)
	{
		m->geometry = this->saveGeometry();
		Gui::Widget::closeEvent(e);
	}

	QAction* Base::action()
	{
		// action has to be initialized here, because pure
		// virtual get_action_name should not be called from ctor
		const auto name = actionName();
		if(!m->action)
		{
			m->action = new Action(name, this);
		}

		m->action->setText(name + "...");
		return m->action;
	}

	void Base::showEvent(QShowEvent* e)
	{
		if(!isUiInitialized())
		{
			initUi();
		}

		Gui::Widget::showEvent(e);

		if(!m->geometry.isEmpty())
		{
			restoreGeometry(m->geometry);
		}
	}
} // namespace