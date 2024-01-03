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
#include "ArtistMatch.h"
#include "Components/Streaming/LastFM/LFMWebAccess.h"
#include "Components/Streaming/LastFM/LFMGlobals.h"
#include "LfmSimiliarArtistsParser.h"

#include "Utils/Logger/Logger.h"

#include <QHash>
#include <QUrl>

using DynamicPlayback::ArtistMatch;
using DynamicPlayback::LfmSimilarArtistFetcher;
using DynamicPlayback::SimilarArtistFetcher;

using LastFM::WebAccess;

struct LfmSimilarArtistFetcher::Private
{
	QString artist;
	ArtistMatch artistMatch;
	QHash<QString, ArtistMatch> similarArtistsCache;
};

LfmSimilarArtistFetcher::LfmSimilarArtistFetcher(const QString& artist, QObject* parent) :
	SimilarArtistFetcher(artist, parent)
{
	m = Pimpl::make<Private>();
}

LfmSimilarArtistFetcher::~LfmSimilarArtistFetcher() = default;

const ArtistMatch& LfmSimilarArtistFetcher::similarArtists() const
{
	return m->artistMatch;
}

void LfmSimilarArtistFetcher::fetchSimilarArtists(const QString& artistName)
{
	m->artist = artistName;

	// check if already in cache
	if(m->similarArtistsCache.contains(m->artist))
	{
		m->artistMatch = m->similarArtistsCache.value(m->artist);
		emit sigFinished();
		return;
	}

	auto* webAccess = new WebAccess();
	connect(webAccess, &WebAccess::sigResponse, this, &LfmSimilarArtistFetcher::responseReceived);
	connect(webAccess, &WebAccess::sigError, this, &LfmSimilarArtistFetcher::errorReceived);
	connect(webAccess, &WebAccess::sigFinished, this, &LfmSimilarArtistFetcher::sigFinished);
	connect(webAccess, &WebAccess::sigFinished, webAccess, &QObject::deleteLater);

	const auto url =
		QString("http://ws.audioscrobbler.com/2.0/?"
		        "method=artist.getsimilar&"
		        "artist=%1&api_key=%2")
			.arg(QString::fromUtf8(QUrl::toPercentEncoding(m->artist)))
			.arg(QString::fromUtf8(LastFM::ApiKey));

	webAccess->callUrl(url);
}

void LfmSimilarArtistFetcher::responseReceived(const QByteArray& data)
{
	m->artistMatch = parseLastFMAnswer(m->artist, data);
	if(m->artistMatch.isValid())
	{
		m->similarArtistsCache[m->artist] = m->artistMatch;

		// maybe lastfm corrected the artist name
		const auto artistName = m->artistMatch.artistName();
		if(m->artist != artistName)
		{
			m->similarArtistsCache[artistName] = m->artistMatch;
		}
	}
}

void LfmSimilarArtistFetcher::errorReceived(const QString& answer)
{
	spLog(Log::Warning, this) << "Could not fetch similar artists: " << answer;
}

