/* DatabaseTracks.cpp */

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

#include "Database/Tracks.h"
#include "Database/Library.h"
#include "Database/Query.h"

#include "Utils/MetaData/MetaDataList.h"
#include "Utils/MetaData/Genre.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Utils.h"
#include "Utils/FileUtils.h"
#include "Utils/Library/Filter.h"
#include "Utils/Set.h"

#include <QDateTime>
#include <QMap>

#include <utility>
#include <tuple>

using DB::Tracks;
using DB::Query;
using DB::SearchableModule;

struct Tracks::Private
{
	QString track_view;
	QString search_view;

	LibraryId library_id;

	Private(LibraryId library_id) :
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

	QString select = "SELECT "
					"trackID, "									// 0
					"title, "									// 1
					"length, "									// 2
					"year, "									// 3
					"bitrate, "									// 4
					"filename, "								// 5
					"filesize, "								// 6
					"track AS trackNum, "						// 7
					"genre, "									// 8
					"discnumber, "								// 9
					"tracks.rating, "							// 10
					"tracks.albumID AS albumID, "				// 11
					"tracks.artistID AS artistID, "				// 12
					"tracks.albumArtistID AS albumArtistID, "	// 13
					"tracks.comment AS comment, "				// 14
					"createDate, "								// 15
					"modifyDate, "								// 16
					"tracks.libraryID AS trackLibraryID "		// 17
	;

	drop_track_view();
	create_track_view(select);

	drop_search_view();
	create_track_search_view(select);
}

Tracks::~Tracks() {}


void Tracks::drop_track_view()
{
	if(m->library_id < 0){
		return;
	}

	Query q(this);
	q.prepare("DROP VIEW " + m->track_view + "; ");
	q.exec();
}

void Tracks::create_track_view(const QString& select_statement)
{
	if(m->library_id < 0){
		return;
	}

	Query q(this);
	QString query =	"CREATE VIEW "
					+ m->track_view + " "
					"AS " + select_statement + " "
					"FROM tracks "
					"WHERE tracks.libraryID = " + QString::number(m->library_id);

	q.prepare(query);
	if(!q.exec())
	{
		q.show_error("Cannot create track view");
	}
}

void Tracks::drop_search_view()
{
	Query drop_search_view(this);
	drop_search_view.prepare("DROP VIEW " + m->search_view + "; ");
	drop_search_view.exec();
}

void Tracks::create_track_search_view(const QString& select_statement)
{
	QString query =
			"CREATE VIEW "
			+ m->search_view + " "
			"AS "
			+ select_statement + ", "
			"albums.name AS albumName, "						// 18
			"albums.rating AS albumRating, "					// 19
			"artists.name AS artistName, "						// 20
			"albumArtists.name AS albumArtistName, "			// 21
			"(albums.cissearch || ',' || artists.cissearch || ',' || tracks.cissearch) AS allCissearch, " // 22
			"tracks.fileCissearch AS fileCissearch "
			"FROM tracks "
			"LEFT OUTER JOIN albums ON tracks.albumID = albums.albumID "
			"LEFT OUTER JOIN artists ON tracks.artistID = artists.artistID "
			"LEFT OUTER JOIN artists albumArtists ON tracks.albumArtistID = albumArtists.artistID "
	;

	if(m->library_id >= 0){
		query += "WHERE libraryID=" + QString::number(m->library_id);
	}

	query += ";";

	Query q(this);
	q.prepare(query);

	if(!q.exec())
	{
		q.show_error("Cannot create track search view");
	}
}


QString Tracks::fetch_query_tracks() const
{
	return "SELECT * FROM " + m->search_view + " ";
}


bool Tracks::db_fetch_tracks(Query& q, MetaDataList& result)
{
	result.clear();

	if (!q.exec()) {
		q.show_error("Cannot fetch tracks from database");
		return false;
	}

	result.reserve(q.fetched_rows());

	while(q.next())
	{
		MetaData data;

		data.id = 		 	q.value(0).toInt();
		data.set_title(		q.value(1).toString());
		data.length_ms = 	q.value(2).toInt();
		data.year = 	 	q.value(3).toInt();
		data.bitrate = 	 	q.value(4).toInt();
		data.set_filepath(	q.value(5).toString());
		data.filesize =  	q.value(6).toInt();
		data.track_num = 	q.value(7).toInt();
		data.set_genres(	q.value(8).toString().split(","));
		data.discnumber = 	q.value(9).toInt();
		data.rating = 		q.value(10).toInt();
		data.album_id =  	q.value(11).toInt();
		data.artist_id = 	q.value(12).toInt();
		data.set_comment(	q.value(14).toString());

		data.library_id = 	q.value(17).toInt();
		data.set_album(		q.value(18).toString().trimmed());
		data.set_artist(	q.value(20).toString().trimmed());
		data.set_album_artist(q.value(21).toString(), q.value(13).toInt());

		data.set_db_id(db_id());

		result.push_back(std::move(data));
	}

	return true;
}


