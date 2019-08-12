/* DatabaseTracks.cpp */

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

#include "Database/Tracks.h"
#include "Database/Library.h"
#include "Database/Query.h"

#include "Utils/MetaData/MetaDataList.h"
#include "Utils/MetaData/Genre.h"

#include "Utils/Algorithm.h"
#include "Utils/Utils.h"
#include "Utils/Set.h"
#include "Utils/Library/Filter.h"
#include "Utils/Library/SearchMode.h"
#include "Utils/Logger/Logger.h"

#include <QDateTime>
#include <QMap>

#include <utility>
#include <tuple>

using DB::Tracks;
using DB::Query;
using DB::SearchableModule;
using SMM=::Library::SearchModeMask;
namespace LibraryUtils=::Library::Utils;
using ::Library::Filter;

static QString get_filter_clause(const Filter& filter, QString cis_placeholder, QString searchterm_placeholder);

struct Tracks::Private
{
	QString track_view;
	QString search_view;

	LibraryId library_id;

	explicit Private(LibraryId library_id) :
		library_id(library_id)
	{
		if(library_id < 0)
		{
			track_view = QString("tracks");
			search_view = QString("track_search_view");
		}

		else
		{
			track_view = QString("track_view_%1").arg(library_id);
			search_view = QString("track_search_view_%1").arg(library_id);
		}
	}
};

Tracks::Tracks(const QString& connection_name, DbId db_id, LibraryId library_id) :
	DB::SearchableModule(connection_name, db_id)
{
	m = Pimpl::make<Private>(library_id);

	QStringList fields
	{
		"tracks.trackID",							// 0
		"tracks.title",								// 1
		"tracks.length",							// 2
		"tracks.year",								// 3
		"tracks.bitrate",							// 4
		"tracks.filename",							// 5
		"tracks.filesize",							// 6
		"tracks.track         AS trackNum",			// 7
		"tracks.genre",								// 8
		"tracks.discnumber",						// 9
		"tracks.rating",							// 10
		"tracks.albumID	      AS albumID",			// 11
		"tracks.artistID      AS artistID",			// 12
		"tracks.albumArtistID AS albumArtistID",	// 13
		"tracks.comment       AS comment",			// 14
		"tracks.createDate",						// 15
		"tracks.modifyDate",						// 16
		"tracks.libraryID     AS trackLibraryID"	// 17
	};

	QString select = "SELECT " + fields.join(", ") + " ";

	drop_track_view();
	create_track_view(select);

	drop_search_view();
	create_track_search_view(select);
}

Tracks::~Tracks() = default;

void Tracks::drop_track_view()
{
	if(m->library_id < 0){
		return;
	}

	run_query("DROP VIEW IF EXISTS " + m->track_view + ";", "Cannot drop " + m->track_view);
}

void Tracks::create_track_view(const QString& select_statement)
{
	if(m->library_id < 0){
		return;
	}

	QString query =	"CREATE VIEW "
					+ m->track_view + " "
					"AS " + select_statement + " "
					"FROM tracks "
					"WHERE tracks.libraryID = " + QString::number(m->library_id);

	run_query(query, "Cannot create track view");
}

void Tracks::drop_search_view()
{
	run_query("DROP VIEW IF EXISTS " + m->search_view + "; ", "Cannot drop " + m->search_view);
}

void Tracks::create_track_search_view(const QString& select_statement)
{
	QString query =
			"CREATE VIEW "
			+ m->search_view + " "
			"AS "
			+ select_statement + ", "
			"albums.name			AS albumName, "					// 18
			"albums.rating			AS albumRating, "				// 19
			"artists.name			AS artistName, "				// 20
			"albumArtists.name		AS albumArtistName, "			// 21
			"(albums.cissearch || ',' || artists.cissearch || ',' || tracks.cissearch) AS allCissearch, " // 22
			"tracks.fileCissearch	AS fileCissearch "				// 23
			"FROM tracks "
			"LEFT OUTER JOIN albums ON tracks.albumID = albums.albumID "
			"LEFT OUTER JOIN artists ON tracks.artistID = artists.artistID "
			"LEFT OUTER JOIN artists albumArtists ON tracks.albumArtistID = albumArtists.artistID "
	;

	if(m->library_id >= 0){
		query += "WHERE libraryID=" + QString::number(m->library_id);
	}

	query += ";";

	run_query(query, "Cannot create track search view");
}


