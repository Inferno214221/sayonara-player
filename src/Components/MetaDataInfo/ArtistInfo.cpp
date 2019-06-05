/* ArtistInfo.cpp */

/* Copyright (C) 2011-2019  Lucio Carreras
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
#include "Utils/globals.h"
#include "Utils/Language/Language.h"
#include "Utils/SimilarArtists/SimilarArtists.h"
#include "Utils/MetaData/Artist.h"
#include "Utils/MetaData/MetaData.h"
#include "Utils/MetaData/MetaDataList.h"

#include <QFile>

namespace Algorithm=Util::Algorithm;

struct ArtistInfo::Private
{
	Cover::Location cover_location;

	DbId db_id;

	Private(DbId db_id) :
		db_id(db_id)
	{}
};

ArtistInfo::ArtistInfo(const MetaDataList& v_md) :
	MetaDataInfo(v_md)
{
	DbId db_id = (DbId) -1;
	if(v_md.size() > 0){
		db_id = v_md.first().db_id();
	}

	m = Pimpl::make<Private>(db_id);

	insert_numeric_info_field(InfoStrings::nAlbums, albums().count());

	_additional_info.clear();

	if(artist_ids().size() == 1)
	{
		Artist artist;
		bool success;

		ArtistId artist_id = artist_ids().first();

		DB::Connector* db = DB::Connector::instance();
		DB::LibraryDatabase* lib_db = db->library_db(-1, m->db_id);

		success = lib_db->getArtistByID(artist_id, artist);

		if(success)
		{
			_additional_info.clear();
			calc_similar_artists(artist);
			// custom fields
			const CustomFieldList custom_fields = artist.get_custom_fields();
			if(!custom_fields.empty()){
				_additional_info << StringPair(Lang::get(Lang::SimilarArtists), QString());
			}

			for(const CustomField& field : custom_fields)
			{
				QString name = field.get_display_name();
				QString value = field.get_value();
				if(value.isEmpty()){
					continue;
				}

				_additional_info << StringPair(name, field.get_value());
			}
		}
	}

	else if(artists().size() > 1){
		insert_numeric_info_field(InfoStrings::nArtists, artists().count());
	}

	calc_header();
	calc_subheader();
	calc_cover_location();
}

ArtistInfo::~ArtistInfo() {}

void ArtistInfo::calc_header()
{
	_header = calc_artist_str();
}


void ArtistInfo::calc_similar_artists(Artist& artist)
{
	using SimPair=QPair<double, QString>;

	QList<SimPair> sim_list;
	QMap<QString, double> sim_artists = SimilarArtists::get_similar_artists(artist.name());

	for(auto it=sim_artists.cbegin(); it != sim_artists.cend(); it++)
	{
		sim_list << SimPair(it.value(), it.key());
	}

	Algorithm::sort(sim_list, [](const SimPair& p1, const SimPair& p2){
		return (p2.first < p1.first);
	});

	for(const SimPair& p : sim_list)
	{
		artist.add_custom_field(p.second, p.second, QString("%1%").arg((int) (p.first * 100)));
	}
}


void ArtistInfo::calc_subheader()
{
	_subheader = "";
}


void ArtistInfo::calc_cover_location()
{
	if( artists().size() == 1)
	{
		QString artist = artists().first();
		m->cover_location = Cover::Location::cover_location(artist);
	}

	else if(album_artists().size() == 1)
	{
		QString artist = album_artists().first();
		m->cover_location = Cover::Location::cover_location(artist);
	}

	else
	{
		m->cover_location = Cover::Location::invalid_location();
	}
}

// todo: delete me
QString ArtistInfo::additional_infostring() const
{
	return QString();
}

Cover::Location ArtistInfo::cover_location() const
{
	return m->cover_location;
}