QString Tracks::append_track_sort_string(QString querytext, ::Library::SortOrder sort)
{
	if(sort == ::Library::SortOrder::TrackArtistAsc) querytext += QString(" ORDER BY artistName ASC, discnumber ASC, albumName ASC, trackNum;");
	else if(sort == ::Library::SortOrder::TrackArtistDesc) querytext += QString(" ORDER BY artistName DESC, discnumber ASC, albumName ASC, trackNum;");
	else if(sort == ::Library::SortOrder::TrackAlbumAsc) querytext += QString(" ORDER BY discnumber ASC, albumName ASC, trackNum;");
	else if(sort == ::Library::SortOrder::TrackAlbumDesc) querytext += QString(" ORDER BY discnumber ASC, albumName DESC, trackNum;");
	else if(sort == ::Library::SortOrder::TrackTitleAsc) querytext += QString(" ORDER BY title ASC;");
	else if(sort == ::Library::SortOrder::TrackTitleDesc) querytext += QString(" ORDER BY title DESC;");
	else if(sort == ::Library::SortOrder::TrackNumAsc) querytext += QString(" ORDER BY trackNum ASC;");
	else if(sort == ::Library::SortOrder::TrackNumDesc) querytext += QString(" ORDER BY trackNum DESC;");
	else if(sort == ::Library::SortOrder::TrackYearAsc) querytext += QString(" ORDER BY year ASC;");
	else if(sort == ::Library::SortOrder::TrackYearDesc) querytext += QString(" ORDER BY year DESC;");
	else if(sort == ::Library::SortOrder::TrackLenghtAsc) querytext += QString(" ORDER BY length ASC;");
	else if(sort == ::Library::SortOrder::TrackLengthDesc) querytext += QString(" ORDER BY length DESC;");
	else if(sort == ::Library::SortOrder::TrackBitrateAsc) querytext += QString(" ORDER BY bitrate ASC;");
	else if(sort == ::Library::SortOrder::TrackBitrateDesc) querytext += QString(" ORDER BY bitrate DESC;");
	else if(sort == ::Library::SortOrder::TrackSizeAsc) querytext += QString(" ORDER BY filesize ASC;");
	else if(sort == ::Library::SortOrder::TrackSizeDesc) querytext += QString(" ORDER BY filesize DESC;");
	else if(sort == ::Library::SortOrder::TrackRatingAsc) querytext += QString(" ORDER BY rating ASC;");
	else if(sort == ::Library::SortOrder::TrackRatingDesc) querytext += QString(" ORDER BY rating DESC;");

	else querytext += ";";

	return querytext;
}


bool Tracks::getMultipleTracksByPath(const QStringList& paths, MetaDataList& v_md)
{
	db().transaction();

	for(const QString& path : paths) {
		v_md << getTrackByPath(path);
	}

	db().commit();

	return (v_md.count() == paths.size());
}


MetaData Tracks::getTrackByPath(const QString& path)
{
	DB::Query q(this);

	QString querytext = fetch_query_tracks() + "WHERE filename = :filename;";
	q.prepare(querytext);
	q.bindValue(":filename", Util::cvt_not_null(path));

	MetaData md(path);
	md.set_db_id(db_id());

	MetaDataList v_md;
	if(!db_fetch_tracks(q, v_md)) {
		return md;
	}

	if(v_md.size() == 0)
	{
		md.is_extern = true;
		return md;
	}

	return v_md.first();
}