QString Tracks::fetch_query_tracks() const
{
	return "SELECT * FROM " + m->search_view + " ";
}

bool Tracks::db_fetch_tracks(Query& q, MetaDataList& result) const
{
	result.clear();

	if (!q.exec()) {
		q.show_error("Cannot fetch tracks from database");
		return false;
	}

	while(q.next())
	{
		MetaData data;

		data.id = 		 	q.value(0).toInt();
		data.set_title(		q.value(1).toString());
		data.duration_ms = 	q.value(2).toInt();
		data.year = 	 	q.value(3).value<uint16_t>();
		data.bitrate = 	 	q.value(4).value<Bitrate>();
		data.set_filepath(	q.value(5).toString());
		data.filesize =  	q.value(6).value<Filesize>();
		data.track_num = 	q.value(7).value<uint16_t>();
		data.set_genres(	q.value(8).toString().split(","));
		data.discnumber = 	q.value(9).value<Disc>();
		data.rating = 		q.value(10).value<Rating>();
		data.album_id =  	q.value(11).toInt();
		data.artist_id = 	q.value(12).toInt();
		data.set_comment(	q.value(14).toString());

		data.library_id = 	q.value(17).value<LibraryId>();
		data.set_album(		q.value(18).toString().trimmed());
		data.set_artist(	q.value(20).toString().trimmed());
		data.set_album_artist(q.value(21).toString(), q.value(13).toInt());

		data.set_db_id(db_id());

		result.push_back(std::move(data));
	}

	return true;
}



bool Tracks::getMultipleTracksByPath(const QStringList& paths, MetaDataList& v_md) const
{
	db().transaction();

	for(const QString& path : paths) {
		v_md << getTrackByPath(path);
	}

	db().commit();

	return (v_md.count() == paths.size());
}


MetaData Tracks::getTrackByPath(const QString& path) const
{
	DB::Query q(this);

	QString query = fetch_query_tracks() + "WHERE filename = :filename;";
	q.prepare(query);
	q.bindValue(":filename", Util::cvt_not_null(path));

	MetaData md(path);
	md.set_db_id(db_id());

	MetaDataList v_md;
	if(!db_fetch_tracks(q, v_md)) {
		return md;
	}

	if(v_md.empty())
	{
		md.is_extern = true;
		return md;
	}

	return v_md.first();
}


MetaData Tracks::getTrackById(TrackID id) const
{
	Query q(this);
	QString query = fetch_query_tracks() +
		" WHERE trackID = :track_id; ";

	q.prepare(query);
	q.bindValue(":track_id", id);

	MetaDataList v_md;
	if(!db_fetch_tracks(q, v_md)) {
		return MetaData();
	}

	if(v_md.isEmpty()) {
		MetaData md;
		md.is_extern = true;
		return md;
	}

	return v_md.first();
}

int Tracks::getNumTracks() const
{
	DB::Query q = this->run_query(
		"SELECT COUNT(tracks.trackid) FROM tracks WHERE libraryID=:libraryID;",
		{":libraryID", m->library_id},
		"Cannot count tracks"
	);

	if(q.has_error() || !q.next()){
		return -1;
	}

	int ret = q.value(0).toInt();
	return ret;
}


bool Tracks::getTracksByIds(const QList<TrackID>& ids, MetaDataList& v_md) const
{
	QStringList queries;
	for(const TrackID& id : ids)
	{
		queries << fetch_query_tracks() + QString(" WHERE trackID = :track_id_%1").arg(id);
	}

	QString query = queries.join(" UNION ");
	query += ";";

	Query q(this);
	q.prepare(query);

	for(TrackID id : ids)
	{
		q.bindValue(QString(":track_id_%1").arg(id), id);
	}

	return db_fetch_tracks(q, v_md);
}

bool Tracks::getAllTracks(MetaDataList& result) const
{
	Query q(this);

	QString query = fetch_query_tracks() + ";";

	q.prepare(query);

	return db_fetch_tracks(q, result);
}


bool DB::Tracks::getAllTracksByAlbum(const IdList& albumsIds, MetaDataList& result) const
{
    return getAllTracksByAlbum(albumsIds, result, Filter(), -1);
}


