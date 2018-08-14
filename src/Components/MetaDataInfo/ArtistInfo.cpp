/* ArtistInfo.cpp */

/* Copyright (C) 2011-2017  Lucio Carreras
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
#include "Utils/Utils.h"
#include "Utils/globals.h"
#include "Utils/FileUtils.h"
#include "Utils/Language.h"
#include "Utils/SimilarArtists/SimilarArtists.h"
#include "Utils/MetaData/Artist.h"
#include "Utils/MetaData/MetaData.h"
#include "Utils/MetaData/MetaDataList.h"


#include <QFile>

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
			for(const CustomField& field : custom_fields)
			{
				QString name = field.get_display_name();
				QString value = field.get_value();
				if(value.isEmpty()){
					continue;
				}

				_additional_info[name] += field.get_value();
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
	QMap<QString, double> sim_artists = SimilarArtists::get_similar_artists(artist.name());

	for(auto it=sim_artists.cbegin(); it != sim_artists.cend(); it++)
	{
		const QString& artist_name = it.key();
		artist.add_custom_field("sim_artist_" + artist_name,
								"sim_artist_" + QString::number(it.value()) + it.key(),
								artist_name);
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


QString ArtistInfo::additional_infostring() const
{
	QString str;
	QStringList sim_artists;

	for(auto it=_additional_info.cbegin(); it != _additional_info.cend(); it++)
	{
		if(it.key().startsWith("sim_artist_")){
			sim_artists << it.key();
		}

		else {
			str += BOLD(it.key()) + ": " + it.value() + CAR_RET;
		}
	}

	Util::sort(sim_artists, [](const QString& artist1, const QString artist2){
		return (artist1 > artist2);
	});

	if(!sim_artists.isEmpty()){
		str = BOLD(Lang::get(Lang::SimilarArtists) + ":") + CAR_RET + CAR_RET;
	}

	int i=0;
	QStringList artist_list;
	for(const QString& sim_artist : ::Util::AsConst(sim_artists))
	{
		if(i++ > 50){
			break;
		}

		QString artist_name = _additional_info[sim_artist];
		DB::Connector* db = DB::Connector::instance();
		DB::LibraryDatabase* lib_db = db->library_db(-1, m->db_id);

		ArtistId id = lib_db->getArtistID(artist_name);

		if( id >= 0 ){
			artist_list << BOLD(artist_name);
		}

		else {
			artist_list << artist_name;
		}

	}

	str += artist_list.join(", ");

	return str;
}

Cover::Location ArtistInfo::cover_location() const
{
	return m->cover_location;
}
