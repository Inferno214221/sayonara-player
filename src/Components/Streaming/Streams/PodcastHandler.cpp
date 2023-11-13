/* StreamHandlerPodcasts.cpp */

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

#include "PodcastHandler.h"
#include "Database/Connector.h"
#include "Database/Podcasts.h"

#include "Interfaces/PlaylistInterface.h"

#include "Utils/Algorithm.h"
#include "Utils/Streams/Station.h"
#include "Utils/MetaData/MetaDataList.h"

PodcastHandler::PodcastHandler(PlaylistCreator* playlistCreator, QObject* parent) :
	AbstractStationHandler(playlistCreator, parent) {}

PodcastHandler::~PodcastHandler() = default;

bool PodcastHandler::getAllStreams(QList<StationPtr>& stations)
{
	auto* db = DB::Connector::instance()->podcastConnector();

	QList<Podcast> podcasts;
	auto podcastsFetched = db->getAllPodcasts(podcasts);

	QList<StationPtr> ret;
	Util::Algorithm::transform(podcasts, stations, [](const auto& p) {
		return std::make_shared<Podcast>(p);
	});

	return podcastsFetched;
}

bool PodcastHandler::addNewStream(const StationPtr& station)
{
	auto* db = DB::Connector::instance()->podcastConnector();
	auto podcast = std::dynamic_pointer_cast<Podcast>(station);
	return podcast
	       ? db->addPodcast(*podcast)
	       : false;
}

bool PodcastHandler::deleteStream(const QString& stationName)
{
	auto* db = DB::Connector::instance()->podcastConnector();
	return db->deletePodcast(stationName);
}

bool PodcastHandler::updateStream(const QString& stationName, const StationPtr& station)
{
	auto* db = DB::Connector::instance()->podcastConnector();
	auto podcast = std::dynamic_pointer_cast<Podcast>(station);
	return podcast
	       ? db->updatePodcast(stationName, *podcast)
	       : false;
}

StationPtr PodcastHandler::createStreamInstance(const QString& name, const QString& url) const
{
	return std::make_shared<Podcast>(name, url);
}

StationPtr PodcastHandler::station(const QString& name)
{
	auto* db = DB::Connector::instance()->podcastConnector();
	const auto podcast = db->getPodcast(name);
	return !podcast.name().isEmpty()
	       ? std::make_shared<Podcast>(podcast)
	       : nullptr;
}

MetaDataList PodcastHandler::preprocessPlaylist(const StationPtr& station, MetaDataList tracks)
{
	const auto podcast = std::dynamic_pointer_cast<Podcast>(station);
	if(podcast && podcast->reversed())
	{
		std::reverse(tracks.begin(), tracks.end());
	}

	return tracks;
}
