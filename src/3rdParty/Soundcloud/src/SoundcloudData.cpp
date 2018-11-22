/* SoundcloudData.cpp */

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

#include "SoundcloudData.h"
#include "SoundcloudWebAccess.h"
#include "SearchInformation.h"

#include "Utils/MetaData/Album.h"
#include "Utils/MetaData/Artist.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Logger/Logger.h"
#include "Utils/globals.h"

#include "Database/Query.h"
#include "Database/Connector.h"
#include "Database/LibraryDatabase.h"

#include <QList>

using DB::Query;

SC::Database::Database() :
	::DB::Base(25, "soundcloud.db"),
	::DB::LibraryDatabase(module()->connection_name(), 25, -1)
{
	this->apply_fixes();
}

SC::Database::~Database()
{
	this->close_db();
}


QString SC::Database::fetch_query_artists(bool also_empty) const
{
	QString sql =
			"SELECT "
			"artists.artistid AS artistID, "
			"artists.name AS artistName, "
			"artists.permalink_url AS permalink_url, "
			"artists.description AS description, "
			"artists.followers_following AS followers_following, "
			"artists.cover_url AS cover_url, "
			"artists.name AS albumArtistName, "
			"COUNT(DISTINCT tracks.trackid) AS trackCount, "
			"GROUP_CONCAT(DISTINCT albums.albumid) AS artistAlbums "
			"FROM artists ";

	QString join = "INNER JOIN";
	if(also_empty){
		join = "LEFT OUTER JOIN";
	}

	sql +=	join + " tracks ON artists.artistID = tracks.artistID " +
			join + " albums ON albums.albumID = tracks.albumID ";

	return sql;
}

QString SC::Database::fetch_query_albums(bool also_empty) const
{
	QString sql =
			"SELECT "
			"albums.albumID AS albumID, "
			"albums.name AS albumName, "
			"SUM(tracks.length) / 1000 AS albumLength, "
			"albums.rating AS albumRating, "
			"albums.permalink_url AS permalink_url, "
			"albums.purchase_url AS purchase_url, "
			"albums.cover_url AS cover_url, "
			"COUNT(DISTINCT tracks.trackid) AS trackCount, "
			"MAX(tracks.year) AS albumYear, "
			"GROUP_CONCAT(DISTINCT artists.name) AS albumArtists, "
			"GROUP_CONCAT(DISTINCT tracks.discnumber) AS discnumbers "
			"FROM albums ";

	QString join = "INNER JOIN";
	if(also_empty){
		join = "LEFT OUTER JOIN";
	}

	sql +=	join + " tracks ON albums.albumID = tracks.albumID " +
			join + " artists ON artists.artistID = tracks.artistID ";

	return sql;
}

QString SC::Database::fetch_query_tracks() const
{
	return	"SELECT "
			"tracks.trackID AS trackID, "
			"tracks.title AS trackTitle, "
			"tracks.length AS trackLength, "
			"tracks.year AS trackYear, "
			"tracks.bitrate AS trackBitrate, "
			"tracks.filename AS trackFilename, "
			"tracks.track AS trackNum, "
			"albums.albumID AS albumID, "
			"artists.artistID AS artistID, "
			"albums.name AS albumName, "
			"artists.name AS artistName, "
			"tracks.genre AS genrename, "
			"tracks.filesize AS filesize, "
			"tracks.discnumber AS discnumber, "
			"tracks.purchase_url AS purchase_url, "
			"tracks.cover_url AS cover_url, "
			"tracks.rating AS rating "
			"FROM tracks "
			"INNER JOIN albums ON tracks.albumID = albums.albumID "
			"INNER JOIN artists ON tracks.artistID = artists.artistID ";
}


QString SC::Database::load_setting(const QString& key)
{
	Query q(module());
	q.prepare("SELECT value FROM Settings WHERE key=:key;");
	q.bindValue(":key", key);
	if(!q.exec()){
		q.show_error("Cannot load setting " + key);
		return QString();
	}

	if(q.next()){
		return q.value(0).toString();
	}

	return QString();
}

bool SC::Database::save_setting(const QString& key, const QString& value)
{
	Query q(module());

	QString v = load_setting(key);
	if(v.isNull()){
		return insert_setting(key, value);
	}

	q.prepare("UPDATE Settings SET value=:value WHERE key=:key;");
	q.bindValue(":key", key);
	q.bindValue(":value", value);

	bool success = q.exec();

	if(!success) {
		q.show_error("Cannot apply setting " + key);
	}

	return success;
}

bool SC::Database::insert_setting(const QString& key, const QString& value)
{
	Query q(module());

	q.prepare("INSERT INTO settings (key, value) VALUES (:key, :value);");
	q.bindValue(":key", key);
	q.bindValue(":value", value);

	bool success = q.exec();

	if(!success) {
		q.show_error("Cannot insert setting " + key);
	}

	return success;
}

