/* LibraryDatabase.cpp */

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

#include "LibraryDatabase.h"
#include "Database/Query.h"
#include "Utils/Settings/Settings.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/MetaData/Album.h"
#include "Utils/MetaData/Artist.h"

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
	DB::Tracks::deleteAllTracks();
}

LibraryId LibraryDatabase::library_id() const
{
	return m->library_id;
}


bool DB::LibraryDatabase::store_metadata(const MetaDataList& v_md)
{
	bool success = true;

	if(v_md.isEmpty()) {
		return success;
	}

	db().transaction();

	AlbumList albums;
	ArtistList artists;
	QHash<QString, Album> album_map;
	QHash<QString, Artist> artist_map;

	sp_log(Log::Develop, this) << " Search for already known albums and artists.";
	DB::Albums::getAllAlbums(albums, true);
	DB::Artists::getAllArtists(artists, true);

	sp_log(Log::Develop, this) << "  Found " << albums.size() << " albums and " << artists.size() << " artists";

	for(const Album& album : albums){
		album_map[album.name()] = album;
	}

	for(const Artist& artist : artists){
		artist_map[artist.name()] = artist;
	}

	albums.clear();
	artists.clear();

	for(MetaData md : v_md)
	{
		ArtistId artist_id, album_artist_id;
		AlbumId album_id;
		//first check if we know the artist and its id
		Album album = album_map[md.album()];
		if(album.id < 0) {
			album_id = DB::Albums::insertAlbumIntoDatabase(md.album());
			album.id = album_id;
			album_map[md.album()] = album;
		}

		else{
			album_id = album.id;
		}

		Artist artist = artist_map[md.artist()];
		if (artist.id < 0) {
			artist_id = DB::Artists::insertArtistIntoDatabase(md.artist());
			artist.id = artist_id;
			artist_map[md.artist()] = artist;
		}

		else{
			artist_id = artist.id;
		}

		if(md.album_artist_id() == -1){
			md.set_album_artist_id(artist_id);
		}

		if(md.album_artist().isEmpty()){
			md.set_album_artist(md.artist());
		}

		Artist album_artist = artist_map[md.album_artist()];
		if (album_artist.id < 0)
		{
			if(md.album_artist().isEmpty()){
				album_artist_id = -1;
			}

			else{
				album_artist_id = DB::Artists::insertArtistIntoDatabase(md.album_artist());
				album_artist.id = album_artist_id;
				artist_map[md.album_artist()] = album_artist;
			}
		}

		else{
			album_artist_id = album_artist.id;
		}

		md.album_id = album_id;
		md.artist_id = artist_id;
		md.library_id = m->library_id;

		if(album_id == -1 || artist_id == -1 || md.library_id == -1){
			sp_log(Log::Warning) << "Cannot insert artist or album of " << md.filepath();
			continue;
		}

		DB::Tracks::insertTrackIntoDatabase(md, artist_id, album_id, album_artist_id);
	}

	sp_log(Log::Develop, this) << "Commit " << v_md.size() << " tracks to database";
	success = db().commit();

	return success;
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
