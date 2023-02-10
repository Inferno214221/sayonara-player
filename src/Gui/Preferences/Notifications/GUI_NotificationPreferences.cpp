/* GUI_NotificationPreferences.cpp */

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

#include "GUI_NotificationPreferences.h"

#include "Components/Notification/NotificationHandler.h"
#include "Gui/Preferences/ui_GUI_NotificationPreferences.h"
#include "Utils/Language/Language.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Language/Language.h"

GUI_NotificationPreferences::GUI_NotificationPreferences(const QString& identifier) :
	Preferences::Base(identifier) {}

GUI_NotificationPreferences::~GUI_NotificationPreferences()
{
	if(ui)
	{
		delete ui; ui=nullptr;
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
	if(!isUiInitialized()){
		return;
	}

	NotificationHandler* nh = NotificationHandler::instance();
	NotificatonList notifications = nh->notificators();

	ui->comboNotifications->clear();

	for(const NotificationInterface* notification : notifications)
	{
		ui->comboNotifications->addItem(notification->displayName(), notification->name());
	}

	ui->comboNotifications->setCurrentIndex(nh->currentIndex());
}


bool GUI_NotificationPreferences::commit()
{
	NotificationHandler* nh = NotificationHandler::instance();

	bool active =       ui->cbActivate->isChecked();
	int timeout =       ui->sbTimeout->value();
	QString cur_data =  ui->comboNotifications->currentData().toString();

	SetSetting(Set::Notification_Name, cur_data);
	SetSetting(Set::Notification_Timeout, timeout);
	SetSetting(Set::Notification_Show, active);

	nh->notificatorChanged(cur_data);

	return true;
}

void GUI_NotificationPreferences::revert()
{
	int timeout = GetSetting(Set::Notification_Timeout);
	int active = GetSetting(Set::Notification_Show);

	ui->sbTimeout->setValue(timeout);
	ui->cbActivate->setChecked(active);

	notificationsChanged();
}


QString GUI_NotificationPreferences::actionName() const
{
	return tr("Notifications");
}

void GUI_NotificationPreferences::initUi()
{
	setupParent(this, &ui);

	NotificationHandler* nh = NotificationHandler::instance();

	revert();

	connect(nh,	&NotificationHandler::sigNotificationsChanged,
			this, &GUI_NotificationPreferences::notificationsChanged);
}
