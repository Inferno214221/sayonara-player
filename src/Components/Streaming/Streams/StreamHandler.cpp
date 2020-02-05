/* StreamHandlerStreams.cpp */

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

#include "StreamHandler.h"
#include "Database/Connector.h"
#include "Database/Streams.h"

#include "Utils/Algorithm.h"
#include "Utils/Streams/Stream.h"

StreamHandler::StreamHandler(QObject* parent) :
	AbstractStationHandler(parent) {}

StreamHandler::~StreamHandler() = default;

bool StreamHandler::get_all_streams(QList<StationPtr>& stations)
{
	DB::Streams* db = DB::Connector::instance()->stream_connector();

	QList<Stream> streams;
	bool b = db->getAllStreams(streams);

	QList<StationPtr> ret;
	Util::Algorithm::transform(streams, stations, [this](const Stream& p)
	{
		return this->create_stream(p.name(), p.url());
	});

	return b;
}

bool StreamHandler::add_stream(StationPtr station)
{
	DB::Streams* db = DB::Connector::instance()->stream_connector();

	auto* s = dynamic_cast<Stream*>(station.get());
	if(s){
		return db->addStream(*s);
	}

	return false;
}

bool StreamHandler::delete_stream(const QString& station_name)
{
	DB::Streams* db = DB::Connector::instance()->stream_connector();
	return db->deleteStream(station_name);
}

bool StreamHandler::update_url(const QString& station_name, const QString& url)
{
	DB::Streams* db = DB::Connector::instance()->stream_connector();
	return db->updateStreamUrl(station_name, url);
}

bool StreamHandler::rename(const QString& old_name, const QString& new_name)
{
	DB::Streams* db = DB::Connector::instance()->stream_connector();
	return db->renameStream(old_name, new_name);
}

StationPtr StreamHandler::create_stream(const QString& name, const QString& url) const
{
	return std::make_shared<Stream>(name, url);
}

StationPtr StreamHandler::station(const QString& name)
{
	DB::Streams* db = DB::Connector::instance()->stream_connector();

	Stream stream = db->getStream(name);
	if(stream.name().isEmpty()){
		return nullptr;
	}

	return std::make_shared<Stream>(stream);
}