bool Tracks::getAllTracksByAlbum(const IdList& albumIds, MetaDataList& result, const Filter& filter, int discnumber) const
{
	if(albumIds.isEmpty()) {
		return false;
	}

	QStringList filters = filter.filtertext(true);
	QStringList search_filters = filter.search_mode_filtertext(true);

	for(int i=0; i<filters.size(); i++)
	{
		Query q(this);

		QString query = fetch_query_tracks();
		query += " WHERE ";
		if( !filter.cleared() )
		{
			query += get_filter_clause(filter, "cissearch", "searchterm") + " AND ";
		}

		{ // album id clauses
			QString aidf = m->search_view + ".albumID ";
			QStringList or_clauses;
			for(int a=0; a<albumIds.size(); a++){
				or_clauses << QString("%1 = :album_id_%2").arg(aidf).arg(a);
			}

			query += " (" + or_clauses.join(" OR ") + ") ";
		}

		query += ";";

		{ // prepare & run
			q.prepare(query);

			for(int a=0; a<albumIds.size(); a++) {
				q.bindValue(QString(":album_id_%1").arg(a), albumIds[a]);
			}

			q.bindValue(":searchterm", filters[i]);
			q.bindValue(":cissearch", search_filters[i]);

			MetaDataList tmp_list;
			db_fetch_tracks(q, tmp_list);

			if(discnumber >= 0)
			{
				for(int i=tmp_list.count() - 1; i>=0; i--)
				{
					if(tmp_list[i].discnumber != discnumber){
						tmp_list.remove_track(i);
					}
				}
			}

			result.append_unique(tmp_list);
		}
	}

	return true;
}

bool Tracks::getAllTracksByArtist(const IdList& artistIds, MetaDataList& result) const
{
	return getAllTracksByArtist(artistIds, result, Filter());
}

bool Tracks::getAllTracksByArtist(const IdList& artistIds, MetaDataList& result, const Filter& filter) const
{
	if(artistIds.empty()){
		return false;
	}

	QStringList filters = filter.filtertext(true);
	QStringList search_filters = filter.search_mode_filtertext(true);

	for(int i=0; i<filters.size(); i++)
	{
		Query q(this);

		QString query = fetch_query_tracks();
		query += " WHERE ";
		if( !filter.cleared() )
		{
			query += get_filter_clause(filter, "cissearch", "searchterm") + " AND ";
		}

		{ // artist conditions
			QString aidf = m->search_view + "." + artistid_field();

			QStringList or_clauses;
			for(int a=0; a<artistIds.size(); a++) {
				or_clauses << QString("%1 = :artist_id_%2").arg(aidf).arg(a);
			}

			query += " (" + or_clauses.join(" OR ") + ") ";
		}

		query += ";";

		{ // prepare & run
			q.prepare(query);

			for(int a=0; a<artistIds.size(); a++) {
				q.bindValue(QString(":artist_id_%1").arg(a), artistIds[a]);
			}

			q.bindValue(":searchterm", filters[i]);
			q.bindValue(":cissearch", search_filters[i]);

			MetaDataList tmp_list;
			db_fetch_tracks(q, tmp_list);
			result.append_unique(tmp_list);
		}
	}

	return true;
}


bool Tracks::getAllTracksBySearchString(const Filter& filter, MetaDataList& result) const
{
	QStringList filters = filter.filtertext(true);
	QStringList search_filters = filter.search_mode_filtertext(true);
	for(int i=0; i<filters.size(); i++)
	{
		Query q(this);

		QString query = fetch_query_tracks();
		query += " WHERE " + get_filter_clause(filter, "cissearch", "searchterm");
		query += ";";

		q.prepare(query);

		q.bindValue(":searchterm", filters[i]);
		q.bindValue(":cissearch", search_filters[i]);

        {
            MetaDataList tracks;

            db_fetch_tracks(q, tracks);
            if(tracks.empty()) {
                q.show_query();
            }
            result.append_unique(tracks);
        }
	}

	return true;
}


bool Tracks::deleteTrack(TrackID id)
{
	Query q = run_query("DELETE FROM tracks WHERE trackID = :trackID", {":trackID", id}, QString("Cannot delete track %1").arg(id));

	return (!q.has_error());
}


