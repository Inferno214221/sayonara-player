/* DBusSessionManager.cpp */
/*
 * Copyright (C) 2011-2023 Michael Lugmair
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

// https://lira.no-ip.org:8443/doc/gnome-session/dbus/gnome-session.html#org.gnome.SessionManager

#include "DBusSessionManager.h"
#include "Components/PlayManager/PlayManager.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Settings/Settings.h"

namespace
{
	constexpr const auto ServiceName = "org.gnome.SessionManager";
	constexpr const auto ObjectPath = "/org/gnome/SessionManager";
	constexpr const auto InhibitIdleFlag = 8U;
}

namespace Dbus
{
	struct SessionManager::Private
	{
		using Interface = OrgGnomeSessionManagerInterface;

		PlayManager* playManager;
		std::shared_ptr<Interface> interface;
		uint32_t cookie {0U};

		explicit Private(PlayManager* playManager, QObject* parent) :
			playManager {playManager},
			interface {std::make_shared<Interface>(ServiceName, ObjectPath, QDBusConnection::sessionBus(), parent)} {}
	};

	SessionManager::SessionManager(PlayManager* playManager) :
		QObject {playManager},
		m {Pimpl::make<Private>(playManager, this)}
	{
		connect(playManager, &PlayManager::sigPlaystateChanged, this, &SessionManager::playstateChanged);
		playstateChanged(playManager->playstate());
	}

	SessionManager::~SessionManager() noexcept
	{
		uninhibit();
	}

	void SessionManager::inhibit()
	{
		if(GetSetting(Set::InhibitIdle) && (m->cookie == 0U))
		{
			auto reply = m->interface->Inhibit("Sayonara", 0U, "Playing music", InhibitIdleFlag);
			reply.waitForFinished();
			if(reply.isValid())
			{
				m->cookie = reply.value();
			}

			else
			{
				spLog(Log::Warning, this) << reply.error().name() << ": "
				                          << reply.error().message();
			}
		}
	}

	void SessionManager::uninhibit()
	{
		if(m->cookie > 0U)
		{
			m->interface->Uninhibit(m->cookie);
			m->cookie = 0U;
		}
	}

	void SessionManager::playstateChanged(const PlayState playState)
	{
		if(playState == PlayState::Stopped)
		{
			uninhibit();
		}
		else
		{
			inhibit();
		}
	}

	void SessionManager::inhibitSettingChanged()
	{
		if(!GetSetting(Set::InhibitIdle))
		{
			uninhibit();
		}

		else
		{
			playstateChanged(m->playManager->playstate());
		}
	}
}