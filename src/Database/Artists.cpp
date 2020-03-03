/* DatabaseArtists.cpp */

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

#include "Database/Query.h"
#include "Database/Artists.h"
#include "Utils/MetaData/MetaData.h"
#include "Utils/MetaData/Artist.h"
#include "Utils/Library/Filter.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Utils.h"

using DB::Artists;
using DB::Query;

Artists::Artists() = default;
Artists::~Artists() = default;

QString Artists::fetchQueryArtists(bool also_empty) const
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

	query += join + " %1 ON %1.%2 = artists.artistID ";			// join with tracks
	query += join + " albums ON %1.albumID = albums.albumID ";	// join with albums

	return query.arg(trackView()).arg(artistIdField());
}


bool Artists::db_fetch_artists(Query& q, ArtistList& result) const
{
	result.clear();

	if (!q.exec()) {
		q.showError("Could not get all artists from database");
		return false;
	}

	while(q.next())
	{
		Artist artist;

		artist.setId(		q.value(0).value<ArtistId>());
		artist.setName(	q.value(1).toString());
		artist.setSongcount(q.value(2).value<uint16_t>());
		artist.setDatabaseId(	module()->databaseId());

		result << artist;
	}

	return true;
}

bool Artists::getArtistByID(ArtistId id, Artist& artist) const
{
    return getArtistByID(id, artist, false);
}

bool Artists::getArtistByID(ArtistId id, Artist& artist, bool also_empty) const
{
	if(id < 0) {
		return false;
	}

	QString query = fetchQueryArtists(also_empty) + " WHERE artists.artistID = ?;";

	Query q(module());
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


ArtistId Artists::getArtistID(const QString& artist) const
{
	Query q = module()->runQuery
	(
		"SELECT artistID FROM artists WHERE name = :name;",
		{":name", Util::convertNotNull(artist)},
		QString("Cannot fetch artistID for artist %1").arg(artist)
	);

	if (q.hasError()) {
		return -1;
	}

	if (q.next()) {
		return q.value(0).toInt();
	}

	return -1;
}

bool Artists::getAllArtists(ArtistList& result, bool also_empty) const
{
	QString query = fetchQueryArtists(also_empty);
	query += "GROUP BY artists.artistID, artists.name; ";

	Query q(module());
	q.prepare(query);

	return db_fetch_artists(q, result);
}

bool Artists::getAllArtistsBySearchString(const Library::Filter& filter, ArtistList& result) const
{
	QStringList filters = filter.filtertext(true);
	QStringList search_filters = filter.searchModeFiltertext(true);
	for(int i=0; i<filters.size(); i++)
	{
		Query q(module());

		QString query = "SELECT " +
						 artistIdField() + ", " +
						 artistNameField() + ", " +
						 "COUNT(DISTINCT trackID) AS trackCount "
						 "FROM " + trackSearchView() + " ";

		query += " WHERE ";

		switch(filter.mode())
		{
			case Library::Filter::Genre:
				if(filter.isInvalidGenre()){
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
						.arg(artistIdField())
						.arg(artistNameField());

		q.prepare(query);
		q.bindValue(":searchterm",	Util::convertNotNull(filters[i]));
		q.bindValue(":cissearch",	Util::convertNotNull(search_filters[i]));

		ArtistList tmp_list;
		db_fetch_artists(q, tmp_list);
		result.appendUnique(tmp_list);
	}

	return true;
}

bool Artists::deleteArtist(ArtistId id)
{
	QMap<QString, QVariant> bindings
	{
		{"id", id}
	};

	Query q = module()->runQuery
	(
		"delete from artists where artistId=:artistId;",
		{":artistId", id},
		QString("Cannot delete artist %1").arg(id)
	);

	return (!q.hasError());
}

ArtistId Artists::insertArtistIntoDatabase(const QString& artist)
{
	ArtistId id = getArtistID(artist);
	if(id >= 0){
		return id;
	}

	QString cis = Library::Utils::convertSearchstring(artist, searchMode());

	QMap<QString, QVariant> bindings
	{
		{"name", Util::convertNotNull(artist)},
		{"cissearch", Util::convertNotNull(cis)}
	};

	Query q = module()->insert("artists", bindings, QString("Cannot insert artist %1").arg(artist));
	if(q.hasError()){
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
	Library::SearchModeMask sm = searchMode();

	ArtistList artists;
	getAllArtists(artists, true);

	module()->db().transaction();

	for(const Artist& artist : artists)
	{
		QString cis = Library::Utils::convertSearchstring(artist.name(), sm);

		module()->update
		(
			"artists",
			{{"cissearch", Util::convertNotNull(cis)}},
			{"artistID", artist.id()},
			"Cannot update artist cissearch"
		);
	}

	module()->db().commit();
}

