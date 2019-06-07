/* DBusMPRIS.cpp */

/* Copyright (C) 2011-2019  Lucio Carreras
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

#include "DBusMPRIS.h"
#include "Components/DBus/org_mpris_media_player2_adaptor.h"

#include "Components/DBus/mediaplayer2adaptor.h"

#include "Utils/Logger/Logger.h"
#include "Utils/Settings/Settings.h"

#include <QMainWindow>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QStringList>
#include <QUrl>


struct DBusMPRIS::MediaPlayer2::Private
{
	QMainWindow*	player=nullptr;

	bool			initialized;

	Private(QMainWindow* player) :
		player(player),
		initialized(false)
	{}
};

DBusMPRIS::MediaPlayer2::MediaPlayer2(QMainWindow* player, QObject *parent) :
	DBusAdaptor("/org/mpris/MediaPlayer2", "org.mpris.MediaPlayer2.sayonara","org.mpris.MediaPlayer2", "org.freedesktop.DBus.Properties", parent)
{
	m = Pimpl::make<Private>(player);
}


DBusMPRIS::MediaPlayer2::~MediaPlayer2()
{
	QDBusConnection::sessionBus().unregisterObject(object_path());
	QDBusConnection::sessionBus().unregisterService(service_name());
}

void DBusMPRIS::MediaPlayer2::init()
{
	if(m->initialized){
		return;
	}

	new OrgMprisMediaPlayer2Adaptor(this);

	if (!QDBusConnection::sessionBus().registerService(service_name())) {
		sp_log(Log::Error, this)	<< "Failed to register "
							<< service_name()
							<< " on the session bus";

		m->initialized = true;

		return;
	}

	sp_log(Log::Info, this) << service_name() << " registered";

	QDBusConnection::sessionBus().registerObject(object_path(), this);
	create_message("DesktopEntry", QString("sayonara"));

	m->initialized = true;
}


bool DBusMPRIS::MediaPlayer2::CanQuit() const
{
	return true;
}

bool DBusMPRIS::MediaPlayer2::CanRaise()
{
	return true;
}

bool DBusMPRIS::MediaPlayer2::HasTrackList()
{
	return false;
}

QString DBusMPRIS::MediaPlayer2::Identity()
{
	return QString("Sayonara Player");
}

QString DBusMPRIS::MediaPlayer2::DesktopEntry()
{
	return QString("sayonara");
}

QStringList DBusMPRIS::MediaPlayer2::SupportedUriSchemes()
{
	QStringList uri_schemes;
	uri_schemes << "file"
				<< "http"
				<< "cdda"
				<< "smb"
				<< "sftp";

	return uri_schemes;
}

QStringList DBusMPRIS::MediaPlayer2::SupportedMimeTypes()
{
	QStringList mimetypes;
	mimetypes   << "audio/mpeg"
				<< "audio/ogg";

	return mimetypes;
}

bool DBusMPRIS::MediaPlayer2::CanSetFullscreen()
{
	return true;
}

bool DBusMPRIS::MediaPlayer2::Fullscreen()
{
	return GetSetting(Set::Player_Fullscreen);
}


void DBusMPRIS::MediaPlayer2::SetFullscreen(bool b)
{
	SetSetting(Set::Player_Fullscreen, b);
}


void DBusMPRIS::MediaPlayer2::Quit()
{
	m->player->close();
}

void DBusMPRIS::MediaPlayer2::Raise()
{
	sp_log(Log::Debug, this) << "Raise";

	m->player->show();
	m->player->raise();
	m->player->show();
	m->player->raise();
}
