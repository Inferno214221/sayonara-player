/* DatabaseArtists.cpp */

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

#include "Database/Query.h"
#include "Database/Artists.h"
#include "Utils/MetaData/MetaData.h"
#include "Utils/MetaData/Artist.h"
#include "Utils/Library/Filter.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Utils.h"

using DB::Artists;
using DB::Query;

struct Artists::Private
{
	QString search_view;
	QString track_view;

	Private(LibraryId library_id)
	{
		if(library_id < 0) {
			search_view = QString("track_search_view");
			track_view = QString("tracks");
		}

		else {
			search_view = QString("track_search_view_%1").arg(library_id);
			track_view = QString("track_view_%1").arg(library_id);
		}
	}
};


Artists::Artists(const QString& connection_name, DbId db_id, LibraryId library_id) :
	DB::SearchableModule(connection_name, db_id)
{
	m = Pimpl::make<Private>(library_id);
}

Artists::~Artists() {}

QString Artists::fetch_query_artists(bool also_empty) const
{
	QString sql =
			"SELECT "
			"artists.artistID AS artistID "
			", artists.name AS artistName "
			", COUNT(DISTINCT " + m->track_view + ".trackID) AS trackCount "
			" FROM artists ";

	QString join = " INNER JOIN ";
	if(also_empty){
		join = " LEFT OUTER JOIN ";
	}

	sql += join + " " + m->track_view + " ON " + m->track_view + "." + artistid_field() + " = artists.artistID ";
	sql += join + " albums ON " + m->track_view + ".albumID = albums.albumID ";

	return sql;
}

bool Artists::db_fetch_artists(Query& q, ArtistList& result)
{
	result.clear();

	if (!q.exec()) {
		q.show_error("Could not get all artists from database");
		return false;
	}

	result.reserve(q.fetched_rows());

	while(q.next())
	{
		Artist artist;

		artist.id = q.value(0).toInt();
		artist.set_name(q.value(1).toString().trimmed());
		artist.num_songs = q.value(2).toInt();
		artist.set_db_id(db_id());

		result << std::move(artist);
	}

	return true;
}

QString Artists::_create_order_string(Library::SortOrder sort)
{
	switch(sort) {
		case Library::SortOrder::ArtistNameAsc:
			return QString(" ORDER BY artistName ASC ");
		case Library::SortOrder::ArtistNameDesc:
			return QString(" ORDER BY artistName DESC ");
		case Library::SortOrder::ArtistTrackcountAsc:
			return QString(" ORDER BY trackCount ASC, artistName ASC ");
		case Library::SortOrder::ArtistTrackcountDesc:
			return QString(" ORDER BY trackCount DESC, artistName DESC ");
		default:
			return  "";
	}
}


bool Artists::getArtistByID(int id, Artist& artist, bool also_empty)
{
	if(id < 0) return false;

	Query q(this);

	ArtistList artists;

	QString query = fetch_query_artists(also_empty) +
				"WHERE artists.artistID = ? "
				"GROUP BY artistName;";

	q.prepare(query);
	q.addBindValue(id);

	bool success = db_fetch_artists(q, artists);
	if(!success){
		return false;
	}

	if(artists.size() > 0) {
		success = true;
		artist = artists.first();
	}

	else {
		success = false;
	}

	return success;
}

ArtistId Artists::getArtistID(const QString& artist)
{
	Query q(this);
	ArtistId artistID = -1;

	QString query("SELECT artistID FROM artists WHERE name = :name;");

	q.prepare(query);
	q.bindValue(":name", Util::cvt_not_null(artist));

	if (!q.exec()) {
		q.show_error("Cannot fetch artistID");
		return -1;
	}

	if (q.next()) {
		artistID = q.value(0).toInt();
	}

	return artistID;
}

bool Artists::getAllArtists(ArtistList& result, bool also_empty)
{
	return getAllArtists(result, Library::SortOrder::ArtistNameAsc, also_empty);
}

