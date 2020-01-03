/* GUI_TrayIcon.cpp */

/* Copyright (C) 2011-2019 Lucio Carreras  gleugner
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
	QAction*	show_action=nullptr;
	QAction*	close_action=nullptr;
	QAction*	play_action=nullptr;
	QAction*	stop_action=nullptr;
	QAction*	mute_action=nullptr;
	QAction*	fwd_action=nullptr;
	QAction*	bwd_action=nullptr;
	QAction*	cur_song_action=nullptr;

	Private(TrayIconContextMenu* parent)
	{
		play_action = new QAction(parent);
		stop_action = new QAction(parent);
		bwd_action = new QAction(parent);
		fwd_action = new QAction(parent);
		mute_action = new QAction(parent);
		show_action = new QAction(parent);
		cur_song_action = new QAction(parent);
		close_action = new QAction(parent);
	}
};

TrayIconContextMenu::TrayIconContextMenu(QWidget* parent) :
	Gui::WidgetTemplate<QMenu>(parent)
{
	m = Pimpl::make<Private>(this);

	this->addAction(m->play_action);
	this->addAction(m->stop_action);
	this->addSeparator();
	this->addAction(m->fwd_action);
	this->addAction(m->bwd_action);
	this->addSeparator();
	this->addAction(m->mute_action);
	this->addSeparator();
	this->addAction(m->cur_song_action);
	this->addSeparator();
	this->addAction(m->show_action);
	this->addAction(m->close_action);

	auto* pm = PlayManager::instance();
	connect(m->play_action, &QAction::triggered, pm, &PlayManager::play_pause);
	connect(m->fwd_action, &QAction::triggered, pm, &PlayManager::next);
	connect(m->bwd_action, &QAction::triggered, pm, &PlayManager::previous);
	connect(m->stop_action, &QAction::triggered, pm, &PlayManager::stop);

	connect(m->mute_action, &QAction::triggered, this, &TrayIconContextMenu::mute_clicked);
	connect(m->cur_song_action, &QAction::triggered, this, &TrayIconContextMenu::current_song_clicked);

	connect(m->show_action, &QAction::triggered, this, &TrayIconContextMenu::sig_show_clicked);
	connect(m->close_action, &QAction::triggered, this, &TrayIconContextMenu::sig_close_clicked);

	connect(pm, &PlayManager::sig_mute_changed, this, &TrayIconContextMenu::mute_changed);
	connect(pm, &PlayManager::sig_playstate_changed, this, &TrayIconContextMenu::playstate_changed);

	language_changed();
	skin_changed();

	mute_changed(pm->is_muted());
	playstate_changed(pm->playstate());

	language_changed();
	skin_changed();
}

TrayIconContextMenu::~TrayIconContextMenu() = default;

void TrayIconContextMenu::set_enable_fwd(bool b)
{
	m->fwd_action->setEnabled(b);
}

void TrayIconContextMenu::mute_clicked()
{
	auto* pm = PlayManager::instance();
	bool mute = pm->is_muted();

	pm->set_muted(!mute);
}

void TrayIconContextMenu::current_song_clicked()
{
	PlayManager* pm = PlayManager::instance();
	NotificationHandler* nh = NotificationHandler::instance();

	nh->notify(pm->current_track());
}

void TrayIconContextMenu::mute_changed(bool muted)
{
	using namespace Gui;
	if(!muted) {
		m->mute_action->setIcon(Icons::icon(Icons::VolMute));
		m->mute_action->setText(Lang::get(Lang::MuteOn));
	}

	else {
		m->mute_action->setIcon(Icons::icon(Icons::Vol3));
		m->mute_action->setText(Lang::get(Lang::MuteOff));
	}
}

void TrayIconContextMenu::playstate_changed(PlayState state)
{
	using namespace Gui;
	if(state == PlayState::Playing)
	{
		m->play_action->setIcon(Icons::icon(Icons::Pause));
		m->play_action->setText(Lang::get(Lang::Pause));
	}

	else
	{
		m->play_action->setIcon(Icons::icon(Icons::Play));
		m->play_action->setText(Lang::get(Lang::Play));
	}
}


void TrayIconContextMenu::language_changed()
{
	auto* pm = PlayManager::instance();

	m->play_action->setText(Lang::get(Lang::PlayPause));
	m->fwd_action->setText(Lang::get(Lang::NextTrack));
	m->bwd_action->setText(Lang::get(Lang::PreviousTrack));
	m->stop_action->setText(Lang::get(Lang::Stop));

	if(pm->is_muted()){
		m->mute_action->setText(Lang::get(Lang::MuteOff));
	}

	else {
		m->mute_action->setText(Lang::get(Lang::MuteOn));
	}

	m->close_action->setText(Lang::get(Lang::Quit));
	m->show_action->setText(Lang::get(Lang::Show));
	m->cur_song_action->setText(tr("Current song"));
}


void TrayIconContextMenu::skin_changed()
{
	QString stylesheet = Style::current_style();
	this->setStyleSheet(stylesheet);

	using namespace Gui;

	m->stop_action->setIcon(Icons::icon(Icons::Stop));
	m->bwd_action->setIcon(Icons::icon(Icons::Previous));
	m->fwd_action->setIcon(Icons::icon(Icons::Next));
	m->cur_song_action->setIcon(Icons::icon(Icons::Info));
	m->close_action->setIcon(Icons::icon(Icons::Exit));

	PlayManager* pm = PlayManager::instance();
	mute_changed(pm->is_muted());
	playstate_changed(pm->playstate());
}


struct GUI_TrayIcon::Private
{
	TrayIconContextMenu*	context_menu=nullptr;
	QTimer*					timer=nullptr;
};

GUI_TrayIcon::GUI_TrayIcon (QObject *parent) :
	QSystemTrayIcon(parent),
	NotificationInterface()
{
	m = Pimpl::make<Private>();

	auto* nh = NotificationHandler::instance();
	nh->register_notificator(this);

	auto* pm = PlayManager::instance();
	connect(pm, &PlayManager::sig_playstate_changed, this, &GUI_TrayIcon::playstate_changed);

	init_context_menu();
	playstate_changed(pm->playstate());

	ListenSetting(Set::Player_ShowTrayIcon, GUI_TrayIcon::s_show_tray_icon_changed);
}

GUI_TrayIcon::~GUI_TrayIcon() = default;

void GUI_TrayIcon::init_context_menu()
{
	if(m->context_menu){
		return;
	}

	m->context_menu = new TrayIconContextMenu();

	connect(m->context_menu, &TrayIconContextMenu::sig_close_clicked, this, &GUI_TrayIcon::sig_close_clicked);
	connect(m->context_menu, &TrayIconContextMenu::sig_show_clicked, this, &GUI_TrayIcon::sig_show_clicked);

	setContextMenu(m->context_menu);
}


bool GUI_TrayIcon::event(QEvent* e)
{
	if (e->type() == QEvent::Wheel)
	{
		auto* wheel_event = static_cast<QWheelEvent*>(e);

		if(wheel_event){
			emit sig_wheel_changed( wheel_event->delta() );
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


void GUI_TrayIcon::notify(const QString &title, const QString &message, const QString &image_path)
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

QString GUI_TrayIcon::display_name() const
{
	return Lang::get(Lang::Default);
}

void GUI_TrayIcon::playstate_changed(PlayState state)
{
	using namespace Gui;

	if(state == PlayState::Playing){
		setIcon(Icons::icon(Icons::Play, Icons::ForceSayonaraIcon));
	}

	else {
		setIcon(Icons::icon(Icons::Pause, Icons::ForceSayonaraIcon));
	}
}

void GUI_TrayIcon::set_enable_fwd(bool b)
{
	if(m->context_menu){
		m->context_menu->set_enable_fwd(b);
	}
}

void GUI_TrayIcon::s_show_tray_icon_changed()
{
	bool show_tray_icon = GetSetting(Set::Player_ShowTrayIcon);
	this->setVisible(show_tray_icon);
}
