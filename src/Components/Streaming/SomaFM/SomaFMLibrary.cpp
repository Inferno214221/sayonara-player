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
#include "SomaFMUtils.h"

#include "Utils/Utils.h"
#include "Utils/Algorithm.h"
#include "Utils/WebAccess/AsyncWebAccess.h"
#include "Utils/Parser/StreamParser.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/StandardPaths.h"

#include "Components/Covers/CoverLocation.h"
#include "Components/Covers/CoverFetchManager.h"
#include "Components/Covers/Fetcher/CoverFetcherUrl.h"

#include "Interfaces/PlaylistCreator.h"

#include <QMap>
#include <QSettings>

namespace Algorithm = Util::Algorithm;

static
void sortStations(QList<SomaFM::Station>& stations)
{
	auto lambda = [](const SomaFM::Station& s1, const SomaFM::Station& s2) {
		if(s1.isLoved() && !s2.isLoved())
		{
			return true;
		}

		else if(!s1.isLoved() && s2.isLoved())
		{
			return false;
		}

		return s1.name() < s2.name();
	};

	Algorithm::sort(stations, lambda);
}

struct SomaFM::Library::Private
{
	QMap<QString, SomaFM::Station> stationMap;
	QString requestedStation;
	QSettings* qsettings = nullptr;
	PlaylistCreator* playlistCreator;

	Private(PlaylistCreator* playlistCreator, QObject* parent) :
		qsettings{new QSettings(Util::xdgConfigPath("somafm.ini"), QSettings::IniFormat, parent)},
		playlistCreator {playlistCreator}
	{}

	~Private() = default;

	int timeout() const
	{
		return qsettings->value("timeout", 5000).toInt();
	}
};

SomaFM::Library::Library(PlaylistCreator* playlistCreator, QObject* parent) :
	QObject(parent)
{
	m = Pimpl::make<Private>(playlistCreator, this);
}

SomaFM::Library::~Library() = default;

void SomaFM::Library::searchStations()
{
	emit sigLoadingStarted();

	auto* awa = new AsyncWebAccess(this);
	connect(awa, &AsyncWebAccess::sigFinished, this, &SomaFM::Library::websiteFetched);

	awa->run("https://somafm.com/listen/", m->timeout());
}

SomaFM::Station SomaFM::Library::station(const QString& name)
{
	m->requestedStation = name;
	return m->stationMap[name];
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

	const auto content = QString::fromUtf8(awa->data());

	auto re = QRegExp("<li\\s?(.+)</li>");
	re.setMinimal(true);

	auto index = re.indexIn(content);
	while(index >= 0)
	{
		const auto stationContent = re.cap(1);
		auto station = SomaFM::Station(stationContent);
		if(station.isValid())
		{
			const auto loved = m->qsettings->value(station.name(), false).toBool();
			station.setLoved(loved);

			m->stationMap[station.name()] = station;
			stations << station;
		}

		index = re.indexIn(content, index + 1);
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

	auto* parser = new StreamParser(this);
	connect(parser, &StreamParser::sigFinished, this, &SomaFM::Library::stationStreamsFetched);

	const auto& station = m->stationMap[m->requestedStation];
	parser->parse(station.playlists(), m->timeout());
}

void SomaFM::Library::stationStreamsFetched(bool success)
{
	auto* parser = static_cast<StreamParser*>(sender());

	if(success)
	{
		auto tracks = parser->tracks();
		auto& station = m->stationMap[m->requestedStation];

		SomaFM::Utils::mapStationToMetadata(station, tracks);
		station.setMetadata(tracks);

		m->playlistCreator->createPlaylist(tracks, station.name(), true);
	}

	sender()->deleteLater();
	emit sigLoadingFinished();
}

bool SomaFM::Library::createPlaylistFromStreamlist(int idx)
{
	const auto station = m->stationMap[m->requestedStation];

	const auto urls = station.playlists();
	if(!Util::between(idx, urls))
	{
		return false;
	}

	emit sigLoadingStarted();

	auto* streamParser = new StreamParser(this);
	connect(streamParser, &StreamParser::sigFinished, this, &SomaFM::Library::playlistContentFetched);
	streamParser->parse(station.name(), urls[idx], m->timeout());

	return true;
}

void SomaFM::Library::playlistContentFetched(bool success)
{
	auto* parser = static_cast<StreamParser*>(sender());

	if(success)
	{
		auto tracks = parser->tracks();
		auto& station = m->stationMap[m->requestedStation];

		SomaFM::Utils::mapStationToMetadata(station, tracks);
		station.setMetadata(tracks);

		m->playlistCreator->createPlaylist(tracks, station.name(), true);
	}

	sender()->deleteLater();
	emit sigLoadingFinished();
}

void SomaFM::Library::setStationLoved(const QString& stationName, bool loved)
{
	m->stationMap[stationName].setLoved(loved);
	m->qsettings->setValue(stationName, loved);

	QList<SomaFM::Station> stations;
	for(auto it = m->stationMap.cbegin(); it != m->stationMap.cend(); it++)
	{
		if(!it.key().isEmpty())
		{
			stations << it.value();
		}
	}

	sortStations(stations);
	emit sigStationsLoaded(stations);
}