bool Artists::getAllArtists(ArtistList& result, Library::SortOrder sortorder, bool also_empty)
{
	Query q(this);
	QString query = fetch_query_artists(also_empty);

	query += "GROUP BY artists.artistID, artists.name ";
	query += _create_order_string(sortorder) + ";";

	q.prepare(query);

	return db_fetch_artists(q, result);
}


bool Artists::getAllArtistsBySearchString(const Library::Filter& filter, ArtistList& result, Library::SortOrder sortorder)
{
	Query q(this);
	QString query;
	QString select = "SELECT " +
					 artistid_field() + ", " +
					 artistname_field() + ", " +
					 "COUNT(DISTINCT trackID) AS trackCount "
					 "FROM " + m->search_view + " ";

	QString where_clause;

	switch(filter.mode())
	{
		case Library::Filter::Genre:
			where_clause = "WHERE genre LIKE :searchterm ";
			break;

		case Library::Filter::Filename:
			where_clause = "WHERE filecissearch LIKE :cissearch ";
			break;

		case Library::Filter::Fulltext:
		default:
			where_clause = "WHERE allCissearch LIKE :cissearch ";
			break;
	}

	if(query.isEmpty()){
			query = select +
					where_clause +
					"GROUP BY " + artistid_field() + ", " + artistname_field() + " ";
	}

	query += _create_order_string(sortorder) + ";";

	q.prepare(query);
	q.bindValue(":searchterm",	Util::cvt_not_null(filter.filtertext(true)));
	q.bindValue(":cissearch",	Util::cvt_not_null(filter.search_mode_filtertext(true)));

	return db_fetch_artists(q, result);
}


ArtistId Artists::insertArtistIntoDatabase(const QString& artist)
{
	ArtistId id = getArtistID(artist);
	if(id >= 0){
		return id;
	}

	Query q(this);

	QString cissearch = Library::Util::convert_search_string(artist, search_mode());
	q.prepare("INSERT INTO artists (name, cissearch) values (:artist, :cissearch);");

	q.bindValue(":artist",		Util::cvt_not_null(artist));
	q.bindValue(":cissearch",	Util::cvt_not_null(cissearch));

	if (!q.exec()) {
		q.show_error(QString("Cannot insert artist ") + artist);
		return -1;
	}

	return getArtistID(artist);
}

ArtistId Artists::insertArtistIntoDatabase (const Artist& artist)
{
	if(artist.id >= 0)
	{
		updateArtist(artist);
		return artist.id;
	}

	return insertArtistIntoDatabase(artist.name());
}


ArtistId Artists::updateArtist(const Artist &artist)
{
	Query q(this);

	if(artist.id < 0) return -1;

	QString cissearch = Library::Util::convert_search_string(artist.name(), search_mode());

	q.prepare("UPDATE artists SET name = :name, cissearch = :cissearch WHERE artistID = :artist_id;");

	q.bindValue(":name",		Util::cvt_not_null(artist.name()));
	q.bindValue(":cissearch",	Util::cvt_not_null(cissearch));
	q.bindValue(":artist_id",	artist.id);

	if (!q.exec()) {
		q.show_error(QString("Cannot insert (2) artist ") + artist.name());
		return -1;
	}

	return artist.id;
}

void Artists::updateArtistCissearch()
{
	SearchableModule::update_search_mode();

	ArtistList artists;
	getAllArtists(artists, true);

	db().transaction();

	for(const Artist& artist : artists)
	{
		QString str = "UPDATE artists SET cissearch=:cissearch WHERE artistID=:id;";
		Query q(this);
		q.prepare(str);
		QString conv_search = Library::Util::convert_search_string(artist.name(), search_mode());
		q.bindValue(":cissearch",	Util::cvt_not_null(conv_search));
		q.bindValue(":id",			artist.id);

		if(!q.exec()){
			q.show_error("Cannot update artist cissearch");
		}
	}

	db().commit();
}
