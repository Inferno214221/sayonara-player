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
	DB::Podcasts* db = DB::Connector::instance()->podcastConnector();
	QList<Podcast> podcasts;
	bool b = db->getAllPodcasts(podcasts);

	QList<StationPtr> ret;
	Util::Algorithm::transform(podcasts, stations, [](const Podcast& p)
	{
		return std::make_shared<Podcast>(p);
	});

	return b;
}

bool PodcastHandler::addNewStream(StationPtr station)
{
	DB::Podcasts* db = DB::Connector::instance()->podcastConnector();

	Podcast* p = dynamic_cast<Podcast*>(station.get());
	if(p) {
		return db->addPodcast(*p);
	}

	return false;
}

bool PodcastHandler::deleteStream(const QString& station_name)
{
	DB::Podcasts* db = DB::Connector::instance()->podcastConnector();
	return db->deletePodcast(station_name);
}

bool PodcastHandler::update(const QString& station_name, StationPtr station)
{
	DB::Podcasts* db = DB::Connector::instance()->podcastConnector();
	Podcast* podcast = dynamic_cast<Podcast*>(station.get());
	if(!podcast) {
		return false;
	}

	return db->updatePodcast(station_name, *podcast);
}

StationPtr PodcastHandler::createStreamInstance(const QString& name, const QString& url) const
{
	return std::make_shared<Podcast>(name, url);
}

StationPtr PodcastHandler::station(const QString& name)
{
	DB::Podcasts* db = DB::Connector::instance()->podcastConnector();

	Podcast podcast = db->getPodcast(name);
	if(podcast.name().isEmpty()){
		return nullptr;
	}

	return std::make_shared<Podcast>(podcast);
}

void PodcastHandler::createPlaylist(StationPtr station, MetaDataList& tracks)
{
	auto* podcast = dynamic_cast<Podcast*>(station.get());
	if(podcast && podcast->reversed())
	{
		std::reverse(tracks.begin(), tracks.end());
	}

	AbstractStationHandler::createPlaylist(station, tracks);
}
