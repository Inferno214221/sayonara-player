/* DBusMediaKeysInterfaceGnome.cpp */

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

#include "DBusMediaKeysInterfaceGnome.h"
#include "DBus/gnome_settings_daemon.h"
#include "Utils/Logger/Logger.h"

namespace
{
	constexpr const auto* service = "org.gnome.SettingsDaemon";
	constexpr const auto* objectPath = "/org/gnome/SettingsDaemon/MediaKeys";
};

namespace Dbus
{
	struct MediaKeysInterfaceGnome::Private
	{
		OrgGnomeSettingsDaemonMediaKeysInterface mediaKeyInterface;

		explicit Private(MediaKeysInterfaceGnome* parent) :
			mediaKeyInterface {service, objectPath, QDBusConnection::sessionBus(), parent} {}
	};

	MediaKeysInterfaceGnome::MediaKeysInterfaceGnome(PlayManager* playManager, QObject* parent) :
		MediaKeysInterface(playManager, parent)
	{
		m = Pimpl::make<Private>(this);

		init();
	}

	MediaKeysInterfaceGnome::~MediaKeysInterfaceGnome() = default;

	QString MediaKeysInterfaceGnome::serviceName() const
	{
		return QString("org.gnome.SettingsDaemon");
	}

	QDBusPendingReply<> MediaKeysInterfaceGnome::grabMediaKeyReply()
	{
		return m->mediaKeyInterface.GrabMediaPlayerKeys("sayonara", 0);
	}

	void MediaKeysInterfaceGnome::connectMediaKeys()
	{
		connect(&m->mediaKeyInterface,
		        &OrgGnomeSettingsDaemonMediaKeysInterface::MediaPlayerKeyPressed,
		        this,
		        &MediaKeysInterfaceGnome::mediaKeyPressed
		);
	}
}
