/* LFMSimilarArtistFetcher.cpp */
/*
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

#include "LfmSimilarArtistFetcher.h"
#include "LfmSimiliarArtistsParser.h"
#include "ArtistMatch.h"

#include "Components/Streaming/LastFM/LFMGlobals.h"
#include "Components/Streaming/LastFM/LFMWebAccess.h"
#include "Utils/Algorithm.h"
#include "Utils/Logger/Logger.h"
#include "Utils/WebAccess/WebClientFactory.h"

#include <QHash>
#include <QUrl>

namespace
{
	QString createUrl(const QString& artist)
	{
		const auto baseUrl = QString {LastFM::BaseUrl} + "?";
		const auto params = std::map<QString, QString> {
			{"method",  "artist.getsimilar"},
			{"artist",  QString::fromUtf8(QUrl::toPercentEncoding(artist))},
			{"api_key", QString::fromUtf8(LastFM::ApiKey)}
		};

		auto lst = QStringList {};
		Util::Algorithm::transform(params, lst, [](const auto& p) {
			return p.first + "=" + p.second;
		});

		return baseUrl + lst.join("&");
	}
}

namespace DynamicPlayback
{
	struct LfmSimilarArtistFetcher::Private
	{
		WebClientFactoryPtr webClientFactory;
		QString artist;
		ArtistMatch artistMatch;

		explicit Private(WebClientFactoryPtr webClientFactory) :
			webClientFactory {std::move(webClientFactory)} {}
	};

	LfmSimilarArtistFetcher::LfmSimilarArtistFetcher(const QString& artist, const WebClientFactoryPtr& webClientFactory,
	                                                 QObject* parent) :
		SimilarArtistFetcher(artist, parent),
		m {Pimpl::make<Private>(webClientFactory)} {}

	LfmSimilarArtistFetcher::~LfmSimilarArtistFetcher() = default;

	const ArtistMatch& LfmSimilarArtistFetcher::similarArtists() const { return m->artistMatch; }

	void LfmSimilarArtistFetcher::fetchSimilarArtists(const QString& artistName)
	{
		m->artist = artistName;

		using LastFM::WebAccess;

		auto* webAccess = new WebAccess(m->webClientFactory);
		connect(webAccess, &WebAccess::sigFinished, this, &LfmSimilarArtistFetcher::webClientFinished);
		connect(webAccess, &WebAccess::sigFinished, webAccess, &QObject::deleteLater);
		connect(webAccess, &WebAccess::sigFinished, this, &LfmSimilarArtistFetcher::sigFinished);

		webAccess->callUrl(createUrl(m->artist));
	}

	void LfmSimilarArtistFetcher::webClientFinished()
	{
		auto* webClient = dynamic_cast<LastFM::WebAccess*>(sender());

		const auto parsingResult = parseLastFMAnswer(webClient->data());
		if(!parsingResult.hasError)
		{
			m->artistMatch = parsingResult.artistMatch;
		}

		else
		{
			m->artistMatch = {};
			spLog(Log::Warning, this) << "Could not fetch similar artists: " << parsingResult.error;
		}
	}

	LfmSimilarArtistFetcherFactory::~LfmSimilarArtistFetcherFactory() = default;

	SimilarArtistFetcher* LfmSimilarArtistFetcherFactory::create(const QString& artist)
	{
		return new LfmSimilarArtistFetcher(artist, std::make_shared<WebClientFactory>());
	}
}