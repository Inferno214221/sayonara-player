/* StationSearcher.cpp */

/* Copyright (C) 2011-2024 Michael Lugmair (Lucio Carreras)
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

#include "StationSearcher.h"
#include "FMStreamParser.h"
#include "RadioStation.h"
#include "Utils/WebAccess/WebClientImpl.h"

#include <QUrl>
#include <QList>

namespace
{
	constexpr const auto EntriesPerPage = 200;
	constexpr const auto MinimumEntrySize = 50;
}

struct StationSearcher::Private
{
	QString searchString;
	QList<RadioStation> foundStations;
	int currentPageIndex {0};
	int lastPageIndex {1};
	StationSearcher::Mode mode {StationSearcher::NewSearch};

	[[nodiscard]] QString url() const
	{
		if(mode == StationSearcher::Style)
		{
			return QString("http://fmstream.org/index.php?style=%1")
				.arg(searchstring);
		}

		if(currentPageIndex == 0)
		{
			return QString("http://fmstream.org/index.php?s=%1&cm=0")
				.arg(searchstring);
		}

		return QString("http://fmstream.org/index.php?s=%1&n=%2")
			.arg(searchstring)
			.arg(currentPageIndex);
	}
};

StationSearcher::StationSearcher(QObject* parent) :
	QObject {parent},
	m {Pimpl::make<Private>()} {}

StationSearcher::~StationSearcher() = default;

void StationSearcher::startCall()
{
	auto* webClient = new WebClientImpl(this);
	connect(webClient, &WebClient::sigFinished, this, &StationSearcher::searchFinished);
	webClient->run(m->url());
}

void StationSearcher::searchStyle(const QString& style)
{
	m->mode = StationSearcher::Style;
	m->searchString = style;

	startCall();
}

void StationSearcher::searchStation(const QString& name)
{
	m->mode = StationSearcher::NewSearch;
	m->currentPageIndex = 0;
	m->lastPageIndex = -1;
	m->searchString = name;

	startCall();
}

void StationSearcher::searchPrevious()
{
	m->currentPageIndex -= EntriesPerPage;
	m->mode = StationSearcher::Incremental;

	startCall();
}

void StationSearcher::searchNext()
{
	m->currentPageIndex += EntriesPerPage;
	m->mode = StationSearcher::Incremental;

	startCall();
}

bool StationSearcher::canSearchNext() const
{
	return (m->foundStations.size() > MinimumEntrySize) &&
	       (m->currentPageIndex != m->lastPageIndex);
}

bool StationSearcher::canSearchPrevious() const { return (m->currentPageIndex > 0); }

StationSearcher::Mode StationSearcher::mode() const { return m->mode; }

const QList<RadioStation>& StationSearcher::foundStations() const { return m->foundStations; }

void StationSearcher::searchFinished()
{
	auto* webClient = dynamic_cast<WebClient*>(sender());

	const auto parser = FMStreamParser(webClient->data());
	auto stations = parser.stations();
	if(stations.isEmpty())
	{
		m->currentPageIndex -= EntriesPerPage;
		m->lastPageIndex = m->currentPageIndex;
	}

	else
	{
		m->foundStations = std::move(stations);
	}

	webClient->deleteLater();

	emit sigStationsFound();
}
