/* AlbumInfo.cpp */

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

#include "AlbumInfo.h"
#include "MetaDataInfo.h"

#include "Database/LibraryDatabase.h"
#include "Database/Connector.h"

#include "Utils/Set.h"
#include "Utils/Language.h"
#include "Utils/MetaData/Album.h"
#include "Utils/MetaData/MetaData.h"
#include "Utils/MetaData/MetaDataList.h"

#include <QStringList>

struct AlbumInfo::Private
{
	DbId db_id;
	Cover::Location cover_location;

	Private(DbId db_id) :
		db_id(db_id)
	{}
};

AlbumInfo::AlbumInfo(const MetaDataList& v_md) :
	MetaDataInfo(v_md)
{
	DbId db_id = (DbId) -1;
	if(v_md.size() > 0)
	{
		db_id = v_md.first().db_id();
	}

	m = Pimpl::make<Private>(db_id);
	QString str_sampler;

	// clear, because it's from Metadata. We are not interested in
	// rather fetch albums' additional data, if there's only one album
	_additional_info.clear();

	if(albums().size() > 1)
	{
		insert_numeric_info_field(InfoStrings::nAlbums, albums().size());
	}

	if(artists().size() > 1)
	{
		insert_numeric_info_field(InfoStrings::nArtists, artists().size());
	}

	if(albums().size() == 1)
	{
		Album album;
		bool success;

		if(artists().size() > 1)
		{
			str_sampler = Lang::get(Lang::Yes).toLower();
			_info.insert(InfoStrings::Sampler, str_sampler);
		}

		if(artists().size() == 1)
		{
			str_sampler = Lang::get(Lang::No).toLower();
			_info.insert(InfoStrings::Sampler, str_sampler);
		}

		AlbumId album_id = album_ids().first();

		DB::Connector* db = DB::Connector::instance();
		DB::LibraryDatabase* lib_db = db->library_db(-1, m->db_id);
		if(lib_db)
		{
			// BIG TODO FOR SOUNDCLOUD
			success = lib_db->getAlbumByID(album_id, album);
		}

		else {
			success = false;
		}

		if(success)
		{
			_additional_info.clear();
			// custom fields
			const CustomFieldList& custom_fields = album.get_custom_fields();
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

	calc_header();
	calc_subheader();
	calc_cover_location();
}

AlbumInfo::~AlbumInfo() {}


void AlbumInfo::calc_header()
{
	_header = calc_album_str();
}

void AlbumInfo::calc_subheader()
{
	_subheader = Lang::get(Lang::By).toLower() + " " + calc_artist_str();
}

void AlbumInfo::calc_cover_location()
{
	if(album_ids().size() == 1)
	{
		DB::Connector* db = DB::Connector::instance();
		DB::LibraryDatabase* lib_db = db->library_db(-1, m->db_id);

		Album album;
		bool success = lib_db->getAlbumByID(album_ids().first(), album, true);
		if(!success) {
			album.id = album_ids().first();
			album.set_name(albums().first());
			album.set_artists(artists().toList());
			album.set_album_artists(album_artists().toList());
			album.set_db_id(lib_db->db_id());
		}

		m->cover_location = Cover::Location::cover_location(album);
	}

	else if( albums().size() == 1)
	{
		QString album = albums().first();

		if(!album_artists().isEmpty())
		{
			m->cover_location = Cover::Location::cover_location(album, album_artists().toList());
		}

		else
		{
			m->cover_location = Cover::Location::cover_location(album, artists().toList());
		}
	}

	else
	{
		m->cover_location = Cover::Location::invalid_location();
	}
}

Cover::Location AlbumInfo::cover_location() const
{
	return m->cover_location;
}

