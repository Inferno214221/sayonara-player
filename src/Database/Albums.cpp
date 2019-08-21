/* DatabaseAlbums.cpp */

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
#include "Database/Albums.h"
#include "Utils/MetaData/Album.h"
#include "Utils/Library/Filter.h"
#include "Utils/Utils.h"

using DB::Albums;
using DB::Query;
using ::Library::Filter;

static QString get_filter_clause(const Filter& filter, QString cis_placeholder, QString searchterm_placeholder);

struct Albums::Private
{
	QString search_view;
	QString track_view;

	explicit Private(LibraryId library_id)
	{
		if(library_id < 0) {
			track_view = "tracks";
			search_view = QString("track_search_view");
		}

		else {
			track_view = QString("track_view_%1").arg(library_id);
			search_view = QString("track_search_view_%1").arg(library_id);
		}
	}
};

Albums::Albums(const QString& connection_name, DbId db_id, LibraryId library_id) :
	DB::SearchableModule(connection_name, db_id)
{
	m = Pimpl::make<Private>(library_id);
}

QString Albums::fetch_query_albums(bool also_empty) const
{
	QStringList fields
	{
	    "albums.albumID		AS albumID",		               // 0
		"albums.name		AS albumName",                     // 1
		"albums.rating		AS albumRating",                   // 2
		"GROUP_CONCAT(DISTINCT artists.name)      AS artistNames",	    // 3
		"GROUP_CONCAT(DISTINCT albumArtists.name) AS albumArtistNames", // 4
		"SUM(%1.length) / 1000				AS albumLength",   // 5
		"COUNT(DISTINCT %1.trackID)			AS trackCount",    // 6
		"MAX(%1.year)						AS albumYear",     // 7
		"GROUP_CONCAT(DISTINCT %1.discnumber) AS discnumbers", // 8
		"GROUP_CONCAT(%1.filename, '#') AS filenames"      // 9
	};

	QString query = "SELECT " + fields.join(", ") + " FROM albums ";

	QString join = " INNER JOIN ";
	if(also_empty){
		join = " LEFT OUTER JOIN ";
	}

	query += join + QString(" %1 ON %1.albumID = albums.albumID ");
	query += join + QString(" artists ON %1.artistID = artists.artistID ");
	query += join + QString(" artists albumArtists ON %1.albumArtistID = albumArtists.artistID ");

	return query.arg(m->track_view);
}

DB::Albums::~Albums() = default;

bool Albums::db_fetch_albums(Query& q, AlbumList& result) const
{
	result.clear();

	if (!q.exec()) {
		q.show_error("Could not get all albums from database");
		return false;
	}

	while(q.next())
	{
		Album album;

		album.id =			q.value(0).value<AlbumId>();
		album.set_name(		q.value(1).toString());
		album.rating =		q.value(2).value<Rating>();
		album.set_artists(	q.value(3).toString().split(','));
		album.set_album_artists(q.value(4).toString().split(','));
		album.length_sec =	q.value(5).value<Seconds>();
		album.num_songs =	q.value(6).value<uint16_t>();
		album.year =		q.value(7).value<uint16_t>();

		QStringList discs =	q.value(8).toString().split(',');
		if(discs.isEmpty()){
			album.discnumbers << 1;
		}

		discs.removeDuplicates();
		for(const QString& disc : discs) {
			album.discnumbers << Disc(disc.toInt());
		}

		album.n_discs = Disc(album.discnumbers.size());
		album.is_sampler = (album.artists().size() > 1);
		album.set_db_id(db_id());

		album.set_path_hint(q.value(9).toString().split("#"));

		result.push_back(std::move(album));
	};

	return true;
}


