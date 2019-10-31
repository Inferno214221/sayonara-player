/* LibraryDatabase.cpp */

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

#include "LibraryDatabase.h"
#include "Database/Query.h"
#include "Database/CoverConnector.h"
#include "Utils/Settings/Settings.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/MetaData/Album.h"
#include "Utils/MetaData/Artist.h"
#include "Utils/Set.h"
#include "Utils/Logger/Logger.h"

using DB::LibraryDatabase;
using DB::Query;

struct LibraryDatabase::Private
{
	QString artistid_field;
	QString artistname_field;
	QString connection_name;
	DbId	db_id;

	LibraryId library_id;

	Private(const QString& connection_name, DbId db_id, LibraryId library_id) :
		connection_name(connection_name),
		db_id(db_id),
		library_id(library_id)
	{
		artistid_field = "artistID";
		artistname_field = "artistName";
	}
};


LibraryDatabase::LibraryDatabase(const QString& connection_name, DbId db_id, LibraryId library_id) :
	DB::Albums(connection_name, db_id, library_id),
	DB::Artists(connection_name, db_id, library_id),
	DB::Tracks(connection_name, db_id, library_id)
{
	m = Pimpl::make<Private>(connection_name, db_id, library_id);

	bool show_album_artists = false;

	AbstrSetting* s = Settings::instance()->setting(SettingKey::Lib_ShowAlbumArtists);
	QString db_key = s->db_key();

	Query q(connection_name, db_id);
	QString querytext = "SELECT value FROM settings WHERE key = '" + db_key + "';";

	q.prepare(querytext);
	if(q.exec())
	{
		if(q.next())
		{
			QVariant var = q.value("value");
			show_album_artists = var.toBool();
		}
	}

	if(show_album_artists){
		change_artistid_field(LibraryDatabase::ArtistIDField::AlbumArtistID);
	}

	else{
		change_artistid_field(LibraryDatabase::ArtistIDField::ArtistID);
	}
}

LibraryDatabase::~LibraryDatabase() {}

void LibraryDatabase::change_artistid_field(LibraryDatabase::ArtistIDField field)
{
	if(field == LibraryDatabase::ArtistIDField::AlbumArtistID)
	{
		m->artistid_field = "albumArtistID";
		m->artistname_field = "albumArtistName";
	}

	else
	{
		m->artistid_field = "artistID";
		m->artistname_field = "artistName";
	}
}

QString LibraryDatabase::artistid_field() const
{
	return m->artistid_field;
}

QString LibraryDatabase::artistname_field() const
{
	return m->artistname_field;
}

void LibraryDatabase::clear()
{
	DB::Tracks::deleteAllTracks(true);
}

LibraryId LibraryDatabase::library_id() const
{
	return m->library_id;
}


bool DB::LibraryDatabase::store_metadata(const MetaDataList& v_md)
{
	if(v_md.isEmpty()) {
		return true;
	}

	sp_log(Log::Develop, this) << " Search for already known albums and artists.";

	// gather all albums in a map
	QHash<QString, Album> album_map;
	{
		AlbumList albums;
		DB::Albums::getAllAlbums(albums, true);

		for(const Album& album : albums){
			album_map[album.name()] = album;
		}
	}

	// gather all artists in a map
	QHash<QString, Artist> artist_map;
	{
		ArtistList artists;
		DB::Artists::getAllArtists(artists, true);

		for(const Artist& artist : artists){
			artist_map[artist.name()] = artist;
		}
	}

	// gather all metadata in a map
	QHash<QString, MetaData> md_map;
	{
		MetaDataList v_md_old;
		DB::Tracks::getAllTracks(v_md_old);
		for(const MetaData& md : v_md_old) {
			md_map[md.filepath()] = md;
		}
	}

	db().transaction();

	for(MetaData md : v_md)
	{
		md.set_library_id(m->library_id);

		{ // check album id
			Album album = album_map[md.album()];
			if(album.id < 0)
			{
				album.id = DB::Albums::insertAlbumIntoDatabase(md.album());
				album_map[md.album()] = album;
			}

			md.set_album_id(album.id);
		}

		{ // check artist id
			Artist artist = artist_map[md.artist()];
			if (artist.id() < 0)
			{
				artist.set_id(DB::Artists::insertArtistIntoDatabase(md.artist()));
				artist_map[md.artist()] = artist;
			}

			md.set_artist_id(artist.id());
		}

		{ // check album artist ...
			Artist album_artist = artist_map[md.album_artist()];
			if(album_artist.id() < 0)
			{
				album_artist.set_id(DB::Artists::insertArtistIntoDatabase(md.album_artist()));
				artist_map[md.album_artist()] = album_artist;
			}

			md.set_album_artist_id(album_artist.id());
		}

		// because all artists and albums should be in the db right now,
		// we should never reach the inner block
		if(md.album_id() < 0 || md.artist_id() < 0 || md.library_id() < 0)
		{
			sp_log(Log::Warning, this) << "Cannot insert artist or album of " << md.filepath();
			continue;
		}

		// check, if the track was known before
		{
			const MetaData& found_md = md_map[md.filepath()];
			if(found_md.id() < 0)
			{
				DB::Tracks::insertTrackIntoDatabase(md, md.artist_id(), md.album_id(), md.album_artist_id());
			}

			else
			{
				md.set_id(found_md.id());
				DB::Tracks::updateTrack(md);
			}
		}
	}

	sp_log(Log::Develop, this) << "Commit " << v_md.size() << " tracks to database";

	return db().commit();
}

QSqlDatabase LibraryDatabase::db() const
{
	DB::Module module(m->connection_name, m->db_id);
	return module.db();
}

DbId LibraryDatabase::db_id() const
{
	return m->db_id;
}
