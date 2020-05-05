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
#include "Utils/Algorithm.h"

using DB::Albums;
using DB::Query;
using ::Library::Filter;

static QString getFilterClause(const Filter& filter, QString cisPlaceholder);

static void dropAlbumView(DB::Module* module, const QString& albumView)
{
	module->runQuery
	(
		"DROP VIEW IF EXISTS " + albumView + "; ",
		"Cannot drop album view"
	);
}

static void createAlbumView(DB::Module* module, const QString& trackView, const QString& albumView)
{
	const QStringList fields
	{
	    "albums.albumID								AS albumID",			// 0
		"albums.name								AS albumName",			// 1
		"albums.rating								AS albumRating",		// 2
		"GROUP_CONCAT(DISTINCT artists.name)		AS artistNames",		// 3
		"GROUP_CONCAT(DISTINCT albumArtists.name)	AS albumArtistName",	// 4
		"SUM(%1.length) / 1000						AS albumLength",		// 5
		"COUNT(DISTINCT %1.trackID)					AS trackCount",			// 6
		"MAX(%1.year)								AS albumYear",			// 7
		"GROUP_CONCAT(DISTINCT %1.discnumber)		AS discnumbers",		// 8
		"GROUP_CONCAT(%1.filename, '#')				AS filenames"			// 9
	};

	const QString query = QString
	(
		"CREATE VIEW " + albumView + " AS "
		"SELECT " + fields.join(", ") + " FROM albums "
		"LEFT OUTER JOIN %1 ON %1.albumID = albums.albumID "							// leave out empty albums
		"LEFT OUTER JOIN artists ON %1.artistID = artists.artistID "
		"LEFT OUTER JOIN artists albumArtists ON %1.albumArtistID = albumArtists.artistID "
		"GROUP BY albums.albumID; "
	).arg(trackView);

	module->runQuery(query, "Cannot create album view");
}

Albums::Albums() = default;
Albums::~Albums() = default;

static QString albumViewName(LibraryId libraryId)
{
	if(libraryId < 0){
		return "album_view";
	}

	return QString("album_view_%1").arg(QString::number(libraryId));
}

void Albums::initViews()
{
	const QString& viewName = albumViewName(libraryId());

	dropAlbumView(module(), viewName);
	createAlbumView(module(), trackView(), viewName);
}

QString Albums::fetchQueryAlbums(bool alsoEmpty) const
{
	const QStringList fields
	{
	    "albumID",			// 0
		"albumName",		// 1
		"albumRating",		// 2
		"artistNames",		// 3
		"albumArtistName",	// 4
		"albumLength",		// 5
		"trackCount",		// 6
		"albumYear",		// 7
		"discnumbers",		// 8
		"filenames"			// 9
	};

	QString query = "SELECT " + fields.join(", ") + " FROM " + albumViewName(libraryId());

	if(alsoEmpty) {
		query += " WHERE 1 ";
	}
	else {
		query += " WHERE trackCount > 0 ";
	}

	return query;
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

		album.setId(q.value(0).value<AlbumId>());
		album.setName(q.value(1).toString());
		album.setRating(q.value(2).value<Rating>());
		album.setArtists(q.value(3).toString().split(','));
		album.setAlbumArtist(q.value(4).toString());
		album.setDurationSec(q.value(5).value<Seconds>());
		album.setSongcount(q.value(6).value<TrackNum>());
		album.setYear(q.value(7).value<Year>());

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
	}

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

bool Albums::getAlbumByID(AlbumId id, Album& album, bool alsoEmpty) const
{
	if(id == -1) {
		return false;
	}

	const QString query =
		fetchQueryAlbums(alsoEmpty) +
		" AND albumID = :id " +
		" GROUP BY albumID, albumName, albumRating;";

	Query q(module());
	q.prepare(query);
	q.bindValue(":id", id);

	AlbumList albums;
	dbFetchAlbums(q, albums);

	if(!albums.empty()) {
		album = albums.first();
	}

	return (!albums.empty());
}

bool Albums::getAllAlbums(AlbumList& result, bool alsoEmpty) const
{
	const QString query =
		fetchQueryAlbums(alsoEmpty) +
		" GROUP BY albumID, albumName, albumRating;";

	Query q(module());
	q.prepare(query);

	return dbFetchAlbums(q, result);
}


