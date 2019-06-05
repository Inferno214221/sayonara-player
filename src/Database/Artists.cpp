/* DatabaseArtists.cpp */

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

	explicit Private(LibraryId library_id)
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

DB::Artists::~Artists() = default;


QString Artists::fetch_query_artists(bool also_empty) const
{
	QStringList fields
	{
			"artists.artistID           AS artistID",
			"artists.name               AS artistName",
			"COUNT(DISTINCT %1.trackID) AS trackCount"
	};

	QString query = "SELECT " + fields.join(", ") + " FROM artists ";

	QString join = " INNER JOIN ";
	if(also_empty){
		join = " LEFT OUTER JOIN ";
	}

	query += join + " %1 ON %1.%2 = artists.artistID ";
	query += join + " albums ON %1.albumID = albums.albumID ";

	return query.arg(m->track_view).arg(artistid_field());
}


bool Artists::db_fetch_artists(Query& q, ArtistList& result)
{
	result.clear();

	if (!q.exec()) {
		q.show_error("Could not get all artists from database");
		return false;
	}

	while(q.next())
	{
		Artist artist;

		artist.id =			q.value(0).toInt();
		artist.set_name(	q.value(1).toString().trimmed());
		artist.num_songs =	q.value(2).toInt();
		artist.set_db_id(	db_id());

		result << artist;
	}

	return true;
}

bool Artists::getArtistByID(ArtistId id, Artist& artist)
{
    return getArtistByID(id, artist, false);
}

bool Artists::getArtistByID(ArtistId id, Artist& artist, bool also_empty)
{
	if(id < 0) {
		return false;
	}

	QString query = fetch_query_artists(also_empty) + " WHERE artists.artistID = ?;";

	Query q(this);
	q.prepare(query);
	q.addBindValue(id);

	ArtistList artists;
	bool success = db_fetch_artists(q, artists);
	if(!success || artists.empty()){
		return false;
	}

	artist = artists.first();
	return true;
}

ArtistId Artists::getArtistID(const QString& artist)
{
	Query q = run_query
	(
		"SELECT artistID FROM artists WHERE name = :name;",
		{":name", Util::cvt_not_null(artist)},
		QString("Cannot fetch artistID for artist %1").arg(artist)
	);

	if (q.has_error()) {
		return -1;
	}

	if (q.next()) {
		return q.value(0).toInt();
	}

	return -1;
}

bool Artists::getAllArtists(ArtistList& result, bool also_empty)
{
	QString query = fetch_query_artists(also_empty);
	query += "GROUP BY artists.artistID, artists.name; ";

	Query q(this);
	q.prepare(query);

	return db_fetch_artists(q, result);
}

bool Artists::getAllArtistsBySearchString(const Library::Filter& filter, ArtistList& result)
{
	QStringList filters = filter.filtertext(true);
	QStringList search_filters = filter.search_mode_filtertext(true);
	for(int i=0; i<filters.size(); i++)
	{
		Query q(this);

		QString query = "SELECT " +
						 artistid_field() + ", " +
						 artistname_field() + ", " +
						 "COUNT(DISTINCT trackID) AS trackCount "
						 "FROM " + m->search_view + " ";

		query += " WHERE ";

		switch(filter.mode())
		{
			case Library::Filter::Genre:
				if(filter.is_invalid_genre()){
					query += "genre = ''";
				}
				else {
					query += "genre LIKE :searchterm";
				}
				break;

			case Library::Filter::Filename:
				query += "filecissearch LIKE :cissearch";
				break;

			case Library::Filter::Fulltext:
			default:
				query += "allCissearch LIKE :cissearch";
				break;
		}

		query += QString(" GROUP BY %1, %2;")
						.arg(artistid_field())
						.arg(artistname_field());

		q.prepare(query);
		q.bindValue(":searchterm",	Util::cvt_not_null(filters[i]));
		q.bindValue(":cissearch",	Util::cvt_not_null(search_filters[i]));

		ArtistList tmp_list;
		db_fetch_artists(q, tmp_list);
		result.append_unique(tmp_list);
	}

	return true;
}

bool Artists::deleteArtist(ArtistId id)
{
	QMap<QString, QVariant> bindings
	{
		{"id", id}
	};

	Query q = run_query
	(
		"delete from artists where artist_id=:artist_id;",
		{":artist_id", id},
		QString("Cannot delete artist %1").arg(id)
	);

	return (!q.has_error());
}

ArtistId Artists::updateArtist(const Artist& artist)
{
	if(artist.id < 0){
		return -1;
	}

	QString cis = Library::Utils::convert_search_string(artist.name(), search_mode());

	QMap<QString, QVariant> bindings
	{
		{"name", Util::cvt_not_null(artist.name())},
		{"cissearch", Util::cvt_not_null(cis)}
	};

	Query q = update("artists", bindings, {"artistID", artist.id}, QString("Cannot insert artist %1").arg(artist.name()));
	if(q.has_error()){
		return -1;
	}

	return artist.id;
}


ArtistId Artists::insertArtistIntoDatabase(const QString& artist)
{
	ArtistId id = getArtistID(artist);
	if(id >= 0){
		Artist a; a.set_name(artist);
		return updateArtist(a);
	}

	QString cis = Library::Utils::convert_search_string(artist, search_mode());

	QMap<QString, QVariant> bindings
	{
		{"name", Util::cvt_not_null(artist)},
		{"cissearch", Util::cvt_not_null(cis)}
	};

	Query q = insert("artists", bindings, QString("Cannot insert artist %1").arg(artist));
	if(q.has_error()){
		return -1;
	}

	return q.lastInsertId().toInt();
}

ArtistId Artists::insertArtistIntoDatabase(const Artist& artist)
{
	return insertArtistIntoDatabase(artist.name());
}

void Artists::updateArtistCissearch()
{
	SearchableModule::update_search_mode();
	Library::SearchModeMask sm = search_mode();

	ArtistList artists;
	getAllArtists(artists, true);

	db().transaction();

	for(const Artist& artist : artists)
	{
		QString cis = Library::Utils::convert_search_string(artist.name(), sm);

		this->update
		(
			"artists",
			{{"cissearch", Util::cvt_not_null(cis)}},
			{"artistID", artist.id},
			"Cannot update artist cissearch"
		);
	}

	db().commit();
}