bool Tracks::deleteTracks(const IdList& ids)
{
	int n_files = 0;

	db().transaction();

	for(const int& id : ids)
	{
		if( deleteTrack(id) )
		{
			n_files++;
		}
	}

	bool success = db().commit();

	return (success && (n_files == ids.size()));
}


bool Tracks::deleteTracks(const MetaDataList& v_md)
{
	if(v_md.isEmpty()){
		return true;
	}

	db().transaction();

	auto deleted_tracks = Util::Algorithm::count_if(v_md, [=](const MetaData& md)
	{
		return this->deleteTrack(md.id);
	});

	db().commit();

	sp_log(Log::Info, this) << "Deleted " << deleted_tracks << " of " << v_md.size() << " tracks";

	return (deleted_tracks == v_md.count());
}

bool Tracks::deleteInvalidTracks(const QString& library_path, MetaDataList& double_metadata)
{
	double_metadata.clear();

	MetaDataList v_md;
	if(!getAllTracks(v_md)){
		sp_log(Log::Error, this) << "Cannot get tracks from db";
		return false;
	}

	QMap<QString, int> map;
	IdList to_delete;
	int idx = 0;

	for(const MetaData& md : v_md)
	{
		if(map.contains(md.filepath()))
		{
			sp_log(Log::Warning, this) << "found double path: " << md.filepath();
			int old_idx = map[md.filepath()];
			to_delete << md.id;
			double_metadata << v_md[old_idx];
		}

		else {
			map.insert(md.filepath(), idx);
		}

		if( (!library_path.isEmpty()) &&
			(!md.filepath().contains(library_path)) )
		{
			to_delete << md.id;
		}

		idx++;
	}

	bool success;
	sp_log(Log::Debug, this) << "Will delete " << to_delete.size() << " double-tracks";
	success = deleteTracks(to_delete);
	sp_log(Log::Debug, this) << "delete tracks: " << success;

	success = deleteTracks(double_metadata);
	sp_log(Log::Debug, this) << "delete other tracks: " << success;

	return false;
}

Util::Set<Genre> Tracks::getAllGenres() const
{
	Query q = run_query("SELECT genre FROM " + m->track_view + " GROUP BY genre;", "Cannot fetch genres");

	if(q.has_error()){
		return Util::Set<Genre>();
	}

	Util::Set<Genre> genres;
	while(q.next())
	{
		QString genre = q.value(0).toString();
		QStringList subgenres = genre.split(",");

		for(const QString& g : subgenres){
			genres.insert( Genre(g) );
		}
	}

	sp_log(Log::Debug, this) << "Load all genres finished";
	return genres;
}


void Tracks::updateTrackCissearch()
{
	SearchableModule::update_search_mode();
	SMM sm = search_mode();

	sp_log(Log::Debug, this) << "UPdate track cissearch " << sm;

	MetaDataList v_md;
	getAllTracks(v_md);

	db().transaction();

	for(const MetaData& md : v_md)
	{
		QString cis = LibraryUtils::convert_search_string(md.title(), sm);
		QString cis_file = LibraryUtils::convert_search_string(md.filepath(), sm);

		this->update("tracks",
		{
			{"cissearch", Util::cvt_not_null(cis)},
			{"filecissearch", Util::cvt_not_null(cis_file)}
		},
		{"trackId", md.id},
		"Cannot update album cissearch"
		);
	}

	db().commit();
}


void Tracks::deleteAllTracks(bool also_views)
{
	if(m->library_id >= 0)
	{
		if(also_views)
		{
			drop_track_view();
			drop_search_view();
		}

		this->run_query
		(
			"DELETE FROM tracks WHERE libraryId=:library_id;",
			{":library_id", m->library_id},
			"Cannot delete library tracks"
		);

	}
}