bool Albums::getAllAlbumsByArtist(const IdList& artists, AlbumList& result, const Library::Filter& filter) const
{
	if(artists.isEmpty()) {
		return false;
	}

	const QStringList fields
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

	const QString joinedFields = fields.join(", ");
	const QString searchView = trackSearchView();

	const QStringList searchFilters = filter.searchModeFiltertext(true);
	for(const QString& searchFilter : searchFilters)
	{
		QString query = "SELECT " + joinedFields + " FROM " + searchView + " WHERE ";

		if( !filter.cleared() )
		{
			query += getFilterClause(filter, ":cissearch") + " AND ";
		}

		{ // artist conditions
			const QString aidf = searchView + "." + artistIdField();

			QStringList orClauses;
			Util::Algorithm::transform(artists, orClauses, [aidf](ArtistId artistId)
			{
				return QString("%1 = :artistId%2").arg(aidf).arg(artistId);
			});

			query += " (" + orClauses.join(" OR ") + ") ";
		}

		query += " GROUP BY albumID, albumName; ";

		{ // prepare and run
			Query q(module());
			q.prepare(query);
			q.bindValue(":cissearch", searchFilter);

			for(ArtistId artistId : artists)
			{
				q.bindValue(QString(":artistId%1").arg(artistId), artistId);
			}

			AlbumList tmpAlbums;
			dbFetchAlbums(q, tmpAlbums);
			result.appendUnique(tmpAlbums);
		}
	}

	return true;
}


bool Albums::getAllAlbumsBySearchString(const Library::Filter& filter, AlbumList& result) const
{
	const QStringList fields
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

	const QString joinedFields = fields.join(", ");

	const QStringList searchFilters = filter.searchModeFiltertext(true);
	for(const QString& searchFilter : searchFilters)
	{
		QString query = "SELECT " + joinedFields + " FROM " + trackSearchView() + " WHERE ";
		query += getFilterClause(filter, ":cissearch");
		query += " GROUP BY albumID, albumName;";

		Query q(module());
		q.prepare(query);
		q.bindValue(":cissearch", searchFilter);

		AlbumList tmpList;
		dbFetchAlbums(q, tmpList);
		result.appendUnique(tmpList);
	}

	return true;
}

AlbumId Albums::updateAlbumRating(AlbumId id, Rating rating)
{
	QMap<QString, QVariant> bindings
	{
		{"rating", QVariant::fromValue(int(rating))}
	};

	Query q = module()->update("albums", bindings, {"albumID", id}, QString("Cannot set album rating for id %1").arg(id));
	if (q.hasError()) {
		return -1;
	}

	return id;
}

void Albums::updateAlbumCissearch()
{
	AlbumList albums;
	getAllAlbums(albums, true);

	module()->db().transaction();

	for(const Album& album : albums)
	{
		const QString cissearch = Library::Utils::convertSearchstring(album.name());

		module()->update
		(
			"albums",
			{{"cissearch", cissearch}},
			{"albumID", album.id()},
			"Cannot update album cissearch"
		);
	}

	module()->db().commit();
}

AlbumId Albums::insertAlbumIntoDatabase(const QString& name)
{
	const QString cissearch = Library::Utils::convertSearchstring(name);

	QMap<QString, QVariant> bindings
	{
		{"name",		Util::convertNotNull(name)},
		{"cissearch",	cissearch},
		{"rating",		QVariant::fromValue(int(Rating::Zero))}
	};

	Query q = module()->insert("albums", bindings, QString("2. Cannot insert album %1").arg(name));
	if (q.hasError()) {
		return -1;
	}

	return q.lastInsertId().toInt();
}

AlbumId Albums::insertAlbumIntoDatabase(const Album& album)
{
	return insertAlbumIntoDatabase(album.name());
}

void Albums::deleteAllAlbums()
{
	module()->runQuery("DELETE FROM albums;", "Could not delete all albums");
}

static QString getFilterClause(const Filter& filter, QString placeholder)
{
	placeholder.remove(":");

	switch(filter.mode())
	{
		case Library::Filter::Genre:
			if(filter.isInvalidGenre()){
				return " genre = '' ";
			}
			else {
				return " genreCissearch LIKE :" + placeholder + " ";
			}

		case Library::Filter::Filename:
			return " fileCissearch LIKE :" + placeholder + " ";

		case Library::Filter::Fulltext:
		default:
			return " allCissearch LIKE :" + placeholder + " ";
	}
}
