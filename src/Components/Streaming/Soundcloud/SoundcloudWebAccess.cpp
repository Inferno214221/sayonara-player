/* SoundcloudWebAccess.cpp */

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

#include "SoundcloudWebAccess.h"

#include <QString>

namespace SC
{
	namespace
	{
		constexpr const auto SoundcloudUrl = "https://api.soundcloud.com";
	}

	QString createLinkGetArtist(const QString& name)
	{
		return (name.isEmpty())
		       ? QString()
		       : QString("%1/users?q=%2").arg(SoundcloudUrl, name);
	}

	QString createLinkGetArtist(int artistId)
	{
		return (artistId <= 0)
		       ? QString()
		       : QString("%1/users/%2")
			       .arg(SoundcloudUrl)
			       .arg(artistId);
	}

	QString createLinkGetPlaylists(int artistId)
	{
		return (artistId <= 0)
		       ? QString()
		       : QString("%1/users/%2/playlists")
			       .arg(SoundcloudUrl)
			       .arg(artistId);
	}

	QString createLinkGetTracks(int artistId)
	{
		return (artistId <= 0)
		       ? QString()
		       : QString("%1/users/%2/tracks")
			       .arg(SoundcloudUrl)
			       .arg(artistId);
	}

	QString createLinkObtainToken()
	{
		return QString("https://sayonara-player.com/soundcloud");
	}
}