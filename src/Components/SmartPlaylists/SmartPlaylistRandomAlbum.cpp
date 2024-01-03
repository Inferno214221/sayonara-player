/* SmartPlaylistRandomAlbum.cpp */
/*
 * Copyright (C) 2011-2024 Michael Lugmair
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

#include "SmartPlaylistRandomAlbum.h"
#include "Database/Connector.h"
#include "Database/LibraryDatabase.h"
#include "Utils/Algorithm.h"
#include "Utils/Language/Language.h"
#include "Utils/MetaData/Album.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/RandomGenerator.h"
#include "Utils/Set.h"
#include "Utils/Utils.h"
#include "Utils/MetaData/MetaData.h"

#include <QObject>

namespace
{
	IdList getRandomAlbums(int count, DB::LibraryDatabase* libraryDatabase)
	{
		auto albums = AlbumList {};
		libraryDatabase->getAllAlbums(albums, false);
		Util::Algorithm::shuffle(albums);

		if(count == 0)
		{
			count = albums.count();
			if(count == 0)
			{
				return {};
			}
		}

		auto albumIds = IdList {};
		for(const auto& album: albums)
		{
			if(album.songcount() > 0)
			{
				albumIds << album.id();
			}

			if(albumIds.count() == count)
			{
				break;
			}
		}

		return albumIds;
	}

	MetaDataList getTracksByAlbumIds(const IdList& albumIds, DB::LibraryDatabase* libraryDatabase)
	{
		if(albumIds.isEmpty())
		{
			return {};
		}

		auto tracks = MetaDataList {};

		libraryDatabase->getAllTracksByAlbum(albumIds, tracks);
		Util::Algorithm::sort(tracks, [&](const auto& track1, const auto& track2) {
			const auto albumIndex1 = albumIds.indexOf(track1.albumId());
			const auto albumIndex2 = albumIds.indexOf(track2.albumId());
			if(albumIndex1 != albumIndex2)
			{
				return albumIndex1 < albumIndex2;
			}

			if(track1.trackNumber() != track2.trackNumber())
			{
				return track1.trackNumber() < track2.trackNumber();
			}

			return track1.filepath() < track2.filepath();
		});

		return tracks;
	}
}

SmartPlaylistRandomAlbum::SmartPlaylistRandomAlbum(const int id, const int count, const bool isRandomized,
                                                   const LibraryId libraryId) :
	SmartPlaylist(id, {count}, isRandomized, libraryId) {}

SmartPlaylistRandomAlbum::~SmartPlaylistRandomAlbum() = default;

int SmartPlaylistRandomAlbum::minimumValue() const { return 0; }

int SmartPlaylistRandomAlbum::maximumValue() const
{
	auto* dbConnector = DB::Connector::instance();
	auto* libraryDatabase = dbConnector->libraryDatabase(-1, 0);
	auto albums = AlbumList {};
	libraryDatabase->getAllAlbums(albums, false);

	return std::max(1, albums.count());
}

MetaDataList SmartPlaylistRandomAlbum::filterTracks(MetaDataList /*tracks*/)
{
	auto* dbConnector = DB::Connector::instance();
	auto* libraryDatabase = dbConnector->libraryDatabase(libraryId(), 0);

	const auto albumIds = getRandomAlbums(value(0), libraryDatabase);
	return getTracksByAlbumIds(albumIds, libraryDatabase);
}

QString SmartPlaylistRandomAlbum::classType() const { return SmartPlaylistRandomAlbum::ClassType; }

QString SmartPlaylistRandomAlbum::displayClassType() const { return QObject::tr("Random albums"); }

QString SmartPlaylistRandomAlbum::name() const
{
	return (value(0) > 0)
	       ? QObject::tr("%n random album(s)", "", value(0))
	       : QObject::tr("All albums randomized");
}

SmartPlaylists::Type SmartPlaylistRandomAlbum::type() const { return SmartPlaylists::Type::RandomAlbums; }

bool SmartPlaylistRandomAlbum::canFetchTracks() const { return true; }

QString SmartPlaylistRandomAlbum::text(int /*index*/) const
{
	return QObject::tr("Number of albums") + " " +
	       QString("(0=%1)").arg(Lang::get(Lang::All));
}

