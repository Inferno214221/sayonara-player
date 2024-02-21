/* AbstractStreamHandler.cpp */

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

#include "AbstractStationHandler.h"

#include "Components/Playlist/Playlist.h"
#include "Playlist/PlaylistInterface.h"
#include "Utils/Logger/Logger.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Parser/StreamParser.h"
#include "Utils/Settings/Settings.h"

#include <QMap>

struct AbstractStationHandler::Private
{
	PlaylistCreator* playlistCreator;
	StationParserFactoryPtr stationParserFactory;
	StreamParser* streamParser = nullptr;
	StationPtr parsedStation;
	QMap<QString, StationPtr> temporaryStations;

	Private(PlaylistCreator* playlistCreator, StationParserFactoryPtr stationParserFactory) :
		playlistCreator(playlistCreator),
		stationParserFactory(std::move(stationParserFactory)) {}
};

AbstractStationHandler::AbstractStationHandler(PlaylistCreator* playlistCreator,
                                               const StationParserFactoryPtr& stationParserFactory, QObject* parent) :
	QObject(parent),
	m {Pimpl::make<Private>(playlistCreator, stationParserFactory)} {}

AbstractStationHandler::~AbstractStationHandler() = default;

void AbstractStationHandler::createPlaylist(const StationPtr& station, const MetaDataList& tracks)
{
	if(!tracks.isEmpty())
	{
		const auto playlistName = GetSetting(Set::Stream_NewTab)
		                          ? station->name()
		                          : QString {};

		const auto index =
			m->playlistCreator->createPlaylist(preprocessPlaylist(station, tracks),
			                                   playlistName,
			                                   true,
			                                   GetSetting(Set::Stream_LockedPlaylistByDefault));

		auto playlist = m->playlistCreator->playlist(index);
		playlist->changeTrack(0);
		playlist->setLocked(GetSetting(Set::Stream_LockedPlaylistByDefault));
	}
}

bool AbstractStationHandler::parseStation(const StationPtr& station)
{
	if(m->streamParser)
	{
		return false;
	}

	m->parsedStation = station;
	m->streamParser = m->stationParserFactory->createParser();

	connect(m->streamParser, &StreamParser::sigFinished, this, &AbstractStationHandler::parserFinished);
	connect(m->streamParser, &StreamParser::sigUrlCountExceeded, this, &AbstractStationHandler::sigUrlCountExceeded);
	connect(m->streamParser, &StreamParser::sigStopped, this, &AbstractStationHandler::parserStopped);

	m->streamParser->parse(m->parsedStation->name(), m->parsedStation->url());

	return true;
}

StationPtr AbstractStationHandler::station(const QString& name)
{
	return m->temporaryStations.contains(name)
	       ? m->temporaryStations[name]
	       : fetchStation(name);
}

bool AbstractStationHandler::addNewStream(const StationPtr& station)
{
	const auto success = saveStream(station);
	if(success)
	{
		m->temporaryStations.remove(station->name());
	}
	return success;
}

bool AbstractStationHandler::removeStream(const QString& name)
{
	if(m->temporaryStations.contains(name))
	{
		m->temporaryStations.remove(name);
		return true;
	}

	return deleteStream(name);
}

void AbstractStationHandler::parserFinished(const bool success)
{
	if(!success)
	{
		emit sigError();
		spLog(Log::Warning, this) << "Stream parser finished with error";
	}

	else
	{
		createPlaylist(m->parsedStation, m->streamParser->tracks());
		emit sigDataAvailable();
	}

	sender()->deleteLater(); // m->stream_parser may be nullptr here
	m->streamParser = nullptr;
}

void AbstractStationHandler::stop()
{
	if(m->streamParser && !m->streamParser->isStopped())
	{
		m->streamParser->stopParsing();
	}

	m->streamParser = nullptr;
	emit sigStopped();
}

void AbstractStationHandler::parserStopped()
{
	sender()->deleteLater(); // m->stream_parser may be nullptr here
	m->streamParser = nullptr;
	emit sigStopped();
}

void AbstractStationHandler::addTemporaryStation(const StationPtr& station)
{
	m->temporaryStations.insert(station->name(), station);
}

bool AbstractStationHandler::isTemporary(const QString& stationName) const
{
	return m->temporaryStations.contains(stationName);
}
