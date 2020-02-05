/* AbstractStreamHandler.cpp */

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

#include "AbstractStationHandler.h"

#include "Database/Connector.h"
#include "Components/Playlist/PlaylistHandler.h"
#include "Components/PlayManager/PlayManager.h"

#include "Utils/WebAccess/AsyncWebAccess.h"
#include "Utils/Parser/StreamParser.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Message/Message.h"
#include "Utils/Settings/Settings.h"

struct AbstractStationHandler::Private
{
	StreamParser*					stream_parser=nullptr;
	Playlist::Handler*				playlist=nullptr;
	QMap<QString, MetaDataList>		station_contents;
	QString							station_name;
	bool							blocked;

	Private()
	{
		playlist = Playlist::Handler::instance();
		blocked = false;
	}
};

AbstractStationHandler::AbstractStationHandler(QObject *parent) :
	QObject(parent)
{
	m = Pimpl::make<AbstractStationHandler::Private>();
}

AbstractStationHandler::~AbstractStationHandler() = default;

void AbstractStationHandler::clear()
{
	m->station_contents.clear();
}

void AbstractStationHandler::stop()
{
	if(m->stream_parser && m->blocked) {
		m->stream_parser->stop();
	}
}

void AbstractStationHandler::stopped()
{
	m->blocked = false;
	emit sig_stopped();

	sender()->deleteLater();
	m->stream_parser = nullptr;
}


bool AbstractStationHandler::parse_station(const QString& url, const QString& station_name)
{
	if(m->blocked) {
		return false;
	}

	m->blocked = true;
	m->station_name = station_name;

	m->stream_parser = new StreamParser(station_name, this);
	connect(m->stream_parser, &StreamParser::sig_finished, this, &AbstractStationHandler::stream_parser_finished);
	connect(m->stream_parser, &StreamParser::sig_too_many_urls_found, this, &AbstractStationHandler::sig_too_many_urls_found);
	connect(m->stream_parser, &StreamParser::sig_stopped, this, &AbstractStationHandler::stopped);

	m->stream_parser->parse_stream(url);

	return true;
}


void AbstractStationHandler::stream_parser_finished(bool success)
{
	auto* stream_parser = static_cast<StreamParser*>(sender());

	if(!success)
	{
		sp_log(Log::Warning, this) << "Stream parser finished with error";
		stream_parser->deleteLater(); m->stream_parser = nullptr;

		m->blocked = false;
		emit sig_error();
	}

	else
	{
		MetaDataList v_md = stream_parser->metadata();
		m->station_contents[m->station_name] = v_md;

		if(!v_md.isEmpty())
		{
			QString station_name;
			if(GetSetting(Set::Stream_NewTab)){
				station_name = m->station_name;
			}

			int idx = m->playlist->create_playlist(v_md, station_name, true, Playlist::Type::Stream);
			m->playlist->change_track(0, idx);
		}

		m->blocked = false;
		emit sig_data_available();
	}

	stream_parser->deleteLater(); m->stream_parser = nullptr;
}

bool AbstractStationHandler::save(StationPtr station)
{
	return add_stream(station);
}