MetaData Tracks::getTrackById(TrackID id)
{
	Query q(this);
	QString querytext = fetch_query_tracks() + "WHERE trackID = :track_id;";

	q.prepare(querytext);
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


bool Tracks::getAllTracks(MetaDataList& returndata, ::Library::SortOrder sort)
{
	Query q(this);

	QString querytext = fetch_query_tracks();
	querytext = append_track_sort_string(querytext, sort);

	q.prepare(querytext);

	return db_fetch_tracks(q, returndata);
}


bool Tracks::getAllTracksByAlbum(AlbumId album, MetaDataList& result)
{
	return getAllTracksByAlbum(album, result, ::Library::Filter());
}


bool Tracks::getAllTracksByAlbum(AlbumId album, MetaDataList& returndata, const ::Library::Filter& filter, ::Library::SortOrder sort, int discnumber)
{
	MetaDataList v_md;

	IdList list{album};
	returndata.clear();

	bool success = getAllTracksByAlbum(list, v_md, filter, sort);

	if(discnumber < 0) {
		returndata = v_md;
	}

	for(const MetaData& md : v_md)
	{
		if(discnumber != md.discnumber) {
			continue;
		}

		returndata << std::move(md);
	}

	return success;
}


bool Tracks::getAllTracksByAlbum(IdList albums, MetaDataList& result)
{
	return getAllTracksByAlbum(albums, result, ::Library::Filter());
}


bool Tracks::getAllTracksByAlbum(IdList albums, MetaDataList& returndata, const ::Library::Filter& filter, ::Library::SortOrder sort)
{
	if(albums.isEmpty()) {
		return false;
	}

	Query q(this);

	QString querytext = fetch_query_tracks();

	if( !filter.cleared() )
	{
		switch( filter.mode() )
		{
			case ::Library::Filter::Genre:
				querytext += "WHERE genre LIKE :searchterm AND ";
				break;

			case ::Library::Filter::Filename:
				querytext += "WHERE filecissearch LIKE :cissearch AND ";
				break;

			case ::Library::Filter::Fulltext:
			default:
				querytext += "WHERE allCissearch LIKE :cissearch AND ";
				break;
		}
	}

	else{
		querytext += " WHERE ";
	}

	if(albums.size() > 0) {
		QString album_id_field = m->search_view + ".albumID ";
		querytext += " (" + album_id_field + "=:albumid_0 ";
		for(int i=1; i<albums.size(); i++) {
			querytext += "OR " + album_id_field + "=:albumid_" + QString::number(i) + " ";
		}

		querytext += ") ";
	}


	querytext = append_track_sort_string(querytext, sort);

	q.prepare(querytext);

	for(int i=0; i<albums.size(); i++)
	{
		q.bindValue(QString(":albumid_%1").arg(i), albums[i]);
	}

	if( !filter.cleared() )
	{
		q.bindValue(":searchterm",	Util::cvt_not_null(filter.filtertext(true)));
		q.bindValue(":cissearch",	Util::cvt_not_null(filter.search_mode_filtertext(true)));
	}

	return db_fetch_tracks(q, returndata);
}

bool Tracks::getAllTracksByArtist(ArtistId artist, MetaDataList& returndata)
{
	return getAllTracksByArtist(artist, returndata, ::Library::Filter());
}

bool Tracks::getAllTracksByArtist(ArtistId artist, MetaDataList& returndata, const ::Library::Filter& filter, ::Library::SortOrder sort)
{
	IdList list{artist};

	return getAllTracksByArtist(list, returndata, filter, sort);
}

bool Tracks::getAllTracksByArtist(IdList artists, MetaDataList& returndata)
{
	return 	getAllTracksByArtist(artists, returndata, ::Library::Filter());
}


bool Tracks::getAllTracksByArtist(IdList artists, MetaDataList& returndata, const ::Library::Filter& filter, ::Library::SortOrder sort)
{
	if(artists.empty()){
		return false;
	}

	Query q(this);

	QString querytext = fetch_query_tracks();
	if( !filter.cleared() )
	{
		switch( filter.mode() )
		{
			case ::Library::Filter::Genre:
				querytext += "WHERE genre LIKE :searchterm AND ";
				break;

			case ::Library::Filter::Filename:
				querytext += "WHERE filecissearch LIKE :cissearch AND ";
				break;

			case ::Library::Filter::Fulltext:
			default:
				querytext += "WHERE allCissearch LIKE :cissearch AND ";
				break;
		}
	}

	else{
		querytext += " WHERE ";
	}

	if(artists.size() > 0)
	{
		QString artist_id_field = m->search_view + "." + artistid_field();
		querytext += " (" + artist_id_field + "=:artist_id_0 ";
		for(int i=1; i<artists.size(); i++) {
			querytext += "OR " + artist_id_field + "=:artist_id_" + QString::number(i) + " ";
		}

		querytext += ") ";
	}

	querytext = append_track_sort_string(querytext, sort);

	q.prepare(querytext);
	q.bindValue(":artist_id", artists.first());

	for(int i=0; i<artists.size(); i++) {
		q.bindValue(QString(":artist_id_%1").arg(i), artists[i]);
	}

	q.bindValue(":searchterm", Util::cvt_not_null(filter.filtertext(true)));
	q.bindValue(":cissearch",  Util::cvt_not_null(filter.search_mode_filtertext(true)));

	return db_fetch_tracks(q, returndata);
}

bool Tracks::getAllTracksBySearchString(const ::Library::Filter& filter, MetaDataList& result, ::Library::SortOrder sort)
{
	Query q(this);

	QString querytext = fetch_query_tracks();

	switch(filter.mode())
	{
		case ::Library::Filter::Genre:
			querytext += "WHERE genre LIKE :searchterm ";
			break;

		case ::Library::Filter::Filename:
			querytext += "WHERE filecissearch LIKE :cissearch ";
			break;

		case ::Library::Filter::Fulltext:
			querytext += "WHERE allCissearch LIKE :cissearch ";
			break;

		default:
			return false;
	}

	querytext = append_track_sort_string(querytext, sort);
	q.prepare(querytext);

	q.bindValue(":searchterm",	Util::cvt_not_null(filter.filtertext(true)));
	q.bindValue(":cissearch",	Util::cvt_not_null(filter.search_mode_filtertext(true)));

	return db_fetch_tracks(q, result);
}



bool Tracks::deleteTrack(TrackID id)
{
	Query q(this);
	QString querytext = QString("DELETE FROM tracks WHERE trackID = :track_id;");

	q.prepare(querytext);
	q.bindValue(":track_id", id);

	if (!q.exec()) {
		q.show_error(QString("Cannot delete track") + QString::number(id));
		return false;
	}

	return true;
}


bool Tracks::deleteTracks(const IdList& ids)
{
	int n_files = 0;

	db().transaction();

	for(const int& id : ids){
		if( deleteTrack(id) ){
			n_files++;
		};
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

	size_t deleted_tracks = std::count_if(v_md.begin(), v_md.end(), [=](const MetaData& md)
	{
		return this->deleteTrack(md.id);
	});

	db().commit();

	sp_log(Log::Info) << "Deleted " << deleted_tracks << " of " << v_md.size() << " tracks";

	return (deleted_tracks == v_md.size());
}

bool Tracks::deleteInvalidTracks(const QString& library_path, MetaDataList& double_metadata)
{
	double_metadata.clear();

	MetaDataList v_md;
	if(!getAllTracks(v_md)){
		sp_log(Log::Error) << "Cannot get tracks from db";
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

SP::Set<Genre> Tracks::getAllGenres()
{
	SP::Set<Genre> genres;
	sp_log(Log::Debug, this) << "Load all genres";

	Query q(this);
	q.prepare("SELECT genre FROM " + m->track_view + " GROUP BY genre;");

	bool success = q.exec();
	if(!success){
		return genres;
	}

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

	sp_log(Log::Debug, this) << "UPdate track cissearch " << search_mode();

	MetaDataList v_md;
	getAllTracks(v_md);

	db().transaction();

	for(const MetaData& md : v_md)
	{
		QString querystring = "UPDATE tracks SET cissearch=:cissearch, filecissearch=:filecissearch WHERE trackID=:id;";
		Query q(this);
		q.prepare(querystring);

		QString cis = ::Library::Util::convert_search_string(md.title(), search_mode());
		QString cis_file = ::Library::Util::convert_search_string(md.filepath(), search_mode());

		q.bindValue(":cissearch",		Util::cvt_not_null(cis));
		q.bindValue(":filecissearch",	Util::cvt_not_null(cis_file));
		q.bindValue(":id", md.id);

		if(!q.exec()){
			q.show_error("Cannot update album cissearch");
		}
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

		Query q2(this);
		q2.prepare("DELETE FROM tracks WHERE libraryId=:library_id;");
		q2.bindValue(":library_id", m->library_id);
		q2.exec();
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

	Query q(this);

	QString cissearch = ::Library::Util::convert_search_string(md.title(), search_mode());
	QString file_cissearch = ::Library::Util::convert_search_string(md.filepath(), search_mode());

	q.prepare("UPDATE tracks "
			  "SET "
			  "albumArtistID=:albumArtistID, "
			  "albumID=:albumID, "
			  "artistID=:artistID, "
			  "bitrate=:bitrate, "
			  "cissearch=:cissearch, "
			  "discnumber=:discnumber, "
			  "filecissearch=:filecissearch, "
			  "filename=:filename, "
			  "filesize=:filesize, "
			  "genre=:genre, "
			  "length=:length, "
			  "libraryID=:libraryID, "
			  "modifydate=:modifydate, "
			  "rating=:rating, "
			  "title=:title, "
			  "track=:track, "
			  "year=:year, "
			  "comment=:comment "
			  "WHERE TrackID = :trackID;");

	q.bindValue(":albumArtistID",	md.album_artist_id());
	q.bindValue(":albumID",			md.album_id);
	q.bindValue(":artistID",		md.artist_id);
	q.bindValue(":bitrate",			md.bitrate);
	q.bindValue(":cissearch",		Util::cvt_not_null(cissearch));
	q.bindValue(":discnumber",		md.discnumber);
	q.bindValue(":filecissearch",	Util::cvt_not_null(file_cissearch));
	q.bindValue(":filename",		Util::cvt_not_null(md.filepath()));
	q.bindValue(":filesize",		QVariant::fromValue(md.filesize));
	q.bindValue(":genre",			Util::cvt_not_null(md.genres_to_string()));
	q.bindValue(":length",			QVariant::fromValue(md.length_ms));
	q.bindValue(":libraryID",		md.library_id);
	q.bindValue(":modifydate",		QVariant::fromValue(Util::current_date_to_int()));
	q.bindValue(":rating",			md.rating);
	q.bindValue(":title",			Util::cvt_not_null(md.title()));
	q.bindValue(":track",			md.track_num);
	q.bindValue(":trackID",			md.id);
	q.bindValue(":year",			md.year);
	q.bindValue(":comment",			Util::cvt_not_null(md.comment()));

	if (!q.exec()) {
		q.show_error(QString("Cannot update track ") + md.filepath());
		return false;
	}

	return true;
}

bool Tracks::updateTracks(const MetaDataList& v_md)
{
	db().transaction();

	size_t n_files = std::count_if(v_md.begin(), v_md.end(), [=](const MetaData& md){
		return this->updateTrack(md);
	});

	bool success = db().commit();

	return success && (n_files == v_md.size());
}

bool Tracks::insertTrackIntoDatabase(const MetaData& md, ArtistId artist_id, AlbumId album_id)
{
	return insertTrackIntoDatabase(md, artist_id, album_id, artist_id);
}

bool Tracks::insertTrackIntoDatabase(const MetaData& md, ArtistId artist_id, AlbumId album_id, ArtistId album_artist_id)
{
	DB::Query q(this);

	MetaData md_tmp = getTrackByPath( md.filepath() );

	if( md_tmp.id >= 0 )
	{
		MetaData track_copy = md;
		track_copy.id = md_tmp.id;
		track_copy.artist_id = artist_id;
		track_copy.album_id = album_id;
		track_copy.set_album_artist_id(album_artist_id);

		return updateTrack(track_copy);
	}

	QString cissearch = ::Library::Util::convert_search_string(md.title(), search_mode());
	QString file_cissearch = ::Library::Util::convert_search_string(md.filepath(), search_mode());
	QString querytext =
			"INSERT INTO tracks "
			"(filename,  albumID, artistID, albumArtistID,  title,  year,  length,  track,  bitrate,  genre,  filesize,  discnumber,  rating,  comment,  cissearch,  filecissearch,  createdate,  modifydate,  libraryID) "
			"VALUES "
			"(:filename,:albumID,:artistID, :albumArtistID, :title, :year, :length, :track, :bitrate, :genre, :filesize, :discnumber, :rating, :comment, :cissearch, :filecissearch, :createdate, :modifydate, :libraryID); ";

	auto current_time = Util::current_date_to_int();
	q.prepare(querytext);

	q.bindValue(":filename",		Util::cvt_not_null(md.filepath()));
	q.bindValue(":albumID",			album_id);
	q.bindValue(":artistID",		artist_id);
	q.bindValue(":albumArtistID",	album_artist_id);
	q.bindValue(":title",			Util::cvt_not_null(md.title()));
	q.bindValue(":year",			md.year);
	q.bindValue(":length",			QVariant::fromValue(md.length_ms));
	q.bindValue(":track",			md.track_num);
	q.bindValue(":bitrate",			md.bitrate);
	q.bindValue(":genre",			Util::cvt_not_null(md.genres_to_string()));
	q.bindValue(":filesize",		QVariant::fromValue(md.filesize));
	q.bindValue(":discnumber",		md.discnumber);
	q.bindValue(":rating",			md.rating);
	q.bindValue(":comment",			Util::cvt_not_null(md.comment()));
	q.bindValue(":cissearch",		Util::cvt_not_null(cissearch));
	q.bindValue(":filecissearch",	Util::cvt_not_null(file_cissearch));
	q.bindValue(":createdate",		QVariant::fromValue(current_time));
	q.bindValue(":modifydate",		QVariant::fromValue(current_time));
	q.bindValue(":libraryID",		md.library_id);

	if (!q.exec()) {
		q.show_error(QString("Cannot insert track into database ") + md.filepath());
		return false;
	}

	return true;
}

