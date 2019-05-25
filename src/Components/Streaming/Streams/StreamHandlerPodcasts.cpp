/* StreamHandlerPodcasts.cpp */

/* Copyright (C) 2011-2019  Lucio Carreras
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

#include "StreamHandlerPodcasts.h"
#include "Database/Connector.h"
#include "Database/Podcasts.h"

StreamHandlerPodcasts::StreamHandlerPodcasts(QObject* parent) :
	AbstractStreamHandler(parent) {}

StreamHandlerPodcasts::~StreamHandlerPodcasts() {}

bool StreamHandlerPodcasts::get_all_streams(StreamMap& streams)
{
	DB::Podcasts* db = DB::Connector::instance()->podcast_connector();
	return db->getAllPodcasts(streams);
}

bool StreamHandlerPodcasts::add_stream(const QString& station_name, const QString& url)
{
	DB::Podcasts* db = DB::Connector::instance()->podcast_connector();
	return db->addPodcast(station_name, url);
}

bool StreamHandlerPodcasts::delete_stream(const QString& station_name)
{
	DB::Podcasts* db = DB::Connector::instance()->podcast_connector();
	return db->deletePodcast(station_name);
}

bool StreamHandlerPodcasts::update_url(const QString& station_name, const QString& url)
{
	DB::Podcasts* db = DB::Connector::instance()->podcast_connector();
	return db->updatePodcastUrl(station_name, url);
}


bool StreamHandlerPodcasts::rename(const QString& old_name, const QString& new_name)
{
	DB::Podcasts* db = DB::Connector::instance()->podcast_connector();
	return db->renamePodcast(old_name, new_name);
}

