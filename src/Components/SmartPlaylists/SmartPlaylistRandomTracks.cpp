/* SmartPlaylistRandomTracks.cpp */

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

#include "SmartPlaylistRandomTracks.h"
#include "Database/Connector.h"
#include "Database/LibraryDatabase.h"

#include "Utils/Language/Language.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/RandomGenerator.h"

#include <QObject>

SmartPlaylistRandomTracks::SmartPlaylistRandomTracks(const int id, const int count, const bool isRandomized,
                                            		 const LibraryId libraryId) :
	SmartPlaylist(id, {count}, isRandomized, libraryId) {}

SmartPlaylistRandomTracks::~SmartPlaylistRandomTracks() = default;

int SmartPlaylistRandomTracks::minimumValue() const { return 0; }

int SmartPlaylistRandomTracks::maximumValue() const {
	// return 1000;
	auto* dbConnector = DB::Connector::instance();
	auto* libraryDatabase = dbConnector->libraryDatabase(-1, 0);
	auto tracks = MetaDataList {};
	libraryDatabase->getAllTracks(tracks);

	return std::max(1, tracks.count());
} // NOLINT(readability-magic-numbers)

QString SmartPlaylistRandomTracks::classType() const { return SmartPlaylistRandomTracks::ClassType; }

QString SmartPlaylistRandomTracks::displayClassType() const { return QObject::tr("Tracks"); }

QString SmartPlaylistRandomTracks::name() const
{
	return (value(0) > 0)
	       ? QObject::tr("%n track(s)", "", value(0))
	       : QObject::tr("All tracks");
}

SmartPlaylists::Type SmartPlaylistRandomTracks::type() const { return SmartPlaylists::Type::RandomTracks; }

MetaDataList SmartPlaylistRandomTracks::filterTracks(MetaDataList tracks)
{
	if (isRandomized()) { Util::Algorithm::shuffle(tracks); }

	if(value(0) == 0) {
		return tracks;
	}
	else if(value(0) < tracks.count())
	{
		tracks.erase(tracks.begin() + value(0), tracks.end());
	}

	return tracks;
}

QString SmartPlaylistRandomTracks::text(int /*index*/) const
{
	return QObject::tr("Number of tracks") + " " +
				"(0=All)";
	    //    QString("(0=%1)").arg(Lang::get(Lang::All));
}

// bool SmartPlaylistRandomTracks::isRandomizable() const { return false; }