AlbumId Albums::getAlbumID(const QString& album) const
{
	Query q(this);
	int albumID = -1;

	q.prepare("SELECT albumID FROM albums WHERE name = ?;");
	q.addBindValue(Util::cvt_not_null(album));

	if(!q.exec()) {
		q.show_error("Cannot fetch albumID");
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

	Query q(this);
	QString query =	fetch_query_albums(also_empty) +
						" WHERE albums.albumID = :id "
						" GROUP BY albums.albumID, albums.name, albums.rating ";

	q.prepare(query);
	q.bindValue(":id", id);

	AlbumList albums;
	db_fetch_albums(q, albums);

	if(!albums.empty()) {
		album = albums.first();
	}

	return (!albums.empty());
}

bool Albums::getAllAlbums(AlbumList& result, bool also_empty) const
{
	Query q(this);
	QString query = fetch_query_albums(also_empty);

	query += " GROUP BY albums.albumID, albums.name, albums.rating; ";

	q.prepare(query);

	return db_fetch_albums(q, result);
}


bool Albums::getAllAlbumsByArtist(const IdList &artists, AlbumList &result, const Library::Filter &filter) const
{
	if(artists.isEmpty()) {
		return false;
	}

	QStringList filters = filter.filtertext(true);
	QStringList search_filters = filter.search_mode_filtertext(true);
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

		QString query = "SELECT " + fields.join(", ") + " FROM " + m->search_view + " WHERE ";

		if( !filter.cleared() )
		{
			query += get_filter_clause(filter, "cissearch", "searchterm") + " AND ";
		}

		{ // artist conditions
			QString aidf = m->search_view + "." + artistid_field();

			QStringList or_clauses;
			for(int a=0; a<artists.size(); a++) {
				or_clauses << QString("%1 = :artist_id_%2").arg(aidf).arg(a);
			}

			query += " (" + or_clauses.join(" OR ") + ") ";
		}

		query += " GROUP BY albumID, albumName; ";

		{ // prepare and run
			Query q(this);
			q.prepare(query);

			q.bindValue(":searchterm",	filters[i]);
			q.bindValue(":cissearch",	search_filters[i]);

			for(int i=0; i<artists.size(); i++) {
				q.bindValue(QString(":artist_id_") + QString::number(i), artists[i]);
			}

			AlbumList tmp_list;
			db_fetch_albums(q, tmp_list);
			result.append_unique(tmp_list);
		}
	}

	return true;
}


bool Albums::getAllAlbumsBySearchString(const Library::Filter& filter, AlbumList& result) const
{
	QStringList filters = filter.filtertext(true);
	QStringList search_filters = filter.search_mode_filtertext(true);
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

		QString query = "SELECT " + fields.join(", ") + " FROM " + m->search_view + " WHERE ";
		query += get_filter_clause(filter, "cissearch", "searchterm");
		query += " GROUP BY albumID, albumName;";

		Query q(this);
		q.prepare(query);
		q.bindValue(":searchterm",	filters[i]);
		q.bindValue(":cissearch",	search_filters[i]);

		AlbumList tmp_list;
		db_fetch_albums(q, tmp_list);
		result.append_unique(tmp_list);
	}

	return true;
}

AlbumId Albums::updateAlbum(const Album& album)
{
	AlbumId id = album.id;
	if(id < 0)
	{
		id = getAlbumID(album.name());
		if(id < 0) {
			return -1;
		}
	}

	QString cissearch = Library::Utils::convert_search_string(album.name(), search_mode());
	QMap<QString, QVariant> bindings
	{
		{"name",		Util::cvt_not_null(album.name())},
		{"cissearch",	Util::cvt_not_null(cissearch)},
		{"rating",		QVariant::fromValue(int(album.rating))}
	};

	Query q = update("albums", bindings, {"albumID", id}, QString("Cannot update album %1").arg(album.name()));
	if (q.has_error()) {
		return -1;
	}

	return id;
}

void Albums::updateAlbumCissearch()
{
	SearchableModule::update_search_mode();
	Library::SearchModeMask sm = search_mode();

	AlbumList albums;
	getAllAlbums(albums, true);

	db().transaction();

	for(const Album& album : albums)
	{
		QString cis = Library::Utils::convert_search_string(album.name(), sm);

		this->update
		(
			"albums",
			{{"cissearch", Util::cvt_not_null(cis)}},
			{"albumID", album.id},
			"Cannot update album cissearch"
		);
	}

	db().commit();
}

AlbumId Albums::insertAlbumIntoDatabase(const QString& album_name)
{
	Album album;
	album.set_name(album_name);

	return insertAlbumIntoDatabase(album);
}

AlbumId Albums::insertAlbumIntoDatabase(const Album& album)
{
	AlbumId id = updateAlbum(album);
	if(id >= 0){
		return id;
	}

	QString cissearch = Library::Utils::convert_search_string(album.name(), search_mode());

	QMap<QString, QVariant> bindings
	{
		{"name",		Util::cvt_not_null(album.name())},
		{"cissearch",	Util::cvt_not_null(cissearch)},
		{"rating",		QVariant::fromValue(int(album.rating))}
	};

	Query q = insert("albums", bindings, QString("2. Cannot insert album %1").arg(album.name()));
	if (q.has_error()) {
		return -1;
	}

	return q.lastInsertId().toInt();
}


static QString get_filter_clause(const Filter& filter, QString cis_placeholder, QString searchterm_placeholder)
{
	cis_placeholder.remove(":");
	searchterm_placeholder.remove(":");

	switch(filter.mode())
	{
		case Library::Filter::Genre:
			if(filter.is_invalid_genre()){
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
