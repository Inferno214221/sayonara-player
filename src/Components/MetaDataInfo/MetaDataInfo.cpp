// clazy:excludeall=qstring-arg
/* MetaDataInfo.cpp */

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

#include "MetaDataInfo.h"

#include "Components/Covers/CoverLocation.h"
#include "Utils/Algorithm.h"
#include "Utils/Language/Language.h"
#include "Utils/MetaData/Album.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Utils.h"

#include <QStringList>

namespace
{
	using AdditionalInfo = LibraryItemInfo::AdditionalInfo;
	using StringSet = Util::Set<QString>;

	void updateAdditionalInfo(const MetaData& track, AdditionalInfo& additionalInfo)
	{
		const auto& customFields = track.customFields();
		for(const auto& field: customFields)
		{
			const auto name = field.displayName();
			const auto value = field.value();
			if(!value.isEmpty())
			{
				additionalInfo << StringPair(name, value);
			}
		}
	}

	QString convertTrackNumToString(const TrackNum trackNumber)
	{
		switch(trackNumber)
		{
			case 1:
				return Lang::get(Lang::First);
			case 2:
				return Lang::get(Lang::Second);
			case 3:
				return Lang::get(Lang::Third);
			default:
				return QString::number(trackNumber) + Lang::get(Lang::Th);
		}
	}

	QString calcHeader(const MetaDataList& tracks)
	{
		return (tracks.size() == 1)
		       ? tracks[0].title()
		       : Lang::get(Lang::VariousTracks);
	}

	QString calcSubheader(const MetaDataList& metaDataList, const QString& artistString, const QString& albumString)
	{
		const auto trackNumber = (metaDataList.size() == 1)
		                         ? metaDataList[0].trackNumber()
		                         : 0;

		const auto onString = (trackNumber > 0)
		                      ? convertTrackNumToString(trackNumber) + " " + Lang::get(Lang::TrackOn)
		                      : Lang::get(Lang::On);

		return QString("%1 <br> %2 %3")
			.arg(artistString)
			.arg(onString)
			.arg(albumString);
	}

	Cover::Location calcCoverLocation(const MetaDataList& tracks, const IdSet& albumIds, const StringSet& albums,
	                                  const StringSet& artists, const StringSet& albumArtists)
	{
		if(tracks.size() == 1)
		{
			return Cover::Location::coverLocation(tracks[0]);
		}

		if(albumIds.size() == 1)
		{
			Album album;
			album.setId(albumIds.first());
			album.setName(albums.first());
			album.setArtists(artists.toList());
			album.setDatabaseId(tracks[0].databaseId());
			if(!albumArtists.isEmpty())
			{
				album.setAlbumArtist(albumArtists.first());
			}

			QStringList pathHints;
			Util::Algorithm::transform(tracks, pathHints, [](const auto& track) {
				return track.filepath();
			});

			album.setPathHint(pathHints);

			return Cover::Location::coverLocation(album);
		}

		if(albums.size() == 1)
		{
			if(artists.size() == 1)
			{
				return Cover::Location::coverLocation(albums.first(), artists.first());
			}

			if(albumArtists.size() == 1)
			{
				return Cover::Location::coverLocation(albums.first(), albumArtists.first());
			}

			return Cover::Location::coverLocation(albums.first(), artists.toList());
		}

		return Cover::Location::invalidLocation();
	}

	AdditionalInfo calcAdditionalInfo(const MetaDataList& metaDataList)
	{
		auto additionalInfo = AdditionalInfo {};
		for(const auto& metaData: metaDataList)
		{
			updateAdditionalInfo(metaData, additionalInfo);
		}

		return additionalInfo;
	}
}

struct MetaDataInfo::Private
{
	AdditionalInfo additionalInfo;
	Cover::Location coverLocation;
	QString header;
	QString subheader;
};

MetaDataInfo::MetaDataInfo(const MetaDataList& tracks) :
	LibraryItemInfo(tracks),
	m {Pimpl::make<Private>()}
{
	m->additionalInfo = calcAdditionalInfo(tracks);
	m->coverLocation = calcCoverLocation(tracks, albumIds(), albums(), artists(), albumArtists());
	m->header = calcHeader(tracks);
	m->subheader = calcSubheader(tracks, calcArtistString(), calcAlbumString());
}

MetaDataInfo::~MetaDataInfo() = default;

AdditionalInfo MetaDataInfo::additionalData() const { return m->additionalInfo; }

Cover::Location MetaDataInfo::coverLocation() const { return m->coverLocation; }

QString MetaDataInfo::header() const { return m->header; }

QString MetaDataInfo::subheader() const { return m->subheader; }
