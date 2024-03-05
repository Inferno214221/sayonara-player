/* SomaFMAsyncDropHandler.cpp
 *
 * Copyright (C) 2011-2024 Michael Lugmair
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

#include "SomaFMAsyncDropHandler.h"

#include "Components/Covers/CoverLocation.h"
#include "Components/Covers/Fetcher/CoverFetcherUrl.h"
#include "Components/Streaming/SomaFM/SomaFMStation.h"
#include "Components/Streaming/SomaFM/SomaFMUtils.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Parser/StreamParser.h"
#include "Utils/WebAccess/WebClientFactory.h"

#include <QUrl>

using SomaFM::AsyncDropHandler;
using SomaFM::Station;

struct AsyncDropHandler::Private
{
	Station station;

	Private(const Station& station) :
		station(station) {}
};

AsyncDropHandler::AsyncDropHandler(const SomaFM::Station& station, QObject* parent) :
	Gui::AsyncDropHandler(parent)
{
	m = Pimpl::make<Private>(station);
}

AsyncDropHandler::~AsyncDropHandler() = default;

void AsyncDropHandler::start()
{
	QStringList files = m->station.playlists();

	auto stationParserFactory =
		StationParserFactory::createStationParserFactory(std::make_shared<WebClientFactory>(), this);
	auto* streamParser = stationParserFactory->createParser();

	const Cover::Location cl = m->station.coverLocation();
	auto searchUrls = cl.searchUrls();
	if(!searchUrls.isEmpty())
	{
		const QString coverUrl = searchUrls.first().url();
		streamParser->setCoverUrl(coverUrl);
	}

	connect(streamParser, &StreamParser::sigFinished, this, &AsyncDropHandler::streamParserFinished);
	streamParser->parse(m->station.name(), files, {}, 5000); // NOLINT(*-magic-numbers)
}

void AsyncDropHandler::streamParserFinished(bool success)
{
	Q_UNUSED(success)

	auto* streamParser = static_cast<StreamParser*>(sender());
	MetaDataList tracks = streamParser->tracks();
	SomaFM::Utils::mapStationToMetadata(m->station, tracks);

	streamParser->deleteLater();

	Gui::AsyncDropHandler::setTracks(tracks);
}
