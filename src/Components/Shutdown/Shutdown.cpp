/* Shutdown.cpp */

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


/* Inspired by qshutdown
 * Original-Maintainer: Christian Metscher <hakaishi@web.de>
 * Homepage: https://launchpad.net/~hakaishi
 */

#include "Components/Shutdown/Shutdown.h"
#include "Components/PlayManager/PlayManager.h"

#include "Database/Connector.h"
#include "Database/Settings.h"

#include "Interfaces/Notification/NotificationHandler.h"

#include "Utils/Utils.h"
#include "Utils/Language.h"
#include "Utils/Logger/Logger.h"

#include <QDBusInterface>
#include <QDBusConnection>
#include <QProcess>
#include <QTimer>

#ifdef WITH_SHUTDOWN

struct Shutdown::Private
{
	DB::Settings*	db=nullptr;
	QTimer*			timer=nullptr;
	QTimer*			timer_countdown=nullptr;
	PlayManagerPtr	play_manager=nullptr;

	MilliSeconds	msecs2go;
	bool			is_running;

	Private(Shutdown* parent) :
		msecs2go(0),
		is_running(false)
	{
		db = DB::Connector::instance()->settings_connector();
		play_manager = PlayManager::instance();

		timer = new QTimer(parent);
		timer_countdown = new QTimer(parent);

		timer->setInterval(100);
		timer_countdown->setInterval(50);
	}

	~Private()
	{
		timer->stop();
		timer->deleteLater();
		timer_countdown->stop();
		timer_countdown->deleteLater();
	}
};

Shutdown::Shutdown(QObject* parent) :
	QObject(parent)
{
	m = Pimpl::make<Private>(this);

	connect(m->timer, &QTimer::timeout, this, &Shutdown::timeout);
	connect(m->timer_countdown, &QTimer::timeout, this, &Shutdown::countdown_timeout);
	connect(m->play_manager, &PlayManager::sig_playlist_finished, this, &Shutdown::playlist_finished);
}

Shutdown::~Shutdown() {}

void Shutdown::shutdown_after_end()
{
	m->is_running = true;

	NotificationHandler::instance()->notify(Lang::get(Lang::Shutdown),
											   tr("Computer will shutdown after playlist has finished"),
											   Util::share_path("logo.png"));
}


bool Shutdown::is_running() const
{
	return m->is_running;
}


void Shutdown::shutdown(MilliSeconds ms)
{
	if(ms == 0){
		timeout();
		return;
	}

	m->is_running = true;
	m->msecs2go = ms;
	m->timer->start((int) ms);
	m->timer_countdown->start(1000);
	emit sig_started(ms);

	NotificationHandler::instance()->notify(Lang::get(Lang::Shutdown),
											   tr("Computer will shutdown in %1 minutes").arg(Util::cvt_ms_to_string(ms, false, true, false)),
											   Util::share_path("logo.png"));
}


void Shutdown::stop()
{
	sp_log(Log::Info) << "Shutdown cancelled";
	m->is_running = false;
	m->timer->stop();
	m->timer_countdown->stop();
	m->msecs2go = 0;

	emit sig_stopped();
}


void Shutdown::countdown_timeout()
{
	if(m->msecs2go >= 1000){
		m->msecs2go -= 1000;
	}

	m->timer_countdown->start(1000);

	emit sig_time_to_go(m->msecs2go);
	sp_log(Log::Info) << "Time to go: " << m->msecs2go;

	if(m->msecs2go % 60000 == 0){
		NotificationHandler::instance()->notify(Lang::get(Lang::Shutdown),
												   tr("Computer will shutdown in %1 minutes").arg(Util::cvt_ms_to_string(m->msecs2go, false, true, false)),
												   Util::share_path("logo.png"));
	}
}


void Shutdown::timeout()
{
	m->is_running = false;
	m->db->store_settings();

#ifdef Q_OS_WIN
	//ExitWindowsEx(

#else

	QDBusMessage response;

	QDBusInterface free_desktop_login(
				"org.freedesktop.login1",
				"/org/freedesktop/login1",
				"org.freedesktop.login1.Manager",
				 QDBusConnection::systemBus()
	);

	QDBusInterface free_desktop_console_kit(
				"org.freedesktop.ConsoleKit",
				"/org/freedesktop/ConsoleKit/Manager",
				"org.freedesktop.ConsoleKit.Manager",
				QDBusConnection::systemBus()
	);

	QDBusInterface gnome_session_manager(
				"org.gnome.SessionManager",
				"/org/gnome/SessionManager",
				"org.gnome.SessionManager",
				QDBusConnection::sessionBus()
	);

	QDBusInterface mate_session_manager(
				"org.mate.SessionManager",
				"/org/mate/SessionManager",
				"org.mate.SessionManager",
				QDBusConnection::sessionBus()
	);


	QDBusInterface kde_session_manager(
				"org.kde.ksmserver",
				"/KSMServer",
				"org.kde.KSMServerInterface",
				QDBusConnection::sessionBus()
	);


	if(QProcess::startDetached("/usr/bin/systemctl poweroff")){
		return;
	}

	bool g_pwr1 = QProcess::startDetached("gnome-power-cmd.sh shutdown");
	bool g_pwr2 = QProcess::startDetached("gnome-power-cmd shutdown");

	if(g_pwr1 || g_pwr2){
		return;
	}

	response = free_desktop_login.call("PowerOff", true);

	if(response.type() != QDBusMessage::ErrorMessage){
		return;
	}

	response = gnome_session_manager.call("RequestShutdown");
	if(response.type() != QDBusMessage::ErrorMessage){
		return;
	}

	response = gnome_session_manager.call("Shutdown");
	if(response.type() != QDBusMessage::ErrorMessage){
		return;
	}

	response = kde_session_manager.call("logout", 0, 2, 2);
	if(response.type() != QDBusMessage::ErrorMessage){
		return;
	}

	response = kde_session_manager.call("Shutdown");
	if(response.type() != QDBusMessage::ErrorMessage){
		return;
	}

	response = mate_session_manager.call("RequestShutdown");
	if(response.type() != QDBusMessage::ErrorMessage){
		return;
	}

	response = mate_session_manager.call("Shutdown");
	if(response.type() != QDBusMessage::ErrorMessage){
		return;
	}

	response = free_desktop_console_kit.call("Stop");
	if(response.type() != QDBusMessage::ErrorMessage){
		return;
	}

	if(QProcess::startDetached("sudo shutdown -P now")){
		return;
	}

	if(QProcess::startDetached("sudo shutdown -h -P now")){
		return;
	}

	sp_log(Log::Warning) << "Sorry, power off is not possible";

#endif
}


void Shutdown::playlist_finished()
{
	if( m->is_running ){
		timeout();
	}
}

#endif