bool SC::Database::getSearchInformation(SC::SearchInformationList& search_information)
{
	Query q(module());

	q.prepare("SELECT artistId, albumId, trackId, allCissearch "
			  "FROM track_search_view;");

	bool success = q.exec();

	if(!success){
		q.show_error("Cannot get search Information");
		return false;
	}


	while(q.next())
	{
		SearchInformation info(
					q.value(0).toInt(),
					q.value(1).toInt(),
					q.value(2).toInt(),
					q.value(3).toString()
		);

		search_information << info;
	}

	return true;
}


bool SC::Database::db_fetch_tracks(Query& q, MetaDataList& result)
{
	result.clear();

	if (!q.exec()) {
		q.show_error("Cannot fetch tracks from database");
		return false;
	}

	if(!q.last()){
		return true;
	}

	for(bool is_element = q.first(); is_element; is_element = q.next())
	{
		MetaData data;

		data.id = 		 q.value(0).toInt();
		data.set_title(q.value(1).toString());
		data.length_ms = q.value(2).toInt();
		data.year = 	 q.value(3).toInt();
		data.bitrate = 	 q.value(4).toInt();
		data.set_filepath(q.value(5).toString());
		data.track_num = q.value(6).toInt();
		data.album_id =  q.value(7).toInt();
		data.artist_id = q.value(8).toInt();
		data.set_album(q.value(9).toString().trimmed());
		data.set_artist(q.value(10).toString().trimmed());
		data.set_genres(q.value(11).toString().split(","));
		data.filesize =  q.value(12).toInt();
		data.discnumber = q.value(13).toInt();
		data.add_custom_field("purchase_url", tr("Purchase Url"), q.value(14).toString());
		data.set_cover_download_url(q.value(15).toString());
		data.rating = q.value(16).toInt();
		data.set_db_id(module()->db_id());

		result << data;
	}

	return true;
}

bool SC::Database::db_fetch_albums(Query& q, AlbumList& result)
{
	result.clear();

	if (!q.exec()) {
		q.show_error("Could not get all albums from database");
		return false;
	}

	while(q.next())
	{
		Album album;

		album.id =					q.value(0).toInt();
		album.set_name(q.value(1).toString().trimmed());
		album.length_sec =			q.value(2).toInt();
		album.rating =				q.value(3).toInt();
		album.add_custom_field("permalink_url", "Permalink Url", q.value(4).toString());
		album.add_custom_field("purchase_url", "Purchase Url", q.value(5).toString());
		album.set_cover_download_url(q.value(6).toString());
		album.num_songs =			q.value(7).toInt();
		album.year =				q.value(8).toInt();

		QStringList lst_artists =	q.value(9).toString().split(',');
		album.set_artists(lst_artists);

		QStringList lst_discnumbers = q.value(10).toString().split(',');
		album.discnumbers.clear();

		for(const QString& disc : lst_discnumbers) {
			int d = disc.toInt();
			if(album.discnumbers.contains(d)) continue;

			album.discnumbers << d;
		}

		if(album.discnumbers.isEmpty()) {
			album.discnumbers << 1;
		}

		album.n_discs = album.discnumbers.size();
		album.is_sampler = (lst_artists.size() > 1);
		album.set_db_id(module()->db_id());

		result << album;
	};

	return true;
}

bool SC::Database::db_fetch_artists(Query& q, ArtistList& result)
{
	result.clear();

	if (!q.exec()) {
		q.show_error("Could not get all artists from database");
		return false;
	}

	if(!q.last()){
		return true;
	}

	for(bool is_element=q.first(); is_element; is_element = q.next()){
		Artist artist;

		artist.id =						q.value(0).toInt();
		artist.set_name(q.value(1).toString().trimmed());

		artist.add_custom_field("permalink_url", "Permalink Url", q.value(2).toString());
		artist.add_custom_field("description", "Description", q.value(3).toString());
		artist.add_custom_field("followers_following", "Followers/Following", q.value(4).toString());

		artist.set_cover_download_url(q.value(5).toString());
		artist.num_songs =				q.value(7).toInt();
		QStringList list =				q.value(8).toString().split(',');
		artist.num_albums =				list.size();
		artist.set_db_id(module()->db_id());

		result << artist;
	}

	return true;
}

