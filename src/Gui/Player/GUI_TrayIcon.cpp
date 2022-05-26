/* GUI_TrayIcon.cpp */

/* Copyright (C) 2011-2020 Michael Lugmair (Lucio Carreras)  gleugner
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

#include "GUI_TrayIcon.h"
#include "Gui/Utils/Icons.h"
#include "Gui/Utils/Style.h"

#include "Interfaces/PlayManager.h"

#include "Utils/MetaData/MetaData.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Language/Language.h"

#include <QAction>
#include <QMenu>
#include <QEvent>
#include <QWheelEvent>
#include <QTimer>

using namespace Gui;

struct TrayIconContextMenu::Private
{
	PlayManager* playManager;

	QAction* bwdAction;
	QAction* closeAction;
	QAction* currentSongAction;
	QAction* fwdAction;
	QAction* muteAction;
	QAction* playAction;
	QAction* showAction;
	QAction* stopAction;

	Private(PlayManager* playManager, TrayIconContextMenu* parent) :
		playManager(playManager),
		bwdAction {new QAction(parent)},
		closeAction {new QAction(parent)},
		currentSongAction {new QAction(parent)},
		fwdAction {new QAction(parent)},
		muteAction {new QAction(parent)},
		playAction {new QAction(parent)},
		showAction {new QAction(parent)},
		stopAction {new QAction(parent)} {}
};

TrayIconContextMenu::TrayIconContextMenu(PlayManager* playManager, GUI_TrayIcon* /*parent*/) :
	Gui::WidgetTemplate<QMenu>(nullptr)
{
	m = Pimpl::make<Private>(playManager, this);

	this->addAction(m->playAction);
	this->addAction(m->stopAction);
	this->addSeparator();
	this->addAction(m->fwdAction);
	this->addAction(m->bwdAction);
	this->addSeparator();
	this->addAction(m->muteAction);
	this->addSeparator();
	this->addAction(m->currentSongAction);
	this->addSeparator();
	this->addAction(m->showAction);
	this->addAction(m->closeAction);

	connect(m->playAction, &QAction::triggered, m->playManager, &PlayManager::playPause);
	connect(m->fwdAction, &QAction::triggered, m->playManager, &PlayManager::next);
	connect(m->bwdAction, &QAction::triggered, m->playManager, &PlayManager::previous);
	connect(m->stopAction, &QAction::triggered, m->playManager, &PlayManager::stop);

	connect(m->muteAction, &QAction::triggered, this, &TrayIconContextMenu::muteClicked);
	connect(m->currentSongAction, &QAction::triggered, this, &TrayIconContextMenu::currentSongClicked);

	connect(m->showAction, &QAction::triggered, this, &TrayIconContextMenu::sigShowClicked);
	connect(m->closeAction, &QAction::triggered, this, &TrayIconContextMenu::sigCloseClicked);

	connect(m->playManager, &PlayManager::sigMuteChanged, this, &TrayIconContextMenu::muteChanged);
	connect(m->playManager, &PlayManager::sigPlaystateChanged, this, &TrayIconContextMenu::playstateChanged);

	muteChanged(m->playManager->isMuted());
	playstateChanged(m->playManager->playstate());
}

TrayIconContextMenu::~TrayIconContextMenu() = default;

void TrayIconContextMenu::setForwardEnabled(bool b)
{
	m->fwdAction->setEnabled(b);
}

void TrayIconContextMenu::setDisplayNames()
{
	m->playAction->setText(Lang::get(Lang::PlayPause));
	m->fwdAction->setText(Lang::get(Lang::NextTrack));
	m->bwdAction->setText(Lang::get(Lang::PreviousTrack));
	m->stopAction->setText(Lang::get(Lang::Stop));

	if(m->playManager->isMuted())
	{
		m->muteAction->setText(Lang::get(Lang::MuteOff));
	}

	else
	{
		m->muteAction->setText(Lang::get(Lang::MuteOn));
	}

	m->closeAction->setText(Lang::get(Lang::Quit));
	m->showAction->setText(Lang::get(Lang::Show));
	m->currentSongAction->setText(tr("Current song"));
}

void TrayIconContextMenu::muteClicked()
{
	m->playManager->toggleMute();
}

void TrayIconContextMenu::currentSongClicked()
{
	auto* nh = NotificationHandler::instance();
	nh->notify(m->playManager->currentTrack());
}

void TrayIconContextMenu::muteChanged(bool muted)
{
	using namespace Gui;
	if(!muted)
	{
		m->muteAction->setIcon(Icons::icon(Icons::VolMute));
		m->muteAction->setText(Lang::get(Lang::MuteOn));
	}

	else
	{
		m->muteAction->setIcon(Icons::icon(Icons::Vol3));
		m->muteAction->setText(Lang::get(Lang::MuteOff));
	}
}

