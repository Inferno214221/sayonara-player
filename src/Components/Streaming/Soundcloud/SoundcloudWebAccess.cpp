/* SoundcloudWebAccess.cpp */

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

#include "SoundcloudWebAccess.h"
#include "SoundcloudGlobal.h"
#include "Utils/Logger/Logger.h"

#include <QString>

#define sc_main		QString("https://api.soundcloud.com")
#define sc_users	QString("https://api.soundcloud.com/users")

QString	SC::WebAccess::createLinkGetArtist(const QString& name)
{
	if(name.isEmpty()) {
		return QString("");
	}


	QString client_id_string(CLIENT_ID_STR);
	//QString ret = QString("%1?%2&q=%3").arg(sc_users, client_id_string, name);
	QString ret = sc_users + "?" + client_id_string + "&q=" + name;

	spLog(Log::Debug, "SCWA") << "Concat: " << sc_users << ", " << client_id_string << ", " << name;
	spLog(Log::Debug, "SCMA") << "Get Artist info from " << ret;

	return ret;
}

QString	SC::WebAccess::createLinkGetArtist(int artistId)
{
	QString ret;

	if(artistId <= 0){
		return ret;
	}

	ret += sc_users + "/" + QString::number(artistId) + "?" + CLIENT_ID_STR;

	spLog(Log::Debug, "SCMA") << "Get Artist info from " << ret;

	return ret;
}

QString	SC::WebAccess::createLinkGetPlaylists(int artistId)
{
	QString ret;

	ret = sc_users + "/" + QString::number(artistId) + "/playlists?" +
			CLIENT_ID_STR;

	spLog(Log::Debug, "Soundcloud") << "Get artist playlists from " << ret;

	return ret;
}

QString	SC::WebAccess::createLinkGetTracks(int artistId)
{
	QString ret;

	ret = sc_users + "/" + QString::number(artistId) + "/tracks?" +	CLIENT_ID_STR;

	spLog(Log::Debug, "Soundcloud") << "Get Artist tracks from " << ret;

	return ret;
}

