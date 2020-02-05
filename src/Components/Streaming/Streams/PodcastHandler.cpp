/* StreamHandlerPodcasts.cpp */

/* Copyright (C) 2011-2020  Lucio Carreras
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
#include "Utils/Algorithm.h"
#include "Utils/Streams/Station.h"

PodcastHandler::PodcastHandler(QObject* parent) :
	AbstractStationHandler(parent) {}

PodcastHandler::~PodcastHandler() = default;

bool PodcastHandler::get_all_streams(QList<StationPtr>& stations)
{
	DB::Podcasts* db = DB::Connector::instance()->podcast_connector();
	QList<Podcast> podcasts;
	bool b = db->getAllPodcasts(podcasts);

	QList<StationPtr> ret;
	Util::Algorithm::transform(podcasts, stations, [](const Podcast& p)
	{
		return std::make_shared<Podcast>(p);
	});

	return b;
}

bool PodcastHandler::add_stream(StationPtr station)
{
	DB::Podcasts* db = DB::Connector::instance()->podcast_connector();

	Podcast* p = dynamic_cast<Podcast*>(station.get());
	if(p) {
		return db->addPodcast(*p);
	}

	return false;
}

bool PodcastHandler::delete_stream(const QString& station_name)
{
	DB::Podcasts* db = DB::Connector::instance()->podcast_connector();
	return db->deletePodcast(station_name);
}

bool PodcastHandler::update_url(const QString& station_name, const QString& url)
{
	DB::Podcasts* db = DB::Connector::instance()->podcast_connector();
	return db->updatePodcastUrl(station_name, url);
}

bool PodcastHandler::rename(const QString& old_name, const QString& new_name)
{
	DB::Podcasts* db = DB::Connector::instance()->podcast_connector();
	return db->renamePodcast(old_name, new_name);
}

StationPtr PodcastHandler::create_stream(const QString& name, const QString& url) const
{
	return std::make_shared<Podcast>(name, url);
}

StationPtr PodcastHandler::station(const QString& name)
{
	DB::Podcasts* db = DB::Connector::instance()->podcast_connector();

	Podcast podcast = db->getPodcast(name);
	if(podcast.name().isEmpty()){
		return nullptr;
	}

	return std::make_shared<Podcast>(podcast);
}