ArtistId SC::Database::updateArtist(const Artist& artist)
{
	Query q(module());

	QString query_text = QString("UPDATE artists SET ") +
			"name = :name, "
			"cissearch = :cissearch, "
			"permalink_url = :permalink_url, "
			"description = :description, "
			"followers_following = :followers_following, "
			"cover_url = :cover_url "
			"WHERE artistid = :sc_id;";

	q.prepare(query_text);
	q.bindValue(":sc_id", artist.id);
	q.bindValue(":name", artist.name());
	q.bindValue(":cissearch", artist.name().toLower());
	q.bindValue(":permalink_url", artist.get_custom_field("permalink_url"));
	q.bindValue(":description", artist.get_custom_field("description"));
	q.bindValue(":followers_following", artist.get_custom_field("followers_following"));
	q.bindValue(":cover_url", artist.cover_download_url());

	if (!q.exec()) {
		q.show_error(QString("Soundcloud: Cannot update artist ") + artist.name());
		return -1;
	}

	return getArtistID(artist.name());
}

ArtistId SC::Database::insertArtistIntoDatabase (const QString& artist)
{
	Q_UNUSED(artist)
	return -1;
}

ArtistId SC::Database::insertArtistIntoDatabase (const Artist& artist)
{
	Query q(module());

	Artist tmp_artist;
	if(getArtistByID(artist.id, tmp_artist)){
		if(tmp_artist.id > 0){
			return updateArtist(artist);
		}
	}

	QString query_text =
			"INSERT INTO artists "
			"(artistid, name, cissearch, permalink_url, description, followers_following, cover_url) "
			"VALUES "
			"(:sc_id, :name, :cissearch, :permalink_url, :description, :followers_following, :cover_url); ";

	q.prepare(query_text);
	q.bindValue(":sc_id", artist.id);
	q.bindValue(":name", artist.name());
	q.bindValue(":cissearch", artist.name().toLower());
	q.bindValue(":permalink_url", artist.get_custom_field("permalink_url"));
	q.bindValue(":description", artist.get_custom_field("description"));
	q.bindValue(":followers_following", artist.get_custom_field("followers_following"));
	q.bindValue(":cover_url", artist.cover_download_url());

	if (!q.exec()) {
		q.show_error(QString("Soundcloud: Cannot insert artist ") + artist.name());
		return -1;
	}

	return getArtistID(artist.name());
}


AlbumId SC::Database::updateAlbum(const Album& album)
{
	Query q(module());

	QString query_text = QString("UPDATE albums SET ") +
			"name = :name, "
			"cissearch = :cissearch, "
			"permalink_url = :permalink_url, "
			"purchase_url = :purchase_url, "
			"cover_url = :cover_url "
			"WHERE albumID = :sc_id;";

	q.prepare(query_text);

	q.bindValue(":sc_id", album.id);
	q.bindValue(":name", album.name());
	q.bindValue(":cissearch", album.name().toLower());
	q.bindValue(":permalink_url", album.get_custom_field("permalink_url"));
	q.bindValue(":purchase_url", album.get_custom_field("purchase_url"));
	q.bindValue(":cover_url", album.cover_download_url());

	if (!q.exec()) {
		q.show_error(QString("Soundcloud: Cannot insert album ") + album.name());
		return -1;
	}

	return getAlbumID(album.name());
}


AlbumId SC::Database::insertAlbumIntoDatabase (const QString& album)
{
	Q_UNUSED(album)
	return -1;
}

AlbumId SC::Database::insertAlbumIntoDatabase (const Album& album)
{
	Query q(module());

	Album tmp_album;
	if(getAlbumByID(album.id, tmp_album) && tmp_album.id > 0){
		return updateAlbum(album);
	}

	QString query_text =
			"INSERT INTO albums "
			"(albumid, name, cissearch, permalink_url, purchase_url, cover_url) "
			"VALUES "
			"(:sc_id, :name, :cissearch, :permalink_url, :purchase_url, :cover_url); ";

	q.prepare(query_text);

	q.bindValue(":sc_id", album.id);
	q.bindValue(":name", album.name());
	q.bindValue(":cissearch", album.name().toLower());
	q.bindValue(":permalink_url", album.get_custom_field("permalink_url"));
	q.bindValue(":purchase_url", album.get_custom_field("purchase_url"));
	q.bindValue(":cover_url", album.cover_download_url());

	if (!q.exec()) {
		q.show_error(QString("Soundcloud: Cannot insert album ") + album.name());
		return -1;
	}

	return getAlbumID(album.name());
}

