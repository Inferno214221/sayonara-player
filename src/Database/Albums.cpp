/* DatabaseAlbums.cpp */

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
#include "Database/Albums.h"
#include "Utils/MetaData/Album.h"
#include "Utils/Library/Filter.h"
#include "Utils/Utils.h"

using DB::Albums;
using DB::Query;
using ::Library::Filter;

static QString get_filter_clause(const Filter& filter, QString cis_placeholder, QString searchterm_placeholder);

Albums::Albums() = default;
Albums::~Albums() = default;

QString Albums::fetchQueryAlbums(bool also_empty) const
{
	QStringList fields
	{
	    "albums.albumID								AS albumID",			// 0
		"albums.name								AS albumName",			// 1
		"albums.rating								AS albumRating",		// 2
		"GROUP_CONCAT(DISTINCT artists.name)		AS artistNames",		// 3
		"GROUP_CONCAT(DISTINCT albumArtists.name)	AS albumArtistNames",	// 4
		"SUM(%1.length) / 1000						AS albumLength",		// 5
		"COUNT(DISTINCT %1.trackID)					AS trackCount",			// 6
		"MAX(%1.year)								AS albumYear",			// 7
		"GROUP_CONCAT(DISTINCT %1.discnumber)		AS discnumbers",		// 8
		"GROUP_CONCAT(%1.filename, '#')				AS filenames"			// 9
	};

	QString query = "SELECT " + fields.join(", ") + " FROM albums ";

	QString join = " INNER JOIN ";
	if(also_empty){
		join = " LEFT OUTER JOIN ";
	}

	query += join + QString(" %1 ON %1.albumID = albums.albumID ");
	query += join + QString(" artists ON %1.artistID = artists.artistID ");
	query += join + QString(" artists albumArtists ON %1.albumArtistID = albumArtists.artistID ");

	return query.arg(trackView());
}

bool Albums::dbFetchAlbums(Query& q, AlbumList& result) const
{
	result.clear();

	if (!q.exec()) {
		q.showError("Could not get all albums from database");
		return false;
	}

	while(q.next())
	{
		Album album;

		album.setId(		q.value(0).value<AlbumId>());
		album.setName(		q.value(1).toString());
		album.setRating(	q.value(2).value<Rating>());
		album.setArtists(	q.value(3).toString().split(','));
		album.setAlbumArtists(q.value(4).toString().split(','));
		album.setDurationSec(q.value(5).value<Seconds>());
		album.setSongcount(q.value(6).value<TrackNum>());
		album.setYear(		q.value(7).value<Year>());

		QStringList discs =	q.value(8).toString().split(',');
		auto discnumbers = album.discnumbers();
		if(discs.isEmpty()){
			discnumbers << 1;
		}

		discs.removeDuplicates();
		for(const QString& disc : discs) {
			discnumbers << Disc(disc.toInt());
		}

		album.setDiscnumbers(discnumbers);
		album.setDatabaseId(module()->databaseId());
		album.setPathHint(q.value(9).toString().split("#"));

		result.push_back(std::move(album));
	};

	return true;
}


AlbumId Albums::getAlbumID(const QString& album) const
{
	Query q(module());
	int albumID = -1;

	q.prepare("SELECT albumID FROM albums WHERE name = ?;");
	q.addBindValue(Util::convertNotNull(album));

	if(!q.exec()) {
		q.showError("Cannot fetch albumID");
		return -1;
	}

	if (q.next()) {
		albumID = q.value(0).toInt();
	}

	return albumID;
}

bool Albums::getAlbumByID(AlbumId id, Album& album) const
{
    return getAlbumByID(id, album, false);
}

bool Albums::getAlbumByID(AlbumId id, Album& album, bool also_empty) const
{
	if(id == -1) {
		return false;
	}

	Query q(module());
	QString query =	fetchQueryAlbums(also_empty) +
						" WHERE albums.albumID = :id "
						" GROUP BY albums.albumID, albums.name, albums.rating ";

	q.prepare(query);
	q.bindValue(":id", id);

	AlbumList albums;
	dbFetchAlbums(q, albums);

	if(!albums.empty()) {
		album = albums.first();
	}

	return (!albums.empty());
}

bool Albums::getAllAlbums(AlbumList& result, bool also_empty) const
{
	Query q(module());
	QString query = fetchQueryAlbums(also_empty);

	query += " GROUP BY albums.albumID, albums.name, albums.rating; ";

	q.prepare(query);

	return dbFetchAlbums(q, result);
}