bool Tracks::updateTrack(const MetaData& md)
{
	if(md.id < 0 || md.album_id < 0 || md.artist_id < 0 || md.library_id < 0)
	{
		sp_log(Log::Warning, this) << "Cannot update track (value negative): "
								   << " ArtistID: " << md.artist_id
								   << " AlbumID: " << md.album_id
								   << " TrackID: " << md.id
								   << " LibraryID: " << md.library_id;
		return false;
	}

	SMM sm = search_mode();
	QString cissearch = LibraryUtils::convert_search_string(md.title(), sm);
	QString file_cissearch = LibraryUtils::convert_search_string(md.filepath(), sm);

	QMap<QString, QVariant> bindings
	{
		{"albumArtistID",	md.album_artist_id()},
		{"albumID",			md.album_id},
		{"artistID",		md.artist_id},
		{"bitrate",			md.bitrate},
		{"cissearch",		Util::cvt_not_null(cissearch)},
		{"discnumber",		md.discnumber},
		{"filecissearch",	Util::cvt_not_null(file_cissearch)},
		{"filename",		Util::cvt_not_null(md.filepath())},
		{"filesize",		QVariant::fromValue(md.filesize)},
		{"genre",			Util::cvt_not_null(md.genres_to_string())},
		{"length",			QVariant::fromValue(md.duration_ms)},
		{"libraryID",		md.library_id},
		{"modifydate",		QVariant::fromValue(Util::current_date_to_int())},
		{"rating",			QVariant(int(md.rating))},
		{"title",			Util::cvt_not_null(md.title())},
		{"track",			md.track_num},
		{"year",			md.year},
		{"comment",			Util::cvt_not_null(md.comment())}
	};

	Query q = update("tracks", bindings, {"trackId", md.id}, QString("Cannot update track %1").arg(md.filepath()));

	return (!q.has_error());
}

bool Tracks::updateTracks(const MetaDataList& v_md)
{
	db().transaction();

	int n_files = Util::Algorithm::count_if(v_md, [=](const MetaData& md){
		return this->updateTrack(md);
	});

	bool success = db().commit();

	return success && (n_files == v_md.count());
}


bool Tracks::insertTrackIntoDatabase(const MetaData& md, ArtistId artist_id, AlbumId album_id)
{
	return insertTrackIntoDatabase(md, artist_id, album_id, artist_id);
}

bool Tracks::insertTrackIntoDatabase(const MetaData& md, ArtistId artist_id, AlbumId album_id, ArtistId album_artist_id)
{
	if(album_artist_id == -1)
	{
		album_artist_id = artist_id;
	}

	auto current_time = Util::current_date_to_int();

	QString cissearch = ::Library::Utils::convert_search_string(md.title(), search_mode());
	QString file_cissearch = ::Library::Utils::convert_search_string(md.filepath(), search_mode());

	QMap<QString, QVariant> bindings =
	{
		{"filename",		Util::cvt_not_null(md.filepath())},
		{"albumID",			album_id},
		{"artistID",		artist_id},
		{"albumArtistID",	album_artist_id},
		{"title",			Util::cvt_not_null(md.title())},
		{"year",			md.year},
		{"length",			QVariant::fromValue(md.duration_ms)},
		{"track",			md.track_num},
		{"bitrate",			md.bitrate},
		{"genre",			Util::cvt_not_null(md.genres_to_string())},
		{"filesize",		QVariant::fromValue(md.filesize)},
		{"discnumber",		md.discnumber},
		{"rating",			QVariant(int(md.rating))},
		{"comment",			Util::cvt_not_null(md.comment())},
		{"cissearch",		Util::cvt_not_null(cissearch)},
		{"filecissearch",	Util::cvt_not_null(file_cissearch)},
		{"createdate",		QVariant::fromValue(current_time)},
		{"modifydate",		QVariant::fromValue(current_time)},
		{"libraryID",		md.library_id}
	};

	Query q = insert("tracks", bindings, QString("Cannot insert track %1").arg(md.filepath()));

	return (!q.has_error());
}


static QString get_filter_clause(const Filter& filter, QString cis_placeholder, QString searchterm_placeholder)
{
	cis_placeholder.remove(":");
	searchterm_placeholder.remove(":");

	switch( filter.mode() )
	{
		case Filter::Genre:
			if(filter.is_invalid_genre())
			{
				return "genre = ''";
			}

			return "genre LIKE :" + searchterm_placeholder;

		case Filter::Filename:
			return "filecissearch LIKE :" + cis_placeholder;

		case Filter::Fulltext:
		case Filter::Invalid:
		default:
			return "allCissearch LIKE :" + cis_placeholder;
	}
}
