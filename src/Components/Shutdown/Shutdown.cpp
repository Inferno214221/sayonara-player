/* Shutdown.cpp */

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
#include "Utils/Language/Language.h"
#include "Utils/Logger/Logger.h"

#include <QDBusInterface>
#include <QDBusConnection>
#include <QProcess>
#include <QTimer>

#ifdef SAYONARA_WITH_SHUTDOWN

struct Shutdown::Private
{
	QString logoPath;
	DB::Settings* db = nullptr;
	QTimer* timer = nullptr;
	QTimer* timerCountdown = nullptr;
	PlayManager* playManager = nullptr;

	MilliSeconds msecs2go;
	bool isRunning;

	Private(Shutdown* parent) :
		logoPath(":/Icons/logo.png"),
		timer(new QTimer(parent)),
		timerCountdown(new QTimer(parent)),
		playManager(PlayManagerProvider::instance()->playManager()),
		msecs2go(0),
		isRunning(false)
	{
		db = DB::Connector::instance()->settingsConnector();

		timer->setInterval(100);
		timerCountdown->setInterval(50);
	}

	~Private()
	{
		timer->stop();
		timer->deleteLater();
		timerCountdown->stop();
		timerCountdown->deleteLater();
	}
};

Shutdown::Shutdown(QObject* parent) :
	QObject(parent)
{
	m = Pimpl::make<Private>(this);

	connect(m->timer, &QTimer::timeout, this, &Shutdown::timeout);
	connect(m->timerCountdown, &QTimer::timeout, this, &Shutdown::countdownTimeout);
	connect(m->playManager, &PlayManager::sigPlaylistFinished, this, &Shutdown::playlistFinished);
}

Shutdown::~Shutdown() = default;

void Shutdown::shutdownAfterSessionEnd()
{
	m->isRunning = true;

	NotificationHandler::instance()->notify(
		Lang::get(Lang::Shutdown),
		tr("Computer will shutdown after playlist has finished"),
		m->logoPath
	);
}

bool Shutdown::is_running() const
{
	return m->isRunning;
}

void Shutdown::shutdown(MilliSeconds ms)
{
	if(ms == 0)
	{
		timeout();
		return;
	}

	m->isRunning = true;
	m->msecs2go = ms;
	m->timer->start(static_cast<int>(ms));
	m->timerCountdown->start(1000);

	emit sigStarted(ms);

	const auto minutes = static_cast<int>(ms / 60000);

	NotificationHandler::instance()->notify(
		Lang::get(Lang::Shutdown),
		tr("Computer will shutdown in %n minute(s)", "", minutes),
		m->logoPath
	);
}

void Shutdown::stop()
{
	spLog(Log::Info, this) << "Shutdown cancelled";

	m->isRunning = false;
	m->timer->stop();
	m->timerCountdown->stop();
	m->msecs2go = 0;

	emit sigStopped();
}

void Shutdown::countdownTimeout()
{
	m->msecs2go = std::max<MilliSeconds>(m->msecs2go - 1000, 0);
	m->timerCountdown->start(1000);

	emit sigTimeToGoChanged(m->msecs2go);
	spLog(Log::Debug, this) << "Time to go: " << m->msecs2go;

	if(m->msecs2go % 60000 == 0)
	{
		const auto minutes = static_cast<int>(m->msecs2go / 60000);
		NotificationHandler::instance()->notify(
			Lang::get(Lang::Shutdown),
			tr("Computer will shutdown in %n minute(s)", "", minutes),
			m->logoPath
		);
	}
}

void Shutdown::timeout()
{
	m->isRunning = false;
	m->db->storeSettings();

#ifdef Q_OS_WIN
	//ExitWindowsEx(

#else

	QDBusMessage response;
	QDBusInterface freeDesktopLogin(
		"org.freedesktop.login1",
		"/org/freedesktop/login1",
		"org.freedesktop.login1.Manager",
		QDBusConnection::systemBus()
	);

	QDBusInterface freeDesktopConsoleKit(
		"org.freedesktop.ConsoleKit",
		"/org/freedesktop/ConsoleKit/Manager",
		"org.freedesktop.ConsoleKit.Manager",
		QDBusConnection::systemBus()
	);

	QDBusInterface gnomeSessionManager(
		"org.gnome.SessionManager",
		"/org/gnome/SessionManager",
		"org.gnome.SessionManager",
		QDBusConnection::sessionBus()
	);

	QDBusInterface mateSessionManager(
		"org.mate.SessionManager",
		"/org/mate/SessionManager",
		"org.mate.SessionManager",
		QDBusConnection::sessionBus()
	);

	QDBusInterface kdeSessionManager(
		"org.kde.ksmserver",
		"/KSMServer",
		"org.kde.KSMServerInterface",
		QDBusConnection::sessionBus()
	);

	if(QProcess::startDetached("systemctl", QStringList {"poweroff"}))
	{
		return;
	}

	const auto gnomePowerCmd1 = QProcess::startDetached("gnome-power-cmd.sh",
	                                                    QStringList {"shutdown"});
	const auto gnomePowerCmd2 = QProcess::startDetached("gnome-power-cmd",
	                                                    QStringList {"shutdown"});

	if(gnomePowerCmd1 || gnomePowerCmd2)
	{
		return;
	}

	response = freeDesktopLogin.call("PowerOff", true);
	if(response.type() != QDBusMessage::ErrorMessage)
	{
		return;
	}

	response = gnomeSessionManager.call("RequestShutdown");
	if(response.type() != QDBusMessage::ErrorMessage)
	{
		return;
	}

	response = gnomeSessionManager.call("Shutdown");
	if(response.type() != QDBusMessage::ErrorMessage)
	{
		return;
	}

	response = kdeSessionManager.call("logout", 0, 2, 2);
	if(response.type() != QDBusMessage::ErrorMessage)
	{
		return;
	}

	response = kdeSessionManager.call("Shutdown");
	if(response.type() != QDBusMessage::ErrorMessage)
	{
		return;
	}

	response = mateSessionManager.call("RequestShutdown");
	if(response.type() != QDBusMessage::ErrorMessage)
	{
		return;
	}

	response = mateSessionManager.call("Shutdown");
	if(response.type() != QDBusMessage::ErrorMessage)
	{
		return;
	}

	response = freeDesktopConsoleKit.call("Stop");
	if(response.type() != QDBusMessage::ErrorMessage)
	{
		return;
	}

	if(QProcess::startDetached("sudo shutdown", QStringList {"-P", "now"}))
	{
		return;
	}

	if(QProcess::startDetached("sudo shutdown", QStringList {"-h", "-P", "now"}))
	{
		return;
	}

	spLog(Log::Warning, this) << "Sorry, power off is not possible";

#endif
}

void Shutdown::playlistFinished()
{
	if(m->isRunning)
	{
		timeout();
	}
}

#endif
