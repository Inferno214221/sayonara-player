/* DatabaseTracks.cpp */

/* Copyright (C) 2011-2020 Michael Lugmair (Lucio Carreras)
 *
 * module() file is part of sayonara player
 *
 * module() program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * module() program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with module() program.  If not, see <http://www.gnu.org/licenses/>.
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

static QString getFilterClause(const Filter& filter, QString cisPlaceholder, QString searchtermPlaceholder);

static void dropTrackView(DB::Module* module, LibraryId libraryId, const QString& trackView)
{
	if(libraryId < 0){
		return;
	}

	module->runQuery("DROP VIEW IF EXISTS " + trackView + ";", "Cannot drop " + trackView);
}

static void dropSearchView(DB::Module* module, const QString& trackSearchView)
{
	module->runQuery("DROP VIEW IF EXISTS " + trackSearchView + "; ", "Cannot drop " + trackSearchView);
}

static void createTrackView(DB::Module* module, LibraryId libraryId, const QString& trackView, const QString& selectStatement)
{
	if(libraryId < 0){
		return;
	}

	QString query =	"CREATE VIEW "
					+ trackView + " "
					"AS " + selectStatement + " "
					"FROM tracks "
					"WHERE tracks.libraryID = " + QString::number(libraryId);

	module->runQuery(query, "Cannot create track view");
}

static void createTrackSearchView(DB::Module* module, LibraryId libraryId, const QString& trackSearchView, const QString& selectStatement)
{
	QString query =
			"CREATE VIEW "
			+ trackSearchView + " "
			"AS "
			+ selectStatement + ", "
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

	if(libraryId >= 0) {
		query += "WHERE libraryID=" + QString::number(libraryId);
	}

	query += ";";

	module->runQuery(query, "Cannot create track search view");
}

Tracks::Tracks() = default;
Tracks::~Tracks() = default;

void Tracks::initViews()
{
	const QStringList fields
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

	dropTrackView(module(), libraryId(), trackView());
	createTrackView(module(), libraryId(), trackView(), select);

	dropSearchView(module(), trackSearchView());
	createTrackSearchView(module(), libraryId(), trackSearchView(), select);
}

QString Tracks::fetchQueryTracks() const
{
	return "SELECT * FROM " + trackSearchView() + " ";
}

bool Tracks::dbFetchTracks(Query& q, MetaDataList& result) const
{
	result.clear();

	if (!q.exec()) {
		q.showError("Cannot fetch tracks from database");
		return false;
	}

	while(q.next())
	{
		MetaData data;

		data.setId(	 	q.value(0).toInt());
		data.setTitle(		q.value(1).toString());
		data.setDurationMs(q.value(2).toInt());
		data.setYear(		q.value(3).value<Year>());
		data.setBitrate(	q.value(4).value<Bitrate>());
		data.setFilepath(	q.value(5).toString());
		data.setFilesize(  q.value(6).value<Filesize>());
		data.setTrackNumber( q.value(7).value<TrackNum>());
		data.setGenres(	q.value(8).toString().split(","));
		data.setDiscnumber(q.value(9).value<Disc>());
		data.setRating(    q.value(10).value<Rating>());
		data.setAlbumId(  q.value(11).toInt());
		data.setArtistId( q.value(12).toInt());
		data.setComment(	q.value(14).toString());
		data.setCreatedDate(q.value(15).value<uint64_t>());
		data.setModifiedDate(q.value(16).value<uint64_t>());
		data.setLibraryid(q.value(17).value<LibraryId>());
		data.setAlbum(		q.value(18).toString().trimmed());
		data.setArtist(	q.value(20).toString().trimmed());
		data.setAlbumArtist(q.value(21).toString(), q.value(13).toInt());

		data.setDatabaseId(module()->databaseId());

		result.push_back(std::move(data));
	}

	return true;
}


bool Tracks::getMultipleTracksByPath(const QStringList& paths, MetaDataList& tracks) const
{
	for(const QString& path : paths) {
		tracks << getTrackByPath(path);
	}

	return (tracks.count() == paths.size());
}


MetaData Tracks::getTrackByPath(const QString& path) const
{
	DB::Query q(module());

	QString query = fetchQueryTracks() + "WHERE filename = :filename;";
	q.prepare(query);
	q.bindValue(":filename", Util::convertNotNull(path));

	MetaData md(path);
	md.setDatabaseId(module()->databaseId());

	MetaDataList tracks;
	if(!dbFetchTracks(q, tracks)) {
		return md;
	}

	if(tracks.empty())
	{
		md.setExtern(true);
		return md;
	}

	return tracks.first();
}


MetaData Tracks::getTrackById(TrackID id) const
{
	Query q(module());
	QString query = fetchQueryTracks() +
		" WHERE trackID = :trackId; ";

	q.prepare(query);
	q.bindValue(":trackId", id);

	MetaDataList tracks;
	if(!dbFetchTracks(q, tracks)) {
		return MetaData();
	}

	if(tracks.isEmpty()) {
		MetaData md;
		md.setExtern(true);
		return md;
	}

	return tracks.first();
}

int Tracks::getNumTracks() const
{
	DB::Query q = module()->runQuery(
		"SELECT COUNT(tracks.trackid) FROM tracks WHERE libraryID=:libraryID;",
		{":libraryID", libraryId()},
		"Cannot count tracks"
	);

	if(q.hasError() || !q.next()){
		return -1;
	}

	int ret = q.value(0).toInt();
	return ret;
}


bool Tracks::getTracksByIds(const QList<TrackID>& ids, MetaDataList& tracks) const
{
	QStringList queries;
	for(const TrackID& id : ids)
	{
		queries << fetchQueryTracks() + QString(" WHERE trackID = :trackId_%1").arg(id);
	}

	QString query = queries.join(" UNION ");
	query += ";";

	Query q(module());
	q.prepare(query);

	for(TrackID id : ids)
	{
		q.bindValue(QString(":trackId_%1").arg(id), id);
	}

	return dbFetchTracks(q, tracks);
}

bool Tracks::getAllTracks(MetaDataList& result) const
{
	Query q(module());

	QString query = fetchQueryTracks() + ";";

	q.prepare(query);

	return dbFetchTracks(q, result);
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
	QStringList search_filters = filter.searchModeFiltertext(true);

	for(int i=0; i<filters.size(); i++)
	{
		Query q(module());

		QString query = fetchQueryTracks();
		query += " WHERE ";
		if( !filter.cleared() )
		{
			query += getFilterClause(filter, "cissearch", "searchterm") + " AND ";
		}

		{ // album id clauses
			QString aidf = trackSearchView() + ".albumID ";
			QStringList or_clauses;
			for(int a=0; a<albumIds.size(); a++){
				or_clauses << QString("%1 = :albumId_%2").arg(aidf).arg(a);
			}

			query += " (" + or_clauses.join(" OR ") + ") ";
		}

		query += ";";

		{ // prepare & run
			q.prepare(query);

			for(int a=0; a<albumIds.size(); a++) {
				q.bindValue(QString(":albumId_%1").arg(a), albumIds[a]);
			}

			q.bindValue(":searchterm", filters[i]);
			q.bindValue(":cissearch", search_filters[i]);

			MetaDataList tmp_list;
			dbFetchTracks(q, tmp_list);

			if(discnumber >= 0)
			{
				for(int i=tmp_list.count() - 1; i>=0; i--)
				{
					if(tmp_list[i].discnumber() != discnumber){
						tmp_list.removeTrack(i);
					}
				}
			}

			result.appendUnique(tmp_list);
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
	QStringList search_filters = filter.searchModeFiltertext(true);

	for(int i=0; i<filters.size(); i++)
	{
		Query q(module());

		QString query = fetchQueryTracks();
		query += " WHERE ";
		if( !filter.cleared() )
		{
			query += getFilterClause(filter, "cissearch", "searchterm") + " AND ";
		}

		{ // artist conditions
			QString aidf = trackSearchView() + "." + artistIdField();

			QStringList or_clauses;
			for(int a=0; a<artistIds.size(); a++) {
				or_clauses << QString("%1 = :artistId_%2").arg(aidf).arg(a);
			}

			query += " (" + or_clauses.join(" OR ") + ") ";
		}

		query += ";";

		{ // prepare & run
			q.prepare(query);

			for(int a=0; a<artistIds.size(); a++) {
				q.bindValue(QString(":artistId_%1").arg(a), artistIds[a]);
			}

			q.bindValue(":searchterm", filters[i]);
			q.bindValue(":cissearch", search_filters[i]);

			MetaDataList tmp_list;
			dbFetchTracks(q, tmp_list);
			result.appendUnique(tmp_list);
		}
	}

	return true;
}


bool Tracks::getAllTracksBySearchString(const Filter& filter, MetaDataList& result) const
{
	QStringList filters = filter.filtertext(true);
	QStringList search_filters = filter.searchModeFiltertext(true);
	for(int i=0; i<filters.size(); i++)
	{
		Query q(module());

		QString query = fetchQueryTracks();
		query += " WHERE " + getFilterClause(filter, "cissearch", "searchterm");
		query += ";";

		q.prepare(query);

		q.bindValue(":searchterm", filters[i]);
		q.bindValue(":cissearch", search_filters[i]);

        {
            MetaDataList tracks;

            dbFetchTracks(q, tracks);
            if(tracks.empty()) {
                q.showQuery();
            }
            result.appendUnique(tracks);
        }
	}

	return true;
}

bool Tracks::getAllTracksByPaths(const QStringList& paths, MetaDataList& tracks) const
{
	QStringList queries;
	for(int i=0; i<paths.size(); i++)
	{
		queries << fetchQueryTracks() + " WHERE filename LIKE :" + QString("path%1").arg(i);
	}

	QString query = queries.join(" UNION ") + ";";
	Query q(module());
	q.prepare(query);
	for(int i=0; i<paths.size(); i++)
	{
		q.bindValue( QString(":path%1").arg(i), paths[i] + "%");
	}

	bool success = dbFetchTracks(q, tracks);
	return success;
}


bool Tracks::deleteTrack(TrackID id)
{
	Query q = module()->runQuery("DELETE FROM tracks WHERE trackID = :trackID", {":trackID", id}, QString("Cannot delete track %1").arg(id));

	return (!q.hasError());
}


bool Tracks::deleteTracks(const IdList& ids)
{
	module()->db().transaction();

	int fileCount = Util::Algorithm::count(ids, [this](Id id)
	{
		return deleteTrack(id);
	});

	bool success = module()->db().commit();

	return (success && (fileCount == ids.size()));
}

bool Tracks::deleteTracks(const MetaDataList& tracks)
{
	if(tracks.isEmpty()){
		return true;
	}

	module()->db().transaction();

	auto deletedTracks = Util::Algorithm::count(tracks, [this](const MetaData& md)
	{
		return deleteTrack(md.id());
	});

	module()->db().commit();

	spLog(Log::Info, this) << "Deleted " << deletedTracks << " of " << tracks.size() << " tracks";

	return (deletedTracks == tracks.count());
}

bool Tracks::deleteInvalidTracks(const QString& libraryPath, MetaDataList& doubleMetadata)
{
	doubleMetadata.clear();

	MetaDataList tracks;
	if(!getAllTracks(tracks))
	{
		spLog(Log::Error, this) << "Cannot get tracks from db";
		return false;
	}

	QMap<QString, int> map;
	IdList toDelete;
	int idx = 0;

	for(const MetaData& md : tracks)
	{
		if(map.contains(md.filepath()))
		{
			spLog(Log::Warning, this) << "found double path: " << md.filepath();
			int old_idx = map[md.filepath()];
			toDelete << md.id();
			doubleMetadata << tracks[old_idx];
		}

		else {
			map.insert(md.filepath(), idx);
		}

		if( (!libraryPath.isEmpty()) &&
			(!md.filepath().contains(libraryPath)) )
		{
			toDelete << md.id();
		}

		idx++;
	}

	bool success;
	spLog(Log::Debug, this) << "Will delete " << toDelete.size() << " double-tracks";
	success = deleteTracks(toDelete);
	spLog(Log::Debug, this) << "delete tracks: " << success;

	success = deleteTracks(doubleMetadata);
	spLog(Log::Debug, this) << "delete other tracks: " << success;

	return false;
}

Util::Set<Genre> Tracks::getAllGenres() const
{
	Query q = module()->runQuery("SELECT genre FROM " + trackView() + " GROUP BY genre;", "Cannot fetch genres");

	if(q.hasError()){
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

	spLog(Log::Debug, this) << "Load all genres finished";
	return genres;
}


void Tracks::updateTrackCissearch()
{
	SMM sm = searchMode();

	spLog(Log::Debug, this) << "UPdate track cissearch " << sm;

	MetaDataList tracks;
	getAllTracks(tracks);

	module()->db().transaction();

	for(const MetaData& md : tracks)
	{
		QString cis = LibraryUtils::convertSearchstring(md.title(), sm);
		QString cis_file = LibraryUtils::convertSearchstring(md.filepath(), sm);

		module()->update("tracks",
		{
			{"cissearch", Util::convertNotNull(cis)},
			{"filecissearch", Util::convertNotNull(cis_file)}
		},
		{"trackId", md.id()},
		"Cannot update album cissearch"
		);
	}

	module()->db().commit();
}


void Tracks::deleteAllTracks(bool also_views)
{
	if(libraryId() >= 0)
	{
		if(also_views)
		{
			dropTrackView(module(), libraryId(), trackView());
			dropSearchView(module(), trackSearchView());
		}

		module()->runQuery
		(
			"DELETE FROM tracks WHERE libraryId=:libraryId;",
			{":libraryId", libraryId()},
			"Cannot delete library tracks"
		);

	}
}


bool Tracks::updateTrack(const MetaData& md)
{
	if(md.id() < 0 || md.albumId() < 0 || md.artistId() < 0 || md.libraryId() < 0)
	{
		spLog(Log::Warning, this) << "Cannot update track (value negative): "
								   << " ArtistID: " << md.artistId()
								   << " AlbumID: " << md.albumId()
								   << " TrackID: " << md.id()
								   << " LibraryID: " << md.libraryId();
		return false;
	}

	SMM sm = searchMode();
	QString cissearch = LibraryUtils::convertSearchstring(md.title(), sm);
	QString fileCissearch = LibraryUtils::convertSearchstring(md.filepath(), sm);

	QMap<QString, QVariant> bindings
	{
		{"albumArtistID",	md.albumArtistId()},
		{"albumID",			md.albumId()},
		{"artistID",		md.artistId()},
		{"bitrate",			md.bitrate()},
		{"cissearch",		Util::convertNotNull(cissearch)},
		{"discnumber",		md.discnumber()},
		{"filecissearch",	Util::convertNotNull(fileCissearch)},
		{"filename",		Util::convertNotNull(md.filepath())},
		{"filesize",		QVariant::fromValue(md.filesize())},
		{"genre",			Util::convertNotNull(md.genresToString())},
		{"length",			QVariant::fromValue(md.durationMs())},
		{"libraryID",		md.libraryId()},
		{"modifydate",		QVariant::fromValue(Util::currentDateToInt())},
		{"rating",			QVariant(int(md.rating()))},
		{"title",			Util::convertNotNull(md.title())},
		{"track",			md.trackNumber()},
		{"year",			md.year()},
		{"comment",			Util::convertNotNull(md.comment())}
	};

	Query q = module()->update("tracks", bindings, {"trackId", md.id()}, QString("Cannot update track %1").arg(md.filepath()));

	return (!q.hasError());
}

bool Tracks::updateTracks(const MetaDataList& tracks)
{
	module()->db().transaction();

	int fileCount = Util::Algorithm::count(tracks, [=](const MetaData& md){
		return updateTrack(md);
	});

	bool success = module()->db().commit();

	return success && (fileCount == tracks.count());
}


bool Tracks::renameFilepaths(const QMap<QString, QString>& paths, LibraryId targetLibrary)
{
	module()->db().transaction();

	const QStringList originalPaths(paths.keys());
	for(const QString& originalPath : originalPaths)
	{
		QString newPath = paths[originalPath];

		MetaDataList tracks;
		getAllTracksByPaths({originalPath}, tracks);

		for(const MetaData& md : tracks)
		{
			QString oldFilepath = md.filepath();
			QString newFilepath(oldFilepath);

			newFilepath.replace(originalPath, newPath);
			renameFilepath(oldFilepath, newFilepath, targetLibrary);
		}
	}

	return module()->db().commit();
}

bool Tracks::renameFilepath(const QString& oldPath, const QString& newPath, LibraryId targetLibrary)
{
	Query q = module()->update
	(
		"Tracks",
		{
			{"filename", newPath},
			{"libraryID", targetLibrary}
		},

		{"filename", oldPath},
		"Could not rename Filepath"
	);

	return (!q.hasError());
}


bool Tracks::insertTrackIntoDatabase(const MetaData& md, ArtistId artistId, AlbumId albumId)
{
	return insertTrackIntoDatabase(md, artistId, albumId, artistId);
}

bool Tracks::insertTrackIntoDatabase(const MetaData& md, ArtistId artistId, AlbumId albumId, ArtistId albumArtistId)
{
	if(albumArtistId == -1)
	{
		albumArtistId = artistId;
	}

	uint64_t currentTime = Util::currentDateToInt();
	uint64_t modifiedTime = md.modifiedDate();
	uint64_t createdTime = md.createdDate();

	if(!md.createdDateTime().isValid()){
		createdTime = currentTime;
	}

	if(!md.modifiedDateTime().isValid()){
		modifiedTime = currentTime;
	}

	QString cissearch = ::Library::Utils::convertSearchstring(md.title(), searchMode());
	QString fileCissearch = ::Library::Utils::convertSearchstring(md.filepath(), searchMode());

	QMap<QString, QVariant> bindings =
	{
		{"filename",		Util::convertNotNull(md.filepath())},
		{"albumID",			albumId},
		{"artistID",		artistId},
		{"albumArtistID",	albumArtistId},
		{"title",			Util::convertNotNull(md.title())},
		{"year",			md.year()},
		{"length",			QVariant::fromValue(md.durationMs())},
		{"track",			md.trackNumber()},
		{"bitrate",			md.bitrate()},
		{"genre",			Util::convertNotNull(md.genresToString())},
		{"filesize",		QVariant::fromValue(md.filesize())},
		{"discnumber",		md.discnumber()},
		{"rating",			QVariant(int(md.rating()))},
		{"comment",			Util::convertNotNull(md.comment())},
		{"cissearch",		Util::convertNotNull(cissearch)},
		{"filecissearch",	Util::convertNotNull(fileCissearch)},
		{"createdate",		QVariant::fromValue(createdTime)},
		{"modifydate",		QVariant::fromValue(modifiedTime)},
		{"libraryID",		md.libraryId()}
	};

	Query q = module()->insert("tracks", bindings, QString("Cannot insert track %1").arg(md.filepath()));

	return (!q.hasError());
}


static QString getFilterClause(const Filter& filter, QString cisPlaceholder, QString searchtermPlaceholder)
{
	cisPlaceholder.remove(":");
	searchtermPlaceholder.remove(":");

	switch( filter.mode() )
	{
		case Filter::Genre:
			if(filter.isInvalidGenre())
			{
				return "genre = ''";
			}

			return "genre LIKE :" + searchtermPlaceholder;

		case Filter::Filename:
			return "filecissearch LIKE :" + cisPlaceholder;

		case Filter::Fulltext:
		case Filter::Invalid:
		default:
			return "allCissearch LIKE :" + cisPlaceholder;
	}
}
