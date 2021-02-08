/* ArtistInfo.cpp */

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

#include "ArtistInfo.h"

#include "Database/Connector.h"
#include "Database/LibraryDatabase.h"

#include "Utils/Set.h"
#include "Utils/Algorithm.h"
#include "Utils/Language/Language.h"
#include "Utils/SimilarArtists/SimilarArtists.h"
#include "Utils/MetaData/Artist.h"
#include "Utils/MetaData/MetaData.h"
#include "Utils/MetaData/MetaDataList.h"

#include <QFile>

namespace Algorithm=Util::Algorithm;

struct ArtistInfo::Private
{
	Cover::Location coverLocation;
	DbId databaseId;

	Private(DbId databaseId) :
		databaseId(databaseId)
	{}
};

ArtistInfo::ArtistInfo(const MetaDataList& tracks) :
	MetaDataInfo(tracks)
{
	DbId databaseId = DbId(-1);
	if(tracks.size() > 0){
		databaseId = tracks.first().databaseId();
	}

	m = Pimpl::make<Private>(databaseId);

	insertNumericInfoField(InfoStrings::nAlbums, albums().count());

	mAdditionalInfo.clear();

	if(artistIds().size() == 1)
	{
		Artist artist;
		bool success;

		ArtistId artistId = artistIds().first();

		DB::LibraryDatabase* lib_db = DB::Connector::instance()->libraryDatabase(-1, m->databaseId);

		success = lib_db->getArtistByID(artistId, artist);

		if(success)
		{
			mAdditionalInfo.clear();
			calcSimilarArtists(artist);
			// custom fields
			const CustomFieldList custom_fields = artist.customFields();
			if(!custom_fields.empty()){
				mAdditionalInfo << StringPair(Lang::get(Lang::SimilarArtists), QString());
			}

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

	else if(artists().size() > 1){
		insertNumericInfoField(InfoStrings::nArtists, artists().count());
	}

	calcHeader();
	calcSubheader();
	calcCoverLocation();
}

ArtistInfo::~ArtistInfo() {}

void ArtistInfo::calcHeader()
{
	mHeader = calcArtistString();
}


void ArtistInfo::calcSimilarArtists(Artist& artist)
{
	using SimPair=QPair<double, QString>;

	QList<SimPair> sim_list;
	QMap<QString, double> sim_artists = SimilarArtists::getSimilarArtists(artist.name());

	for(auto it=sim_artists.cbegin(); it != sim_artists.cend(); it++)
	{
		sim_list << SimPair(it.value(), it.key());
	}

	Algorithm::sort(sim_list, [](const SimPair& p1, const SimPair& p2){
		return (p2.first < p1.first);
	});

	for(const SimPair& p : sim_list)
	{
		artist.addCustomField(p.second, p.second, QString("%1%").arg((int) (p.first * 100)));
	}
}


void ArtistInfo::calcSubheader()
{
	mSubheader = "";
}


void ArtistInfo::calcCoverLocation()
{
	if( artists().size() == 1)
	{
		QString artist = artists().first();
		m->coverLocation = Cover::Location::coverLocation(artist);
	}

	else if(albumArtists().size() == 1)
	{
		QString artist = albumArtists().first();
		m->coverLocation = Cover::Location::coverLocation(artist);
	}

	else
	{
		m->coverLocation = Cover::Location::invalidLocation();
	}
}

Cover::Location ArtistInfo::coverLocation() const
{
	return m->coverLocation;
}
