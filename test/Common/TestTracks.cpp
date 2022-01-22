/* TestDatabase.cpp */
/*
 * Copyright (C) 2011-2021 Michael Lugmair
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

#include "test/Common/TestTracks.h"

#include "Utils/MetaData/MetaDataList.h"
#include "Utils/MetaData/MetaData.h"
#include "Utils/Utils.h"

#include <QDateTime>

namespace
{
	MetaDataList createMilkyWayTracks()
	{
		MetaDataList result;

		const auto artist = "";
		{
			const auto album = "Solar System";
			result << Test::createTrack(1, "Mercury", artist, album);
			result << Test::createTrack(2, "Venus", artist, album);
			result << Test::createTrack(3, "Earth", artist, album);
			result << Test::createTrack(4, "Mars", artist, album);
			result << Test::createTrack(5, "Jupiter", artist, album);
			result << Test::createTrack(6, "Saturn", artist, album);
			result << Test::createTrack(7, "Neptune", artist, album);
			result << Test::createTrack(8, "Uranus", artist, album);
		}

		return result;
	}

	MetaDataList createOneYearTracks()
	{
		MetaDataList result;

		const auto artist = "One Year";
		{
			const auto album = "Spring";
			result << Test::createTrack(1, "March", artist, album);
			result << Test::createTrack(2, "April", artist, album);
			result << Test::createTrack(3, "May", artist, album);
		}

		{
			const auto album = "Summer";
			result << Test::createTrack(1, "June", artist, album);
			result << Test::createTrack(2, "July", artist, album);
			result << Test::createTrack(3, "August", artist, album);
		}

		{
			const auto album = "Fall";
			result << Test::createTrack(1, "September", artist, album);
			result << Test::createTrack(2, "October", artist, album);
			result << Test::createTrack(3, "November", artist, album);
		}

		{
			const auto album = "Winter";
			result << Test::createTrack(1, "December", artist, album);
			result << Test::createTrack(2, "January", artist, album);
			result << Test::createTrack(3, "Feburaty", artist, album);
		}

		return result;
	}

	MetaDataList createEarthTracks()
	{
		MetaDataList result;

		const auto commonArtist = "Earth";
		{
			const auto album = "Asia Rivers";
			const auto albumArtist = "Asia";
			const auto artist = QString("%1 feat. %2").arg(commonArtist).arg(albumArtist);
			result << Test::createTrack(1, "Yang Tse", artist, album, albumArtist);
			result << Test::createTrack(2, "Mekong", artist, album, albumArtist);
			result << Test::createTrack(3, "Shinano", artist, album, albumArtist);
		}

		{
			const auto album = "Europe Mountains";
			const auto albumArtist = "Europe";
			const auto artist = QString("%1 feat. %2").arg(commonArtist).arg(albumArtist);
			result << Test::createTrack(1, "Zugspitze", artist, album, albumArtist);
			result << Test::createTrack(2, "Mont Blanc", artist, album, albumArtist);
			result << Test::createTrack(3, "Seine", artist, album, albumArtist);
			result << Test::createTrack(4, "Matterhorn", artist, album, albumArtist);
		}

		{
			const auto album = "African Countries";
			const auto albumArtist = "Africa";
			const auto artist = QString("%1 feat. %2").arg(commonArtist).arg(albumArtist);
			result << Test::createTrack(1, "Algeria", artist, album, albumArtist);
			result << Test::createTrack(2, "Mosambique", artist, album, albumArtist);
			result << Test::createTrack(3, "Ivory Coast", artist, album, albumArtist);
			result << Test::createTrack(4, "Kenia", artist, album, albumArtist);
			result << Test::createTrack(5, "Nigeria", artist, album, albumArtist);
		}

		return result;
	}
}

namespace Test
{
	MetaDataList createTracks()
	{
		return createMilkyWayTracks() << createEarthTracks() << createOneYearTracks();
	}

	MetaData
	createTrack(const uint16_t trackNumber, const QString& title, const QString& artist, const QString& album,
	            const QString& albumArtist)
	{
		const auto filepath = QString("/path/to/%1/%2/%3.mp3")
			.arg(artist)
			.arg(album)
			.arg(title);

		MetaData track;
		track.setTrackNumber(trackNumber);
		track.setTitle(title);
		track.setArtist(artist);
		track.setAlbumArtist(albumArtist);
		track.setAlbum(album);
		track.setFilepath(filepath);
		track.setGenres(QStringList() << artist << album);

		const auto currentDate = Util::dateToInt(QDateTime::currentDateTime());

		track.setBitrate(320);
		track.setCreatedDate(currentDate);
		track.setModifiedDate(currentDate);
		track.setDiscnumber(1);
		track.setDurationMs(10'000);
		track.setComment("Some comment");
		track.setFilesize(100'000);
		track.setRating(Rating::Five);
		track.setYear(2000);

		return track;
	}
}