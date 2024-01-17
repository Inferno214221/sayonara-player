/* AlbumInfo.cpp */

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

#include "AlbumInfo.h"
#include "MetaDataInfo.h"

#include "Covers/CoverLocation.h"
#include "Database/Connector.h"
#include "Database/LibraryDatabase.h"
#include "Utils/Language/Language.h"
#include "Utils/MetaData/Album.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Utils.h"

#include <QLocale>
#include <QDateTime>
#include <QDate>
#include <QStringList>

namespace
{
	using IdSet = Util::Set<Id>;
	using StringSet = Util::Set<QString>;
	using AdditionalInfo = LibraryItemInfo::AdditionalInfo;

	QString intToDateTimeString(const uint64_t i)
	{
		const auto dateTime = Util::intToDate(i);
		const auto dateFormat = QLocale().dateFormat(QLocale::FormatType::ShortFormat);
		return dateTime.date().toString(dateFormat);
	}

	void insertSamplerInfo(const StringSet& artists, AdditionalInfo& infoFields)
	{
		const auto samplerString = LibraryItemInfo::convertInfoKeyToString(InfoStrings::Sampler);
		if(artists.size() > 1)
		{
			const auto isSamplerString = Lang::get(Lang::Yes).toLower();
			infoFields << StringPair(samplerString, isSamplerString);
		}

		else if(artists.size() == 1)
		{
			const auto isSamplerString = Lang::get(Lang::No).toLower();
			infoFields << StringPair(samplerString, isSamplerString);
		}
	}

	void insertAdditionalInfoFromAlbum(Id albumId, DB::LibraryDatabase* libraryDatabase, AdditionalInfo& additionalInfo)
	{
		auto album = Album {};
		if((libraryDatabase != nullptr) && libraryDatabase->getAlbumByID(albumId, album))
		{
			additionalInfo << QPair {Lang::get(Lang::Created), intToDateTimeString(album.creationDate())};

			const auto& customFields = album.customFields();
			for(const auto& customField: customFields)
			{
				const auto name = customField.displayName();
				const auto value = customField.value();
				if(!value.isEmpty())
				{
					additionalInfo << StringPair(name, customField.value());
				}
			}
		}
	}

	AdditionalInfo
	calcAdditionalData(const DbId databaseId, const IdSet& albumIds, const StringSet& albums, const StringSet& artists)
	{
		AdditionalInfo additionalInfo;

		if(albums.size() > 1)
		{
			additionalInfo << StringPair(
				LibraryItemInfo::convertInfoKeyToString(InfoStrings::AlbumCount),
				QString::number(albums.count()));
		}

		if(artists.size() > 1)
		{
			additionalInfo << StringPair(
				LibraryItemInfo::convertInfoKeyToString(InfoStrings::ArtistCount),
				QString::number(artists.count()));
		}

		if(albums.size() == 1)
		{
			auto* libraryDatabase = DB::Connector::instance()->libraryDatabase(-1, databaseId);

			insertSamplerInfo(artists, additionalInfo);
			insertAdditionalInfoFromAlbum(albumIds.first(), libraryDatabase, additionalInfo);
		}

		return additionalInfo;
	}

	Cover::Location extractCoverLocationFromAlbum(const Id albumId, const StringSet& albums, const StringSet& artists,
	                                              const StringSet& albumArtists, DB::LibraryDatabase* libraryDatabase)
	{
		auto album = Album {};
		const auto success = libraryDatabase && libraryDatabase->getAlbumByID(albumId, album, true);
		if(!success)
		{
			album.setId(albumId);
			album.setName(albums.first());
			album.setArtists(artists.toList());

			if(!albumArtists.isEmpty())
			{
				album.setAlbumArtist(albumArtists.first());
			}

			album.setDatabaseId(libraryDatabase ? libraryDatabase->databaseId() : DbId(-1));
		}

		return Cover::Location::coverLocation(album);
	}

	Cover::Location
	calcCoverLocation(DbId databaseId, const IdSet& albumIds, const StringSet& albums, const StringSet& artists,
	                  const StringSet& albumArtists)
	{
		if(albumIds.size() == 1) // single library album
		{
			auto* libraryDatabase = DB::Connector::instance()->libraryDatabase(-1, databaseId);
			return extractCoverLocationFromAlbum(albumIds.first(), albums, artists, albumArtists, libraryDatabase);
		}

		if(albums.size() == 1) // single non-library album
		{
			const auto album = albums.first();
			return albumArtists.isEmpty()
			       ? Cover::Location::coverLocation(album, artists.toList())
			       : Cover::Location::coverLocation(album, albumArtists.toList());
		}

		return Cover::Location::invalidLocation();
	}
}

struct AlbumInfo::Private
{
	Cover::Location coverLocation;
	AdditionalInfo additionalData;
	QString header;
	QString subheader;
};

AlbumInfo::AlbumInfo(const MetaDataList& metaDataList) :
	LibraryItemInfo(metaDataList),
	m {Pimpl::make<Private>()}
{
	const auto databaseId = metaDataList.isEmpty()
	                        ? DbId(-1)
	                        : metaDataList[0].databaseId();

	m->additionalData = calcAdditionalData(databaseId, albumIds(), albums(), artists());
	m->coverLocation = calcCoverLocation(databaseId, albumIds(), albums(), artists(), albumArtists());
	m->header = calcAlbumString();
	m->subheader = Lang::get(Lang::By).toLower() + " " + calcArtistString();
}

AlbumInfo::~AlbumInfo() = default;

Cover::Location AlbumInfo::coverLocation() const { return m->coverLocation; }

QString AlbumInfo::header() const { return m->header; }

QString AlbumInfo::subheader() const { return m->subheader; }

AdditionalInfo AlbumInfo::additionalData() const { return m->additionalData; }