bool SC::Database::updateTrack(const MetaData& md)
{
	Query q(module());

	sp_log(Log::Info, this) << "insert new track: " << md.filepath();

	QString querytext =
				"UPDATE tracks SET "
				"filename = :filename, "
				"albumID = :albumID, "
				"artistID = :artistID, "
				"length = :length, "
				"year = :year, "
				"title = :title, "
				"track = :track, "
				"bitrate = :bitrate, "
				"genre = :genre, "
				"filesize = :filesize, "
				"discnumber = :discnumber, "
				"cissearch = :cissearch, "
				"purchase_url = :purchase_url, "
				"cover_url = :cover_url "
				"WHERE trackID = :sc_id;";

	q.prepare(querytext);

	q.bindValue(":sc_id",		md.id);
	q.bindValue(":filename",	md.filepath());
	q.bindValue(":albumID",		md.album_id);
	q.bindValue(":artistID",	md.artist_id);
	q.bindValue(":length",		QVariant::fromValue(md.length_ms));
	q.bindValue(":year",		md.year);
	q.bindValue(":title",		md.title());
	q.bindValue(":track",		md.track_num);
	q.bindValue(":bitrate",		md.bitrate);
	q.bindValue(":genre",		md.genres_to_list().join(","));
	q.bindValue(":filesize",	QVariant::fromValue(md.filesize));
	q.bindValue(":discnumber",	md.discnumber);
	q.bindValue(":cissearch",	md.title().toLower());
	q.bindValue(":purchase_url", md.get_custom_field("purchase_url"));
	q.bindValue(":cover_url",	md.cover_download_url());

	if (!q.exec()) {
		q.show_error(QString("Cannot insert track into database ") + md.filepath());
		return false;
	}

	return true;
}

bool SC::Database::insertTrackIntoDatabase(const MetaData& md, int artist_id, int album_id, int album_artist_id)
{
	Q_UNUSED(album_artist_id)
	return insertTrackIntoDatabase(md, artist_id, album_id);
}

bool SC::Database::insertTrackIntoDatabase(const MetaData &md, int artist_id, int album_id)
{
	Query q(module());

	int new_id = getTrackById(md.id).id;
	if(new_id > 0){
		return updateTrack(md);
	}

	sp_log(Log::Info, this) << "insert new track: " << md.filepath();

	QString querytext =
			"INSERT INTO tracks "
			"(trackid,filename,albumID,artistID,title,year,length,track,bitrate,genre,filesize,discnumber,cissearch,purchase_url,cover_url) "
			"VALUES "
			"(:sc_id,:filename,:albumID,:artistID,:title,:year,:length,:track,:bitrate,:genre,:filesize,:discnumber,:cissearch,:purchase_url,:cover_url); ";

	q.prepare(querytext);

	q.bindValue(":sc_id",		md.id);
	q.bindValue(":filename",	md.filepath());
	q.bindValue(":albumID",		album_id);
	q.bindValue(":artistID",	artist_id);
	q.bindValue(":length",		QVariant::fromValue(md.length_ms));
	q.bindValue(":year",		md.year);
	q.bindValue(":title",		md.title());
	q.bindValue(":track",		md.track_num);
	q.bindValue(":bitrate",		md.bitrate);
	q.bindValue(":genre",		md.genres_to_list().join(","));
	q.bindValue(":filesize",	QVariant::fromValue(md.filesize));
	q.bindValue(":discnumber",	md.discnumber);
	q.bindValue(":cissearch",	md.title().toLower());
	q.bindValue(":purchase_url", md.get_custom_field("purchase_url"));
	q.bindValue(":cover_url",	md.cover_download_url());

	if (!q.exec()) {
		q.show_error(QString("Cannot insert track into database ") + md.filepath());
		return false;
	}

	return true;
}

bool SC::Database::store_metadata(const MetaDataList& v_md)
{
	if(v_md.isEmpty()) {
		return true;
	}

	module()->db().transaction();

	for(const MetaData& md : v_md) {
		sp_log(Log::Debug, this) << "Looking for " << md.artist() << " and " << md.album();
		if(md.album_id == -1 || md.artist_id == -1){
			sp_log(Log::Warning, this) << "AlbumID = " << md.album_id << " - ArtistID = " << md.artist_id;
			continue;
		}

		insertTrackIntoDatabase (md, md.artist_id, md.album_id);
	}

	return module()->db().commit();
}


bool SC::Database::apply_fixes()
{
	QString creation_string = "CREATE TABLE Settings ("
						   " key VARCHAR(100) PRIMARY KEY,"
						   " value TEXT "
						   ");";

	bool success = check_and_create_table("Settings", creation_string);
	if(!success){
		sp_log(Log::Error, this) << "Cannot create settings table for soundcloud";
		return false;
	}

	int version;
	QString version_string = load_setting("version");
	if(version_string.isEmpty()){
		save_setting("version", "1");
		version = 1;
	}

	else{
		version = version_string.toInt();
	}

	if(version < 2){
		bool success = check_and_insert_column("tracks", "albumArtistID", "integer", "-1");
		if(success){
			save_setting("version", "2");
		}
	}

	if(version < 3) {
		bool success = check_and_insert_column("tracks", "libraryID", "integer", "0");
		if(success){
			save_setting("version", "3");
		}
	}

	return true;
}


const DB::Module* SC::Database::module() const
{
	return static_cast<const DB::Base*>(this);
}
