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
#include "Utils/MetaData/MetaData.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Language/Language.h"

#include "Components/PlayManager/PlayManager.h"

#include <QAction>
#include <QMenu>
#include <QEvent>
#include <QWheelEvent>
#include <QHoverEvent>
#include <QIcon>
#include <QTimer>

using namespace Gui;

struct TrayIconContextMenu::Private
{
	QAction*	showAction=nullptr;
	QAction*	closeAction=nullptr;
	QAction*	playAction=nullptr;
	QAction*	stopAction=nullptr;
	QAction*	muteAction=nullptr;
	QAction*	fwdAction=nullptr;
	QAction*	bwdAction=nullptr;
	QAction*	currentSongAction=nullptr;

	Private(TrayIconContextMenu* parent)
	{
		playAction = new QAction(parent);
		stopAction = new QAction(parent);
		bwdAction = new QAction(parent);
		fwdAction = new QAction(parent);
		muteAction = new QAction(parent);
		showAction = new QAction(parent);
		currentSongAction = new QAction(parent);
		closeAction = new QAction(parent);
	}
};

TrayIconContextMenu::TrayIconContextMenu(QWidget* parent) :
	Gui::WidgetTemplate<QMenu>(parent)
{
	m = Pimpl::make<Private>(this);

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

	auto* pm = PlayManager::instance();
	connect(m->playAction, &QAction::triggered, pm, &PlayManager::playPause);
	connect(m->fwdAction, &QAction::triggered, pm, &PlayManager::next);
	connect(m->bwdAction, &QAction::triggered, pm, &PlayManager::previous);
	connect(m->stopAction, &QAction::triggered, pm, &PlayManager::stop);

	connect(m->muteAction, &QAction::triggered, this, &TrayIconContextMenu::muteClicked);
	connect(m->currentSongAction, &QAction::triggered, this, &TrayIconContextMenu::currentSongClicked);

	connect(m->showAction, &QAction::triggered, this, &TrayIconContextMenu::sigShowClicked);
	connect(m->closeAction, &QAction::triggered, this, &TrayIconContextMenu::sigCloseClicked);

	connect(pm, &PlayManager::sigMuteChanged, this, &TrayIconContextMenu::muteChanged);
	connect(pm, &PlayManager::sigPlaystateChanged, this, &TrayIconContextMenu::playstateChanged);

	languageChanged();
	skinChanged();

	muteChanged(pm->isMuted());
	playstateChanged(pm->playstate());

	languageChanged();
	skinChanged();
}

TrayIconContextMenu::~TrayIconContextMenu() = default;

void TrayIconContextMenu::setForwardEnabled(bool b)
{
	m->fwdAction->setEnabled(b);
}

void TrayIconContextMenu::muteClicked()
{
	PlayManager::instance()->toggleMute();
}

void TrayIconContextMenu::currentSongClicked()
{
	NotificationHandler* nh = NotificationHandler::instance();

	nh->notify(PlayManager::instance()->currentTrack());
}

