/* SomaFMLibrary.cpp */

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

/* SomaFMLibrary.cpp */

#include "SomaFMLibrary.h"
#include "SomaFMStation.h"

#include "Utils/Utils.h"
#include "Utils/Algorithm.h"
#include "Utils/FileUtils.h"
#include "Utils/WebAccess/AsyncWebAccess.h"
#include "Utils/Parser/StreamParser.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Logger/Logger.h"

#include "Components/Playlist/PlaylistHandler.h"
#include "Components/Covers/CoverLocation.h"
#include "Components/Covers/CoverFetchManager.h"
#include "Components/Covers/Fetcher/CoverFetcherUrl.h"

#include <QMap>
#include <QSettings>

namespace Algorithm=Util::Algorithm;

struct SomaFM::Library::Private
{
	QMap<QString, SomaFM::Station> 	station_map;
	QString 						requested_station;
	QSettings*						qsettings=nullptr;
};

SomaFM::Library::Library(QObject* parent) :
	QObject(parent)
{
	m = Pimpl::make<Private>();
	QString path = Util::sayonara_path("somafm.ini");

	m->qsettings = new QSettings(path, QSettings::IniFormat, this);
}

SomaFM::Library::~Library()
{
	m->qsettings->deleteLater();
}


void SomaFM::Library::search_stations()
{
	emit sig_loading_started();

	AsyncWebAccess* awa = new AsyncWebAccess(this);
	connect(awa, &AsyncWebAccess::sig_finished, this, &SomaFM::Library::soma_website_fetched);

	awa->run("https://somafm.com/listen/");
}


SomaFM::Station SomaFM::Library::station(const QString& name)
{
	m->requested_station = name;
	return m->station_map[name];
}


void SomaFM::Library::soma_website_fetched()
{
	auto* awa = static_cast<AsyncWebAccess*>(sender());
	QList<SomaFM::Station> stations;

	if(awa->status() != AsyncWebAccess::Status::GotData)
	{
		awa->deleteLater();

		emit sig_stations_loaded(stations);
		emit sig_loading_finished();

		return;
	}

	QString content = QString::fromUtf8(awa->data());
	QStringList station_contents = content.split("<li");

	for(const QString& station_content : station_contents)
	{
		SomaFM::Station station(station_content);
		if(!station.is_valid()){
			continue;
		}

		bool loved = m->qsettings->value(station.name(), false).toBool();

		station.set_loved( loved );

		m->station_map[station.name()] = station;
		stations << station;
	}

	sort_stations(stations);

	emit sig_stations_loaded(stations);
	emit sig_loading_finished();

	awa->deleteLater();
}

void SomaFM::Library::create_playlist_from_station(int row)
{
	Q_UNUSED(row)

	emit sig_loading_started();

	SomaFM::Station station = m->station_map[m->requested_station];

	auto* parser = new StreamParser(station.name(), this);
	connect(parser, &StreamParser::sig_finished, this, &SomaFM::Library::soma_station_playlists_fetched);
	parser->parse_streams(station.playlists());
}

void SomaFM::Library::soma_station_playlists_fetched(bool success)
{
	auto* parser = dynamic_cast<StreamParser*>(sender());

	if(success)
	{
		MetaDataList v_md  = parser->metadata();

		SomaFM::Station station = m->station_map[m->requested_station];

		prepare_metadata_for_playlist(v_md, station);

		station.set_metadata(v_md);

		m->station_map[m->requested_station] = station;

		auto* plh = Playlist::Handler::instance();
		plh->create_playlist(v_md,
							 station.name(),
							 true,
							 Playlist::Type::Stream);
	}

	sender()->deleteLater();
	emit sig_loading_finished();
}


bool SomaFM::Library::create_playlist_from_playlist(int idx)
{
	SomaFM::Station station = m->station_map[m->requested_station];
	QStringList urls = station.playlists();

	if( !Util::between(idx, urls)) {
		return false;
	}

	emit sig_loading_started();

	QString url = urls[idx];
	auto* stream_parser = new StreamParser(station.name(), this);
	connect(stream_parser, &StreamParser::sig_finished, this, &SomaFM::Library::soma_playlist_content_fetched);

	stream_parser->parse_stream(url);

	return true;
}

void SomaFM::Library::soma_playlist_content_fetched(bool success)
{
	auto* parser = static_cast<StreamParser*>(sender());

	if(success) 
	{
		MetaDataList v_md = parser->metadata();
		SomaFM::Station station = m->station_map[m->requested_station];

		prepare_metadata_for_playlist(v_md, station);
	
		station.set_metadata(v_md);

		m->station_map[m->requested_station] = station;

		auto* plh = Playlist::Handler::instance();
		plh->create_playlist(v_md,
							 station.name(),
							 true,
							 Playlist::Type::Stream);
	}

	sender()->deleteLater();
	emit sig_loading_finished();
}


void SomaFM::Library::set_station_loved(const QString& station_name, bool loved)
{
	m->station_map[station_name].set_loved(loved);
	m->qsettings->setValue(station_name, loved);

	QList<SomaFM::Station> stations;

	for(auto it=m->station_map.cbegin(); it!=m->station_map.cend(); it++)
	{
		if(it.key().isEmpty()){
			continue;
		}

		stations << it.value();
	}

	sort_stations(stations);
	emit sig_stations_loaded(stations);
}


void SomaFM::Library::sort_stations(QList<SomaFM::Station>& stations)
{
	auto lambda = [](const SomaFM::Station& s1, const SomaFM::Station& s2){
		if(s1.is_loved() && !s2.is_loved()){
			return true;
		}

		else if(!s1.is_loved() && s2.is_loved()){
			return false;
		}

		return s1.name() < s2.name();
	};

	Algorithm::sort(stations, lambda);
}

void SomaFM::Library::prepare_metadata_for_playlist(MetaDataList& v_md, const SomaFM::Station& station)
{
	Cover::Location cl = station.cover_location();
	QList<Cover::Fetcher::Url> search_urls = cl.search_urls(true);

	QStringList cover_urls;
	for(auto url : search_urls)
	{
		cover_urls << url.url();
	}

	for(MetaData& md : v_md)
	{
		md.set_cover_download_urls(cover_urls);

		QString filepath = md.filepath();
		md.set_radio_station(filepath);

		if(filepath.toLower().contains("mp3")){
			md.set_title(station.name() + " (mp3)");
		}

		else if(filepath.toLower().contains("aac")){
			md.set_title(station.name() + " (aac)");
		}
	}

	v_md.sort(::Library::SortOrder::TrackTitleAsc);
}
