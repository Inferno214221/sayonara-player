/* ArtistInfo.cpp */

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

#include "ArtistInfo.h"

#include "Covers/CoverLocation.h"
#include "Database/Connector.h"
#include "Database/LibraryDatabase.h"
#include "Utils/Algorithm.h"
#include "Utils/Language/Language.h"
#include "Utils/MetaData/Artist.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/SimilarArtists/SimilarArtists.h"

#include <optional>

namespace
{
	using AdditionalInfo = LibraryItemInfo::AdditionalInfo;
	using ArtistSimilarity = std::pair<double, QString>;
	using StringSet = Util::Set<QString>;

	QList<ArtistSimilarity> convertSimilarArtistsToList(const QMap<QString, double>& similarArtists)
	{
		auto result = QList<ArtistSimilarity>();
		for(auto it = similarArtists.cbegin(); it != similarArtists.cend(); it++)
		{
			result << std::make_pair(it.value(), it.key());
		}

		Util::Algorithm::sort(result, [](const auto& p1, const auto& p2) {
			return (p2.first < p1.first);
		});

		return result;
	}

	AdditionalInfo calcSimilarArtistInfo(const QString& artistName)
	{
		const auto similarArtists = SimilarArtists::getSimilarArtists(artistName);
		const auto similarArtistList = convertSimilarArtistsToList(similarArtists);

		auto additionalInfo = AdditionalInfo();
		additionalInfo << StringPair(Lang::get(Lang::SimilarArtists), QString()); // just a header

		for(const auto& [similarity, name]: similarArtistList)
		{
			const auto iSimilarity = static_cast<int>(similarity * 100);
			additionalInfo << StringPair {name, QString("%1%").arg(iSimilarity)};
		}

		return additionalInfo;
	}

	std::optional<Artist> getArtistFromDatabase(const ArtistId artistId, const DbId databaseId)
	{
		auto* libraryDatabase = DB::Connector::instance()->libraryDatabase(-1, databaseId);

		auto artist = Artist {};
		const auto success = libraryDatabase && libraryDatabase->getArtistByID(artistId, artist);

		return success ? std::optional {artist} : std::nullopt;
	}

	Cover::Location calcCoverLocation(const StringSet& artists, const StringSet& albumArtists)
	{
		if(artists.size() == 1)
		{
			return Cover::Location::coverLocation(artists.first());
		}

		if(albumArtists.size() == 1)
		{
			return Cover::Location::coverLocation(albumArtists.first());
		}

		return Cover::Location::invalidLocation();
	}

	AdditionalInfo
	calcAdditionalInfo(const DbId databaseId, const StringSet& albums, const IdSet& artistIds, const StringSet& artists)
	{
		auto additionalInfo = AdditionalInfo {};
		const auto albumCountString = LibraryItemInfo::convertInfoKeyToString(InfoStrings::AlbumCount);
		additionalInfo << StringPair(albumCountString, QString::number(albums.count()));

		if(artistIds.size() == 1)
		{
			const auto artist = getArtistFromDatabase(artistIds.first(), databaseId);
			if(artist.has_value())
			{
				additionalInfo << calcSimilarArtistInfo(artists.first());
			}
		}

		else if(artists.size() > 1)
		{
			const auto artistCountString = LibraryItemInfo::convertInfoKeyToString(InfoStrings::ArtistCount);
			additionalInfo << StringPair(artistCountString, QString::number(artists.count()));
		}

		return additionalInfo;
	}
}

struct ArtistInfo::Private
{
	AdditionalInfo additionalInfo;
	Cover::Location coverLocation;
	QString header;
	QString subheader;
};

ArtistInfo::ArtistInfo(const MetaDataList& metaDataList) :
	LibraryItemInfo(metaDataList),
	m {Pimpl::make<Private>()}
{
	const auto databaseId = metaDataList.isEmpty()
	                        ? DbId(-1)
	                        : metaDataList[0].databaseId();

	m->header = calcArtistString();
	m->subheader = QString();
	m->coverLocation = calcCoverLocation(artists(), albumArtists());
	m->additionalInfo = calcAdditionalInfo(databaseId, albums(), artistIds(), artists());
}

ArtistInfo::~ArtistInfo() = default;

Cover::Location ArtistInfo::coverLocation() const { return m->coverLocation; }

QString ArtistInfo::header() const { return m->header; }

QString ArtistInfo::subheader() const { return m->subheader; }

AdditionalInfo ArtistInfo::additionalData() const { return m->additionalInfo; }