void TrayIconContextMenu::playstateChanged(PlayState state)
{
	using namespace Gui;
	if(state == PlayState::Playing)
	{
		m->playAction->setIcon(Icons::icon(Icons::Pause));
		m->playAction->setText(Lang::get(Lang::Pause));
	}

	else
	{
		m->playAction->setIcon(Icons::icon(Icons::Play));
		m->playAction->setText(Lang::get(Lang::Play));
	}
}

void TrayIconContextMenu::languageChanged()
{
	setDisplayNames();
}

void TrayIconContextMenu::skinChanged()
{
	using namespace Gui;

	m->stopAction->setIcon(Icons::icon(Icons::Stop));
	m->bwdAction->setIcon(Icons::icon(Icons::Previous));
	m->fwdAction->setIcon(Icons::icon(Icons::Next));
	m->currentSongAction->setIcon(Icons::icon(Icons::Info));
	m->closeAction->setIcon(Icons::icon(Icons::Exit));

	muteChanged(m->playManager->isMuted());
	playstateChanged(m->playManager->playstate());
}

struct GUI_TrayIcon::Private
{
	PlayManager* playManager;
	TrayIconContextMenu* contextMenu = nullptr;
	QTimer* timer = nullptr;

	explicit Private(PlayManager* playManager) :
		playManager(playManager) {}
};

GUI_TrayIcon::GUI_TrayIcon(PlayManager* playManager, QObject* parent) :
	QSystemTrayIcon(parent)
{
	m = Pimpl::make<Private>(playManager);

	auto* notificationHandler = NotificationHandler::instance();
	notificationHandler->registerNotificator(this);

	connect(m->playManager, &PlayManager::sigPlaystateChanged, this, &GUI_TrayIcon::playstateChanged);

	initContextMenu();
	playstateChanged(m->playManager->playstate());

	ListenSetting(Set::Player_ShowTrayIcon, GUI_TrayIcon::showTrayIconChanged);
}

GUI_TrayIcon::~GUI_TrayIcon() = default;

void GUI_TrayIcon::initContextMenu()
{
	if(!m->contextMenu)
	{
		m->contextMenu = new TrayIconContextMenu(m->playManager, this);

		connect(m->contextMenu, &TrayIconContextMenu::sigCloseClicked, this, &GUI_TrayIcon::sigCloseClicked);
		connect(m->contextMenu, &TrayIconContextMenu::sigShowClicked, this, &GUI_TrayIcon::sigShowClicked);

		setContextMenu(m->contextMenu);
		m->contextMenu->setDisplayNames();
	}
}

bool GUI_TrayIcon::event(QEvent* e)
{
	if(e->type() == QEvent::Wheel)
	{
		auto* wheelEvent = dynamic_cast<QWheelEvent*>(e);
		const auto delta = wheelEvent->angleDelta().y();
		emit sigWheelChanged(delta);
	}

	return QSystemTrayIcon::event(e);
}

void GUI_TrayIcon::notify(const MetaData& md)
{
	if(isSystemTrayAvailable())
	{
		const auto timeout = GetSetting(Set::Notification_Timeout);
		const auto message = QString("%1 %2 %3")
			.arg(md.title())
			.arg(Lang::get(Lang::By))
			.arg(md.artist());

		showMessage("Sayonara", message, QSystemTrayIcon::Information, timeout);
	}
}

void GUI_TrayIcon::notify(const QString& title, const QString& message, const QString& /*imagePath*/)
{
	if(isSystemTrayAvailable())
	{
		const auto timeout = GetSetting(Set::Notification_Timeout);
		showMessage(title, message, QSystemTrayIcon::Information, timeout);
	}
}

// dbus
QString GUI_TrayIcon::name() const { return "Standard"; }

QString GUI_TrayIcon::displayName() const { return Lang::get(Lang::Default); }

void GUI_TrayIcon::playstateChanged(PlayState state)
{
	using namespace Gui;
	const auto icon = (state == PlayState::Playing)
	                  ? Icons::icon(Icons::Play, Icons::ForceSayonaraIcon)
	                  : Icons::icon(Icons::Pause, Icons::ForceSayonaraIcon);

	setIcon(icon);
}

[[maybe_unused]] void GUI_TrayIcon::setForwardEnabled(bool b)
{
	if(m->contextMenu)
	{
		m->contextMenu->setForwardEnabled(b);
	}
}

void GUI_TrayIcon::showTrayIconChanged()
{
	const auto showTrayIcon = GetSetting(Set::Player_ShowTrayIcon);
	setVisible(showTrayIcon);
}
