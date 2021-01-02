/* ArtistMatchEvaluator.cpp */
/*
 * Copyright (C) 2011-2020 Michael Lugmair
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
#include "ArtistMatchEvaluator.h"
#include "ArtistMatch.h"

#include "Database/Connector.h"
#include "Database/LibraryDatabase.h"

#include "Utils/Utils.h"
#include "Utils/Algorithm.h"
#include "Utils/RandomGenerator.h"
#include "Utils/MetaData/Artist.h"
#include "Utils/Library/SearchMode.h"

#include <QByteArray>
#include <QMap>

using DynamicPlayback::ArtistMatch;

namespace
{
	using ArtistLookupMap = QMap<QString, QList<ArtistId>>;

	class NameUnificator
	{
		private:
			QMap<QString, QString> mNameMap;

		public:
			QString unifyString(const QString& artistName)
			{
				if(mNameMap.contains(artistName))
				{
					return mNameMap[artistName];
				}

				const auto result =
					Library::Utils::convertSearchstring(artistName,
					                                    (Library::CaseInsensitve |
					                                     Library::NoSpecialChars |
					                                     Library::NoDiacriticChars));

				mNameMap[artistName] = result;
				return result;
			}
	};

	static NameUnificator nameUnificator;

	ArtistMatch::Quality generateQuality()
	{
		const auto randomNumber = Util::randomNumber(0, 100);
		if(randomNumber > 60) // [100 - 60[ = 40 %
		{
			return ArtistMatch::Quality::Excellent;
		}

		else if(randomNumber > 30) // [60 - 30[ = 30%
		{
			return ArtistMatch::Quality::VeryGood;
		}

		else if(randomNumber > 10) // [30 - 10[ = 20%
		{
			return ArtistMatch::Quality::Good;
		}

		else  // [0 - 10[ = 10%
		{
			return ArtistMatch::Quality::Poor;
		}
	}

	ArtistMatch::Quality nextQuality(ArtistMatch::Quality quality)
	{
		// Excellent -> Very Good -> Good -> Poor -> Excellent -> Very Good...
		switch(quality)
		{
			case ArtistMatch::Quality::Excellent:
				return ArtistMatch::Quality::VeryGood;
			case ArtistMatch::Quality::VeryGood:
				return ArtistMatch::Quality::Good;
			case ArtistMatch::Quality::Good:
				return ArtistMatch::Quality::Poor;
			case ArtistMatch::Quality::Poor:
			default:
				return ArtistMatch::Quality::Excellent;
		}
	}

	ArtistLookupMap createArtistLookupMap(const ArtistList& artists)
	{
		ArtistLookupMap lookupMap;

		for(const auto& artist : artists)
		{
			const auto& transformedName = nameUnificator.unifyString(artist.name());
			if(!lookupMap.contains(transformedName))
			{
				lookupMap.insert(transformedName, QList<ArtistId> {artist.id()});
			}
			else
			{
				lookupMap[transformedName].push_back(artist.id());
			}
		}

		return lookupMap;
	}

	ArtistLookupMap getAllAvailableArtistsInLibrary(LibraryId libraryId)
	{
		auto* db = DB::Connector::instance();
		auto* libraryDatabase = db->libraryDatabase(libraryId, 0);

		ArtistList artists;
		libraryDatabase->getAllArtists(artists, false);

		return createArtistLookupMap(artists);
	}

	QList<ArtistId>
	filterAvailableArtists(const ArtistMatch& artistMatch, ArtistMatch::Quality quality, const ArtistLookupMap& lookupMap)
	{
		QList<ArtistId> artistIds;

		const auto entries = artistMatch.get(quality);
		for(const auto& entry : entries)
		{
			const auto name = nameUnificator.unifyString(entry.artist);
			const auto artistIdList = lookupMap[name];

			std::copy(artistIdList.begin(), artistIdList.end(), std::back_inserter(artistIds));
		}

		Util::Algorithm::shuffle(artistIds);

		return artistIds;
	}
}

QList<ArtistId>
DynamicPlayback::evaluateArtistMatch(const ArtistMatch& artistMatch, LibraryId libraryId)
{
	if(!artistMatch.isValid())
	{
		return QList<ArtistId> {};
	}

	const auto lookupMap = getAllAvailableArtistsInLibrary(libraryId);

	QList<ArtistId> artistIds;
	const auto originalQuality = generateQuality();
	auto quality = originalQuality;
	do
	{
		auto artistIdsForQuality = filterAvailableArtists(artistMatch, quality, lookupMap);
		std::move(artistIdsForQuality.begin(), artistIdsForQuality.end(), std::back_inserter(artistIds));

		quality = nextQuality(quality);
	} while(quality != originalQuality);

	return artistIds;
}