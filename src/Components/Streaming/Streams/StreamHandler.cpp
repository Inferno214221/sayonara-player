/* StreamHandlerStreams.cpp */

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

#include "StreamHandler.h"
#include "Database/Connector.h"
#include "Database/Streams.h"

#include "Utils/Algorithm.h"
#include "Utils/Streams/Stream.h"

StreamHandler::StreamHandler(PlaylistCreator* playlistCreator, QObject* parent) :
	AbstractStationHandler(playlistCreator, parent) {}

StreamHandler::~StreamHandler() = default;

bool StreamHandler::getAllStreams(QList<StationPtr>& stations)
{
	DB::Streams* db = DB::Connector::instance()->streamConnector();

	QList<Stream> streams;
	bool b = db->getAllStreams(streams);

	QList<StationPtr> ret;
	Util::Algorithm::transform(streams, stations, [this](const Stream& p)
	{
		return this->createStreamInstance(p.name(), p.url());
	});

	return b;
}

bool StreamHandler::addNewStream(StationPtr station)
{
	DB::Streams* db = DB::Connector::instance()->streamConnector();

	auto* stream = dynamic_cast<Stream*>(station.get());
	if(!stream) {
		return false;
	}

	return db->addStream(*stream);
}

bool StreamHandler::deleteStream(const QString& name)
{
	DB::Streams* db = DB::Connector::instance()->streamConnector();
	return db->deleteStream(name);
}

bool StreamHandler::update(const QString& name, StationPtr station)
{
	DB::Streams* db = DB::Connector::instance()->streamConnector();
	auto* stream = dynamic_cast<Stream*>(station.get());
	if(!stream){
		return false;
	}

	return db->updateStream(name, *stream);
}

StationPtr StreamHandler::createStreamInstance(const QString& name, const QString& url) const
{
	return std::make_shared<Stream>(name, url);
}

StationPtr StreamHandler::station(const QString& name)
{
	DB::Streams* db = DB::Connector::instance()->streamConnector();

	Stream stream = db->getStream(name);
	if(stream.name().isEmpty()){
		return nullptr;
	}

	return std::make_shared<Stream>(stream);
}
