/* GUI_NotificationPreferences.cpp */

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

#include "GUI_NotificationPreferences.h"

#include "Components/Notification/NotificationHandler.h"
#include "Gui/Preferences/ui_GUI_NotificationPreferences.h"
#include "Utils/Language/Language.h"
#include "Utils/Settings/Settings.h"

struct GUI_NotificationPreferences::Private
{
	NotificationHandler* notificationHandler;

	explicit Private(NotificationHandler* notificationHandler) :
		notificationHandler {notificationHandler} {}
};

GUI_NotificationPreferences::GUI_NotificationPreferences(const QString& identifier,
                                                         NotificationHandler* notificationHandler) :
	Preferences::Base(identifier),
	m {Pimpl::make<Private>(notificationHandler)} {}

GUI_NotificationPreferences::~GUI_NotificationPreferences()
{
	if(ui)
	{
		delete ui;
		ui = nullptr;
	}
}

void GUI_NotificationPreferences::retranslate()
{
	ui->retranslateUi(this);
	ui->cbActivate->setText(Lang::get(Lang::Active));

	notificationsChanged();
}

void GUI_NotificationPreferences::notificationsChanged()
{
	if(isUiInitialized())
	{
		const auto notificators = m->notificationHandler->notificators();

		ui->comboNotifications->clear();

		for(const auto* notificator: notificators)
		{
			ui->comboNotifications->addItem(notificator->displayName(), notificator->identifier());
		}

		auto* currentNotificator = m->notificationHandler->currentNotificator();
		if(currentNotificator)
		{
			ui->comboNotifications->setCurrentText(currentNotificator->identifier());
		}
	}
}

bool GUI_NotificationPreferences::commit()
{
	const auto currentData = ui->comboNotifications->currentData().toString();

	SetSetting(Set::Notification_Name, currentData);
	SetSetting(Set::Notification_Timeout, ui->sbTimeout->value());
	SetSetting(Set::Notification_Show, ui->cbActivate->isChecked());

	m->notificationHandler->changeCurrentNotificator(currentData);

	return true;
}

void GUI_NotificationPreferences::revert()
{
	ui->sbTimeout->setValue(GetSetting(Set::Notification_Timeout));
	ui->cbActivate->setChecked(GetSetting(Set::Notification_Show));

	notificationsChanged();
}

QString GUI_NotificationPreferences::actionName() const { return tr("Notifications"); }

void GUI_NotificationPreferences::initUi()
{
	setupParent(this, &ui);

	revert();

	connect(m->notificationHandler, &NotificationHandler::sigNotificationsChanged,
	        this, &GUI_NotificationPreferences::notificationsChanged);
}
