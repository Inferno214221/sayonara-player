/* Shutdown.cpp */

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


/* Inspired by qshutdown
 * Original-Maintainer: Christian Metscher <hakaishi@web.de>
 * Homepage: https://launchpad.net/~hakaishi
 */


#include "Components/Shutdown/Shutdown.h"

#include "Components/Notification/NotificationHandler.h"
#include "Components/PlayManager/PlayManager.h"
#include "Database/Connector.h"
#include "Database/Settings.h"
#include "Utils/Language/Language.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Utils.h"

#include <QDBusInterface>
#include <QDBusConnection>
#include <QProcess>
#include <QTimer>

class ShutdownImpl :
	public Shutdown
{
	public:
		explicit ShutdownImpl(PlayManager* playManager, NotificationHandler* notificationHandler) :
			m_notificationHandler {notificationHandler},
			m_timer {new QTimer(this)},
			m_timerCountdown {new QTimer(this)}
		{
			m_timer->setInterval(100);
			m_timerCountdown->setInterval(50);

			connect(m_timer, &QTimer::timeout, this, &ShutdownImpl::timeout);
			connect(m_timerCountdown, &QTimer::timeout, this, &ShutdownImpl::countdownTimeout);
			connect(playManager, &PlayManager::sigPlaylistFinished, this, &ShutdownImpl::timeout);
		}

		~ShutdownImpl() noexcept override
		{
			m_timer->stop();
			m_timer->deleteLater();
			m_timerCountdown->stop();
			m_timerCountdown->deleteLater();
		}

		void shutdownAfterSessionEnd() override
		{
			m_isRunning = true;
			m_notificationHandler->notify(
				Lang::get(Lang::Shutdown),
				tr("Computer will shutdown after playlist has finished"),
				m_logoPath
			);
		}

		[[nodiscard]] bool isRunning() const override
		{
			return m_isRunning;
		}

		void shutdown(const MilliSeconds ms) override
		{
			if(ms == 0)
			{
				timeout();
				return;
			}

			m_isRunning = true;
			m_msecs2go = ms;
			m_timer->start(static_cast<int>(ms));
			m_timerCountdown->start(1000);

			emit sigStarted(ms);

			const auto minutes = static_cast<int>(ms / 60000);

			m_notificationHandler->notify(
				Lang::get(Lang::Shutdown),
				tr("Computer will shutdown in %n minute(s)", "", minutes),
				m_logoPath
			);
		}

		void stop() override
		{
			spLog(Log::Info, this) << "Shutdown cancelled";

			m_isRunning = false;
			m_timer->stop();
			m_timerCountdown->stop();
			m_msecs2go = 0;

			emit sigStopped();
		}

	private slots:

		void countdownTimeout()
		{
			m_msecs2go = std::max<MilliSeconds>(m_msecs2go - 1'000, 0);
			m_timerCountdown->start(1'000);

			emit sigTimeToGoChanged(m_msecs2go);
			spLog(Log::Debug, this) << "Time to go: " << m_msecs2go;

			if(m_msecs2go % 60'000 == 0)
			{
				const auto minutes = static_cast<int>(m_msecs2go / 60'000);
				m_notificationHandler->notify(
					Lang::get(Lang::Shutdown),
					tr("Computer will shutdown in %n minute(s)", "", minutes),
					m_logoPath
				);
			}
		}

		void timeout()
		{
			m_isRunning = false;
			DB::Connector::instance()->settingsConnector()->storeSettings();

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
		}

	private: // NOLINT(readability-redundant-access-specifiers)
		QString m_logoPath {":/Icons/logo.png"};
		NotificationHandler* m_notificationHandler;
		QTimer* m_timer;
		QTimer* m_timerCountdown;
		MilliSeconds m_msecs2go {0};
		bool m_isRunning {false};
};

Shutdown* Shutdown::create(PlayManager* playManager, NotificationHandler* notificationHandler)
{
	return new ShutdownImpl(playManager, notificationHandler);
}