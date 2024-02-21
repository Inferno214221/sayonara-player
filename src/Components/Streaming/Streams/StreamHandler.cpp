/* StreamHandlerStreams.cpp */

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

#include "StreamHandler.h"
#include "Database/Connector.h"
#include "Database/Streams.h"

#include "Utils/Algorithm.h"
#include "Utils/MetaData/MetaData.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Streams/Station.h"
#include "Utils/Parser/StreamParser.h"
#include "Utils/Settings/Settings.h"

StreamHandler::StreamHandler(Playlist::Creator* playlistCreator, const StationParserFactoryPtr& stationParserFactory,
                             QObject* parent) :
	AbstractStationHandler(playlistCreator, stationParserFactory, parent) {}

StreamHandler::~StreamHandler() = default;

bool StreamHandler::getAllStreams(QList<StationPtr>& stations)
{
	auto* db = DB::Connector::instance()->streamConnector();

	auto streams = QList<Stream> {};
	const auto b = db->getAllStreams(streams);

	Util::Algorithm::transform(streams, stations, [](auto& stream) {
		return std::make_shared<Stream>(std::move(stream));
	});

	return b;
}

bool StreamHandler::saveStream(const StationPtr& station)
{
	auto* db = DB::Connector::instance()->streamConnector();

	const auto stream = std::dynamic_pointer_cast<Stream>(station);
	return stream
	       ? db->addStream(*stream)
	       : false;
}

bool StreamHandler::deleteStream(const QString& name)
{
	auto* db = DB::Connector::instance()->streamConnector();

	return db->deleteStream(name);
}

bool StreamHandler::updateStream(const QString& name, const StationPtr& station)
{
	auto* db = DB::Connector::instance()->streamConnector();

	const auto stream = std::dynamic_pointer_cast<Stream>(station);
	return stream
	       ? db->updateStream(name, *stream)
	       : false;
}

StationPtr StreamHandler::createStreamInstance(const QString& name, const QString& url)
{
	return std::make_shared<Stream>(name, url, GetSetting(Set::Stream_UpdateMetadata));
}

StationPtr StreamHandler::fetchStation(const QString& name)
{
	auto* db = DB::Connector::instance()->streamConnector();
	const auto stream = db->getStream(name);

	return !stream.name().isEmpty()
	       ? std::make_shared<Stream>(stream)
	       : nullptr;
}

MetaDataList StreamHandler::preprocessPlaylist(const StationPtr& station, MetaDataList tracks)
{
	const auto stream = std::dynamic_pointer_cast<Stream>(station);
	for(auto& track: tracks)
	{
		track.setUpdateable(stream->isUpdatable());
	}

	return tracks;
}