void TrayIconContextMenu::muteChanged(bool muted)
{
	using namespace Gui;
	if(!muted) {
		m->muteAction->setIcon(Icons::icon(Icons::VolMute));
		m->muteAction->setText(Lang::get(Lang::MuteOn));
	}

	else {
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
	auto* pm = PlayManager::instance();

	m->playAction->setText(Lang::get(Lang::PlayPause));
	m->fwdAction->setText(Lang::get(Lang::NextTrack));
	m->bwdAction->setText(Lang::get(Lang::PreviousTrack));
	m->stopAction->setText(Lang::get(Lang::Stop));

	if(pm->isMuted()){
		m->muteAction->setText(Lang::get(Lang::MuteOff));
	}

	else {
		m->muteAction->setText(Lang::get(Lang::MuteOn));
	}

	m->closeAction->setText(Lang::get(Lang::Quit));
	m->showAction->setText(Lang::get(Lang::Show));
	m->currentSongAction->setText(tr("Current song"));
}

void TrayIconContextMenu::skinChanged()
{
	QString stylesheet = Style::currentStyle();
	this->setStyleSheet(stylesheet);

	using namespace Gui;

	m->stopAction->setIcon(Icons::icon(Icons::Stop));
	m->bwdAction->setIcon(Icons::icon(Icons::Previous));
	m->fwdAction->setIcon(Icons::icon(Icons::Next));
	m->currentSongAction->setIcon(Icons::icon(Icons::Info));
	m->closeAction->setIcon(Icons::icon(Icons::Exit));

	PlayManager* pm = PlayManager::instance();
	muteChanged(pm->isMuted());
	playstateChanged(pm->playstate());
}

struct GUI_TrayIcon::Private
{
	TrayIconContextMenu*	contextMenu=nullptr;
	QTimer*					timer=nullptr;
};

GUI_TrayIcon::GUI_TrayIcon (QObject* parent) :
	QSystemTrayIcon(parent),
	NotificationInterface()
{
	m = Pimpl::make<Private>();

	auto* nh = NotificationHandler::instance();
	nh->registerNotificator(this);

	auto* pm = PlayManager::instance();
	connect(pm, &PlayManager::sigPlaystateChanged, this, &GUI_TrayIcon::playstateChanged);

	initContextMenu();
	playstateChanged(pm->playstate());

	ListenSetting(Set::Player_ShowTrayIcon, GUI_TrayIcon::showTrayIconChanged);
}

GUI_TrayIcon::~GUI_TrayIcon() = default;

void GUI_TrayIcon::initContextMenu()
{
	if(m->contextMenu){
		return;
	}

	m->contextMenu = new TrayIconContextMenu();

	connect(m->contextMenu, &TrayIconContextMenu::sigCloseClicked, this, &GUI_TrayIcon::sigCloseClicked);
	connect(m->contextMenu, &TrayIconContextMenu::sigShowClicked, this, &GUI_TrayIcon::sigShowClicked);

	setContextMenu(m->contextMenu);
}


bool GUI_TrayIcon::event(QEvent* e)
{
	if (e->type() == QEvent::Wheel)
	{
		auto* wheel_event = static_cast<QWheelEvent*>(e);

		if(wheel_event){
			emit sigWheelChanged( wheel_event->delta() );
		}
	}

	return QSystemTrayIcon::event(e);
}

void GUI_TrayIcon::notify(const MetaData& md)
{
	if ( !isSystemTrayAvailable() ) {
		return;
	}

	int timeout = GetSetting(Set::Notification_Timeout);

	QString msg = md.title() + " " + Lang::get(Lang::By).space() + md.artist();

	showMessage("Sayonara", msg, QSystemTrayIcon::Information, timeout);
}


void GUI_TrayIcon::notify(const QString& title, const QString& message, const QString& image_path)
{
	Q_UNUSED(image_path)

	if(!isSystemTrayAvailable()){
		return;
	}

	int timeout = GetSetting(Set::Notification_Timeout);

	showMessage(title, message, QSystemTrayIcon::Information, timeout);
}


// dbus
QString GUI_TrayIcon::name() const
{
	return "Standard";
}

QString GUI_TrayIcon::displayName() const
{
	return Lang::get(Lang::Default);
}

void GUI_TrayIcon::playstateChanged(PlayState state)
{
	using namespace Gui;

	if(state == PlayState::Playing){
		setIcon(Icons::icon(Icons::Play, Icons::ForceSayonaraIcon));
	}

	else {
		setIcon(Icons::icon(Icons::Pause, Icons::ForceSayonaraIcon));
	}
}

void GUI_TrayIcon::setForwardEnabled(bool b)
{
	if(m->contextMenu){
		m->contextMenu->setForwardEnabled(b);
	}
}

void GUI_TrayIcon::showTrayIconChanged()
{
	bool show_tray_icon = GetSetting(Set::Player_ShowTrayIcon);
	this->setVisible(show_tray_icon);
}