bool Albums::getAllAlbumsByArtist(const IdList &artists, AlbumList &result, const Library::Filter &filter) const
{
	if(artists.isEmpty()) {
		return false;
	}

	QStringList filters = filter.filtertext(true);
	QStringList search_filters = filter.searchModeFiltertext(true);
	for(int i=0; i<filters.size(); i++)
	{
		QStringList fields
		{
			 "albumID",
			 "albumName",
			 "albumRating",
			 "GROUP_CONCAT(DISTINCT artistName)",
			 "GROUP_CONCAT(DISTINCT albumArtistName)",
			 "SUM(length) / 1000 AS albumLength",
			 "COUNT(DISTINCT trackID) AS trackCount",
			 "MAX(year) AS albumYear",
			 "GROUP_CONCAT(DISTINCT discnumber)",
			 "GROUP_CONCAT(DISTINCT filename)"
		};

		const QString search_view = trackSearchView();
		QString query = "SELECT " + fields.join(", ") + " FROM " + search_view + " WHERE ";

		if( !filter.cleared() )
		{
			query += get_filter_clause(filter, "cissearch", "searchterm") + " AND ";
		}

		{ // artist conditions
			QString aidf = search_view + "." + artistIdField();

			QStringList or_clauses;
			for(int a=0; a<artists.size(); a++) {
				or_clauses << QString("%1 = :artistId_%2").arg(aidf).arg(a);
			}

			query += " (" + or_clauses.join(" OR ") + ") ";
		}

		query += " GROUP BY albumID, albumName; ";

		{ // prepare and run
			Query q(module());
			q.prepare(query);

			q.bindValue(":searchterm",	filters[i]);
			q.bindValue(":cissearch",	search_filters[i]);

			for(int i=0; i<artists.size(); i++) {
				q.bindValue(QString(":artistId_") + QString::number(i), artists[i]);
			}

			AlbumList tmp_list;
			dbFetchAlbums(q, tmp_list);
			result.appendUnique(tmp_list);
		}
	}

	return true;
}


bool Albums::getAllAlbumsBySearchString(const Library::Filter& filter, AlbumList& result) const
{
	QStringList filters = filter.filtertext(true);
	QStringList search_filters = filter.searchModeFiltertext(true);
	for(int i=0; i<filters.size(); i++)
	{
		QStringList fields
		{
			 "albumID",
			 "albumName",
			 "albumRating",
			 "GROUP_CONCAT(DISTINCT artistName)",
			 "GROUP_CONCAT(DISTINCT albumArtistName)",
			 "SUM(length) / 1000 AS albumLength",
			 "COUNT(DISTINCT trackID) AS trackCount",
			 "MAX(year) AS albumYear",
			 "GROUP_CONCAT(DISTINCT discnumber)",
			 "GROUP_CONCAT(DISTINCT filename)"
		};

		QString query = "SELECT " + fields.join(", ") + " FROM " + trackSearchView() + " WHERE ";
		query += get_filter_clause(filter, "cissearch", "searchterm");
		query += " GROUP BY albumID, albumName;";

		Query q(module());
		q.prepare(query);
		q.bindValue(":searchterm",	filters[i]);
		q.bindValue(":cissearch",	search_filters[i]);

		AlbumList tmp_list;
		dbFetchAlbums(q, tmp_list);
		result.appendUnique(tmp_list);
	}

	return true;
}

AlbumId Albums::updateAlbumRating(AlbumId id, Rating rating)
{
	QMap<QString, QVariant> bindings
	{
		{"rating",		QVariant::fromValue(int(rating))}
	};

	Query q = module()->update("albums", bindings, {"albumID", id}, QString("Cannot set album rating for id %1").arg(id));
	if (q.hasError()) {
		return -1;
	}

	return id;
}

void Albums::updateAlbumCissearch()
{
	Library::SearchModeMask sm = searchMode();

	AlbumList albums;
	getAllAlbums(albums, true);

	module()->db().transaction();

	for(const Album& album : albums)
	{
		QString cis = Library::Utils::convertSearchstring(album.name(), sm);

		module()->update
		(
			"albums",
			{{"cissearch", Util::convertNotNull(cis)}},
			{"albumID", album.id()},
			"Cannot update album cissearch"
		);
	}

	module()->db().commit();
}

AlbumId Albums::insertAlbumIntoDatabase(const QString& album_name)
{
	AlbumId id = getAlbumID(album_name);
	if(id >= 0){
		return id;
	}

	QString cissearch = Library::Utils::convertSearchstring(album_name, searchMode());

	QMap<QString, QVariant> bindings
	{
		{"name",		Util::convertNotNull(album_name)},
		{"cissearch",	Util::convertNotNull(cissearch)},
		{"rating",		QVariant::fromValue(int(Rating::Zero))}
	};

	Query q = module()->insert("albums", bindings, QString("2. Cannot insert album %1").arg(album_name));
	if (q.hasError()) {
		return -1;
	}

	return q.lastInsertId().toInt();

}

AlbumId Albums::insertAlbumIntoDatabase(const Album& album)
{
	return insertAlbumIntoDatabase(album.name());
}


static QString get_filter_clause(const Filter& filter, QString cis_placeholder, QString searchterm_placeholder)
{
	cis_placeholder.remove(":");
	searchterm_placeholder.remove(":");

	switch(filter.mode())
	{
		case Library::Filter::Genre:
			if(filter.isInvalidGenre()){
				return " genre = '' ";
			}
			else {
				return " genre LIKE :" + searchterm_placeholder + " ";
			}

		case Library::Filter::Filename:
			return " filecissearch LIKE :" + cis_placeholder + " ";

		case Library::Filter::Fulltext:
		default:
			return " allCissearch LIKE :" + cis_placeholder + " ";
	}
}
