/* SomaFMLibrary.cpp */

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
	QString path = Util::sayonaraPath("somafm.ini");

	m->qsettings = new QSettings(path, QSettings::IniFormat, this);
}

SomaFM::Library::~Library()
{
	m->qsettings->deleteLater();
}


void SomaFM::Library::searchStations()
{
	emit sigLoadingStarted();

	AsyncWebAccess* awa = new AsyncWebAccess(this);
	connect(awa, &AsyncWebAccess::sigFinished, this, &SomaFM::Library::websiteFetched);

	awa->run("https://somafm.com/listen/");
}


SomaFM::Station SomaFM::Library::station(const QString& name)
{
	m->requested_station = name;
	return m->station_map[name];
}


void SomaFM::Library::websiteFetched()
{
	auto* awa = static_cast<AsyncWebAccess*>(sender());
	QList<SomaFM::Station> stations;

	if(awa->status() != AsyncWebAccess::Status::GotData)
	{
		awa->deleteLater();

		emit sigStationsLoaded(stations);
		emit sigLoadingFinished();

		return;
	}

	QString content = QString::fromUtf8(awa->data());
	QStringList station_contents = content.split("<li");

	for(const QString& station_content : station_contents)
	{
		SomaFM::Station station(station_content);
		if(!station.isValid()){
			continue;
		}

		bool loved = m->qsettings->value(station.name(), false).toBool();

		station.setLoved( loved );

		m->station_map[station.name()] = station;
		stations << station;
	}

	sortStations(stations);

	emit sigStationsLoaded(stations);
	emit sigLoadingFinished();

	awa->deleteLater();
}

void SomaFM::Library::createPlaylistFromStation(int row)
{
	Q_UNUSED(row)

	emit sigLoadingStarted();

	SomaFM::Station station = m->station_map[m->requested_station];

	auto* parser = new StreamParser(this);
	connect(parser, &StreamParser::sigFinished, this, &SomaFM::Library::stationStreamsFetched);
	parser->parse(station.playlists());
}

void SomaFM::Library::stationStreamsFetched(bool success)
{
	auto* parser = dynamic_cast<StreamParser*>(sender());

	if(success)
	{
		MetaDataList tracks  = parser->tracks();

		SomaFM::Station station = m->station_map[m->requested_station];

		parseMetadataForPlaylist(tracks, station);

		station.setMetadata(tracks);

		m->station_map[m->requested_station] = station;

		auto* plh = Playlist::Handler::instance();
		plh->createPlaylist(tracks,
							 station.name(),
							 true,
							 Playlist::Type::Stream);
	}

	sender()->deleteLater();
	emit sigLoadingFinished();
}


bool SomaFM::Library::createPlaylistFromStreamlist(int idx)
{
	SomaFM::Station station = m->station_map[m->requested_station];
	QStringList urls = station.playlists();

	if( !Util::between(idx, urls)) {
		return false;
	}

	emit sigLoadingStarted();

	QString url = urls[idx];

	auto* stream_parser = new StreamParser(this);
	connect(stream_parser, &StreamParser::sigFinished, this, &SomaFM::Library::playlistContentFetched);
	stream_parser->parse(station.name(), url);

	return true;
}

void SomaFM::Library::playlistContentFetched(bool success)
{
	auto* parser = static_cast<StreamParser*>(sender());

	if(success) 
	{
		MetaDataList v_md = parser->tracks();
		SomaFM::Station station = m->station_map[m->requested_station];

		parseMetadataForPlaylist(v_md, station);
	
		station.setMetadata(v_md);

		m->station_map[m->requested_station] = station;

		auto* plh = Playlist::Handler::instance();
		plh->createPlaylist(v_md,
							 station.name(),
							 true,
							 Playlist::Type::Stream);
	}

	sender()->deleteLater();
	emit sigLoadingFinished();
}


void SomaFM::Library::setStationLoved(const QString& station_name, bool loved)
{
	m->station_map[station_name].setLoved(loved);
	m->qsettings->setValue(station_name, loved);

	QList<SomaFM::Station> stations;

	for(auto it=m->station_map.cbegin(); it!=m->station_map.cend(); it++)
	{
		if(it.key().isEmpty()){
			continue;
		}

		stations << it.value();
	}

	sortStations(stations);
	emit sigStationsLoaded(stations);
}


void SomaFM::Library::sortStations(QList<SomaFM::Station>& stations)
{
	auto lambda = [](const SomaFM::Station& s1, const SomaFM::Station& s2){
		if(s1.isLoved() && !s2.isLoved()){
			return true;
		}

		else if(!s1.isLoved() && s2.isLoved()){
			return false;
		}

		return s1.name() < s2.name();
	};

	Algorithm::sort(stations, lambda);
}

void SomaFM::Library::parseMetadataForPlaylist(MetaDataList& tracks, const SomaFM::Station& station)
{
	const Cover::Location cl = station.coverLocation();
	const QList<Cover::Fetcher::Url> searchUrls = cl.searchUrls();

	QStringList coverUrls;
	Util::Algorithm::transform(searchUrls, coverUrls, [](auto url)
	{
		return url.url();
	});

	for(MetaData& md : tracks)
	{
		md.setCoverDownloadUrls(coverUrls);
		md.addCustomField("cover-hash", "", cl.hash());

		const QString filepath = md.filepath();
		md.setRadioStation(filepath, station.name());

		if(filepath.toLower().contains("mp3")){
			md.setTitle(station.name() + " (mp3)");
		}

		else if(filepath.toLower().contains("aac")){
			md.setTitle(station.name() + " (aac)");
		}
	}

	tracks.sort(::Library::SortOrder::TrackTitleAsc);
}
