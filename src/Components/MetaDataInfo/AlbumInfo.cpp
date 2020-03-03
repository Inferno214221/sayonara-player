/* AlbumInfo.cpp */

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

#include "AlbumInfo.h"
#include "MetaDataInfo.h"

#include "Database/LibraryDatabase.h"
#include "Database/Connector.h"

#include "Utils/Set.h"
#include "Utils/Language/Language.h"
#include "Utils/MetaData/Album.h"
#include "Utils/MetaData/MetaData.h"
#include "Utils/MetaData/MetaDataList.h"

#include <QStringList>

struct AlbumInfo::Private
{
	DbId databaseId;
	Cover::Location coverLocation;

	Private(DbId databaseId) :
		databaseId(databaseId)
	{}
};

AlbumInfo::AlbumInfo(const MetaDataList& tracks) :
	MetaDataInfo(tracks)
{
	DbId databaseId = DbId(-1);
	if(tracks.size() > 0)
	{
		databaseId = tracks.first().databaseId();
	}

	m = Pimpl::make<Private>(databaseId);
	QString str_sampler;

	// clear, because it's from Metadata. We are not interested in
	// rather fetch albums' additional data, if there's only one album
	mAdditionalInfo.clear();

	if(albums().size() > 1)
	{
		insertNumericInfoField(InfoStrings::nAlbums, albums().count());
	}

	if(artists().size() > 1)
	{
		insertNumericInfoField(InfoStrings::nArtists, artists().count());
	}

	if(albums().size() == 1)
	{
		Album album;
		bool success;

		if(artists().size() > 1)
		{
			str_sampler = Lang::get(Lang::Yes).toLower();
			mInfo.insert(InfoStrings::Sampler, str_sampler);
		}

		if(artists().size() == 1)
		{
			str_sampler = Lang::get(Lang::No).toLower();
			mInfo.insert(InfoStrings::Sampler, str_sampler);
		}

		AlbumId albumId = albumIds().first();

		DB::LibraryDatabase* lib_db = DB::Connector::instance()->libraryDatabase(-1, m->databaseId);
		if(lib_db)
		{
			// BIG TODO FOR SOUNDCLOUD
			success = lib_db->getAlbumByID(albumId, album);
		}

		else {
			success = false;
		}

		if(success)
		{
			mAdditionalInfo.clear();
			// custom fields
			const CustomFieldList& custom_fields = album.customFields();
			for(const CustomField& field : custom_fields)
			{
				QString name = field.displayName();
				QString value = field.value();
				if(value.isEmpty()){
					continue;
				}

				mAdditionalInfo << StringPair(name, field.value());
			}
		}
	}

	calcHeader();
	calcSubheader();
	calcCoverLocation();
}

AlbumInfo::~AlbumInfo() {}


void AlbumInfo::calcHeader()
{
	mHeader = calcAlbumString();
}

void AlbumInfo::calcSubheader()
{
	mSubheader = Lang::get(Lang::By).toLower() + " " + calcArtistString();
}

void AlbumInfo::calcCoverLocation()
{
	if(albumIds().size() == 1)
	{
		DB::LibraryDatabase* libraryDatabase = DB::Connector::instance()->libraryDatabase(-1, m->databaseId);

		Album album;
		bool success = libraryDatabase->getAlbumByID(albumIds().first(), album, true);
		if(!success)
		{
			album.setId(albumIds().first());
			album.setName(albums().first());
			album.setArtists(artists().toList());

			if(albumArtists().size() > 0){
				album.setAlbumArtist(albumArtists().first());
			}

			album.setDatabaseId(libraryDatabase->databaseId());
		}

		m->coverLocation = Cover::Location::xcoverLocation(album);
	}

	else if( albums().size() == 1)
	{
		QString album = albums().first();

		if(!albumArtists().isEmpty())
		{
			m->coverLocation = Cover::Location::coverLocation(album, albumArtists().toList());
		}

		else
		{
			m->coverLocation = Cover::Location::coverLocation(album, artists().toList());
		}
	}

	else
	{
		m->coverLocation = Cover::Location::invalidLocation();
	}
}

Cover::Location AlbumInfo::coverLocation() const
{
	return m->coverLocation;
}

