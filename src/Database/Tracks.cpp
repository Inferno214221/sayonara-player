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
#include "Database/Utils.h"

#include "Utils/MetaData/MetaDataList.h"
#include "Utils/MetaData/Genre.h"

#include "Utils/Algorithm.h"
#include "Utils/Utils.h"
#include "Utils/FileUtils.h"
#include "Utils/Ranges.h"
#include "Utils/Set.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Library/Filter.h"
#include "Utils/Library/SearchMode.h"
#include "Utils/Logger/Logger.h"

#include <QDateTime>
#include <QMap>

#include <utility>
#include <tuple>

using DB::Tracks;
using DB::Query;
using ::Library::Filter;

namespace
{
	constexpr const auto* CisPlaceholder = ":cissearch";

	void dropTrackView(DB::Module* module, LibraryId libraryId, const QString& trackView)
	{
		if(libraryId >= 0)
		{
			module->runQuery(QString("DROP VIEW IF EXISTS %1;").arg(trackView),
			                 QString("Cannot drop %1").arg(trackView));
		}
	}

	void dropTrackSearchView(DB::Module* module, const QString& trackSearchView)
	{
		module->runQuery(QString("DROP VIEW IF EXISTS %1;").arg(trackSearchView),
		                 QString("Cannot drop %1").arg(trackSearchView));
	}

	void
	createTrackView(DB::Module* module, LibraryId libraryId, const QString& trackView, const QString& selectStatement)
	{
		if(libraryId >= 0)
		{
			const auto query = QString("CREATE VIEW %1 AS %2 FROM tracks WHERE tracks.libraryID = %3")
				.arg(trackView)
				.arg(selectStatement)
				.arg(QString::number(libraryId));

			module->runQuery(query, "Cannot create track view");
		}
	}

	void createTrackSearchView(DB::Module* module, LibraryId libraryId, const QString& trackSearchView,
	                           const QString& selectStatement)
	{
		static const auto additionalFields = QStringList {
			QStringLiteral("albums.name           AS albumName"),                 // 18
			QStringLiteral("albums.rating         AS albumRating"),               // 19
			QStringLiteral("artists.name          AS artistName"),                // 20
			QStringLiteral("albumArtists.name     AS albumArtistName"),           // 21
			QStringLiteral("(albums.cissearch || ',' || artists.cissearch || ',' || tracks.cissearch) AS allCissearch"), // 22
			QStringLiteral("tracks.fileCissearch  AS fileCissearch"),             // 23
			QStringLiteral("tracks.genreCissearch AS genreCissearch")             // 24
		};

		static const auto joinedAdditionalFields = additionalFields.join(", ");

		const auto joinStatement = QStringLiteral("LEFT OUTER JOIN albums ON tracks.albumID = albums.albumID "
		                                          "LEFT OUTER JOIN artists ON tracks.artistID = artists.artistID "
		                                          "LEFT OUTER JOIN artists albumArtists ON tracks.albumArtistID = albumArtists.artistID");

		const auto whereStatement = (libraryId >= 0)
		                            ? QString("libraryID=%1").arg(QString::number(libraryId))
		                            : QString("1");

		auto query = QString("CREATE VIEW %1 AS %2, %3 FROM tracks %4 WHERE %5;")
			.arg(trackSearchView)
			.arg(selectStatement)
			.arg(joinedAdditionalFields)
			.arg(joinStatement)
			.arg(whereStatement);

		module->runQuery(query, "Cannot create track search view");
	}

	QMap<QString, QVariant>
	getTrackBindings(const MetaData& track, ArtistId artistId, AlbumId albumId, ArtistId albumArtistId)
	{
		return QMap<QString, QVariant> {
			{QStringLiteral("albumArtistID"),  albumArtistId},
			{QStringLiteral("albumID"),        albumId},
			{QStringLiteral("artistID"),       artistId},
			{QStringLiteral("bitrate"),        track.bitrate()},
			{QStringLiteral("cissearch"),      Library::convertSearchstring(track.title())},
			{QStringLiteral("comment"),        Util::convertNotNull(track.comment())},
			{QStringLiteral("discnumber"),     track.discnumber()},
			{QStringLiteral("filecissearch"),  Library::convertSearchstring(track.filepath())},
			{QStringLiteral("filename"),       Util::File::cleanFilename(track.filepath())},
			{QStringLiteral("filesize"),       QVariant::fromValue(track.filesize())},
			{QStringLiteral("genre"),          Util::convertNotNull(track.genresToString())},
			{QStringLiteral("genreCissearch"), Library::convertSearchstring(track.genresToString())},
			{QStringLiteral("length"),         QVariant::fromValue(track.durationMs())},
			{QStringLiteral("libraryID"),      track.libraryId()},
			{QStringLiteral("rating"),         QVariant(static_cast<int>(track.rating()))},
			{QStringLiteral("title"),          Util::convertNotNull(track.title())},
			{QStringLiteral("track"),          track.trackNumber()},
			{QStringLiteral("year"),           track.year()}};
	}
} // namespace

Tracks::Tracks() = default;
Tracks::~Tracks() = default;

void Tracks::initViews()
{
	static const auto fields = QStringList
		{
			QStringLiteral("tracks.trackID"),                           // 0
			QStringLiteral("tracks.title"),                             // 1
			QStringLiteral("tracks.length"),                            // 2
			QStringLiteral("tracks.year"),                              // 3
			QStringLiteral("tracks.bitrate"),                           // 4
			QStringLiteral("tracks.filename"),                          // 5
			QStringLiteral("tracks.filesize"),                          // 6
			QStringLiteral("tracks.track         AS trackNum"),         // 7
			QStringLiteral("tracks.genre"),                             // 8
			QStringLiteral("tracks.discnumber"),                        // 9
			QStringLiteral("tracks.rating"),                            // 10
			QStringLiteral("tracks.albumID	     AS albumID"),          // 11
			QStringLiteral("tracks.artistID      AS artistID"),         // 12
			QStringLiteral("tracks.albumArtistID AS albumArtistID"),    // 13
			QStringLiteral("tracks.comment       AS comment"),          // 14
			QStringLiteral("tracks.createDate"),                        // 15
			QStringLiteral("tracks.modifyDate"),                        // 16
			QStringLiteral("tracks.libraryID     AS trackLibraryID")    // 17
		};

	static const auto joinedFields = fields.join(", ");
	const auto selectStatement = QString("SELECT %1 ").arg(joinedFields);

	dropTrackView(module(), libraryId(), trackView());
	createTrackView(module(), libraryId(), trackView(), selectStatement);

	dropTrackSearchView(module(), trackSearchView());
	createTrackSearchView(module(), libraryId(), trackSearchView(), selectStatement);
}

QString Tracks::fetchQueryTracks(const QString& where) const
{
	return QString("SELECT * FROM %1 WHERE %2;")
		.arg(trackSearchView())
		.arg(where.isEmpty() ? "1" : where);
}

bool Tracks::dbFetchTracks(Query& q, MetaDataList& result) const
{
	result.clear();

	if(!q.exec())
	{
		showError(q, "Cannot fetch tracks from database");
		return false;
	}

	while(q.next())
	{
		MetaData track;

		track.setId(q.value(0).toInt());
		track.setTitle(q.value(1).toString());
		track.setDurationMs(q.value(2).toInt());
		track.setYear(q.value(3).value<Year>());
		track.setBitrate(q.value(4).value<Bitrate>());
		track.setFilepath(q.value(5).toString()); // NOLINT(readability-magic-numbers)
		track.setFilesize(q.value(6).value<Filesize>()); // NOLINT(readability-magic-numbers)
		track.setTrackNumber(q.value(7).value<TrackNum>()); // NOLINT(readability-magic-numbers)
		track.setGenres(q.value(8).toString().split(",")); // NOLINT(readability-magic-numbers)
		track.setDiscnumber(q.value(9).value<Disc>()); // NOLINT(readability-magic-numbers)
		track.setRating(q.value(10).value<Rating>()); // NOLINT(readability-magic-numbers)
		track.setAlbumId(q.value(11).toInt()); // NOLINT(readability-magic-numbers)
		track.setArtistId(q.value(12).toInt()); // NOLINT(readability-magic-numbers)
		track.setComment(q.value(14).toString()); // NOLINT(readability-magic-numbers)
		track.setCreatedDate(q.value(15).value<uint64_t>()); // NOLINT(readability-magic-numbers)
		track.setModifiedDate(q.value(16).value<uint64_t>()); // NOLINT(readability-magic-numbers)
		track.setLibraryid(q.value(17).value<LibraryId>()); // NOLINT(readability-magic-numbers)
		track.setAlbum(q.value(18).toString().trimmed()); // NOLINT(readability-magic-numbers)
		track.setArtist(q.value(20).toString().trimmed()); // NOLINT(readability-magic-numbers)
		track.setAlbumArtist(q.value(21).toString(), q.value(13).toInt()); // NOLINT(readability-magic-numbers)

		track.setDatabaseId(module()->databaseId());

		result.push_back(std::move(track));
	}

	return true;
}

bool Tracks::getMultipleTracksByPath(const QStringList& paths, MetaDataList& tracks) const
{
	for(const auto& path: paths)
	{
		auto track = getTrackByPath(path);
		if(track.id() >= 0)
		{
			tracks.push_back(std::move(track));
		}
	}

	return (tracks.count() == paths.size());
}

MetaData Tracks::getSingleTrack(const QString& queryText, const std::pair<QString, QVariant>& binding,
                                const QString& errorMessage) const
{
	auto q = Query(module());
	q.prepare(queryText);
	q.bindValue(binding.first, binding.second);

	MetaDataList tracks;
	const auto success = dbFetchTracks(q, tracks) && !tracks.isEmpty();
	if(!success)
	{
		spLog(Log::Warning, this) << errorMessage;
	}

	return success
	       ? tracks[0]
	       : MetaData();
}

MetaData Tracks::getTrackByPath(const QString& path) const
{
	const auto query = fetchQueryTracks("filename = :filename");
	const auto cleanedPath = Util::File::cleanFilename(path);
	return getSingleTrack(query, {":filename", cleanedPath}, "Cannot fetch track by path");
}

MetaData Tracks::getTrackById(TrackID id) const
{
	const auto query = fetchQueryTracks("trackID = :trackId");
	return getSingleTrack(query, {":trackId", id}, "Cannot fetch track by id");
}

int Tracks::getNumTracks() const
{
	const auto query = QStringLiteral("SELECT COUNT(tracks.trackid) FROM tracks WHERE libraryID=:libraryID;");
	auto q = module()->runQuery(
		query,
		{":libraryID", libraryId()},
		"Cannot count tracks"
	);

	return (!hasError(q) && q.next())
	       ? q.value(0).toInt()
	       : -1;
}

bool DB::Tracks::getAllTracksByIdList(const IdList& ids, const QString& idField, const Filter& filter,
                                      MetaDataList& result) const
{
	if(ids.isEmpty())
	{
		return false;
	}

	const auto sortedIds = Util::prepareContainerForRangeCalculation(ids);
	const auto ranges = Util::getRangesFromList(sortedIds);
	const auto mapping = DB::convertRangesToMapping(ranges, idField, "albumId");

	auto whereStatement = (filter.cleared())
	                      ? QString()
	                      : DB::getFilterWhereStatement(filter, CisPlaceholder) + " AND ";

	whereStatement += QString("(%1)").arg(mapping.sqlString);

	const auto query = fetchQueryTracks(whereStatement);

	const auto searchFilters = filter.searchModeFiltertext(true, GetSetting(Set::Lib_SearchMode));
	for(const auto& searchFilter: searchFilters)
	{
		auto q = Query(module());
		q.prepare(query);
		q.bindValue(CisPlaceholder, searchFilter);
		DB::bindMappingToQuery(q, mapping, sortedIds);

		MetaDataList tmpList;
		dbFetchTracks(q, tmpList);

		result.appendUnique(tmpList);
	}

	return true;
}

bool Tracks::getAllTracks(MetaDataList& result) const
{
	auto q = Query(module());
	const auto query = fetchQueryTracks("");
	q.prepare(query);

	return dbFetchTracks(q, result);
}

bool DB::Tracks::getAllTracksByAlbum(const IdList& albumsIds, MetaDataList& result) const
{
	return getAllTracksByAlbum(albumsIds, result, Filter(), -1);
}

bool
Tracks::getAllTracksByAlbum(const IdList& albumIds, MetaDataList& result, const Filter& filter, int discnumber) const
{
	const auto albumIdField = QString("%1.albumID").arg(trackSearchView());

	MetaDataList tracks;
	const auto success = getAllTracksByIdList(albumIds, albumIdField, filter, tracks);
	if(!success)
	{
		return false;
	}

	if(discnumber >= 0)
	{
		tracks.removeTracks([&](const auto& track) {
			return (track.discnumber() != discnumber);
		});
	}

	result.appendUnique(tracks);

	return true;
}

bool Tracks::getAllTracksByArtist(const IdList& artistIds, MetaDataList& result) const
{
	return getAllTracksByArtist(artistIds, result, Filter());
}

bool Tracks::getAllTracksByArtist(const IdList& artistIds, MetaDataList& result, const Filter& filter) const
{
	const auto artistIdField = QString("%1.%2")
		.arg(trackSearchView())
		.arg(this->artistIdField());

	return getAllTracksByIdList(artistIds, artistIdField, filter, result);
}

bool Tracks::getAllTracksBySearchString(const Filter& filter, MetaDataList& result) const
{
	const auto whereStatement = DB::getFilterWhereStatement(filter, CisPlaceholder);
	const auto query = fetchQueryTracks(whereStatement);

	const auto searchFilters = filter.searchModeFiltertext(true, GetSetting(Set::Lib_SearchMode));
	for(const auto& searchFilter: searchFilters)
	{
		auto q = Query(module());
		q.prepare(query);
		q.bindValue(CisPlaceholder, searchFilter);

		MetaDataList tracks;
		dbFetchTracks(q, tracks);
		result.appendUnique(tracks);
	}

	return true;
}

bool Tracks::getAllTracksByPaths(const QStringList& paths, MetaDataList& tracks) const
{
	QStringList queries;
	QMap<QString, QString> placeholderPathMapping;

	for(auto i = 0; i < paths.size(); i++)
	{
		const auto placeholder = QString(":path%1").arg(i);
		const auto whereStatement = QString("filename LIKE %1").arg(placeholder);
		auto query = fetchQueryTracks(whereStatement);
		query.remove(query.size() - 1, 1);

		queries << query;

		placeholderPathMapping[placeholder] = paths[i];
	}

	const auto query = queries.join(" UNION ") + ';';
	auto q = Query(module());
	q.prepare(query);
	for(auto it = placeholderPathMapping.begin(); it != placeholderPathMapping.end(); it++)
	{
		q.bindValue(it.key(), it.value() + '%');
	}

	return dbFetchTracks(q, tracks);
}

bool Tracks::deleteTrack(TrackID id)
{
	const auto queryText = QStringLiteral("DELETE FROM tracks WHERE trackID = :trackID;");
	const auto q = module()->runQuery(
		queryText,
		{":trackID", id},
		QString("Cannot delete track %1").arg(id));

	return !hasError(q);
}

bool Tracks::deleteTracks(const IdList& ids)
{
	if(ids.isEmpty())
	{
		return true;
	}

	module()->db().transaction();

	const auto fileCount = Util::Algorithm::count(ids, [this](const auto& id) {
		return deleteTrack(id);
	});

	const auto success = module()->db().commit();

	return (success && (fileCount == ids.size()));
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

	QMap<QString, int> trackIndexMap;
	Util::Set<Id> toDelete;
	int index = 0;

	for(const auto& track: tracks)
	{
		if(trackIndexMap.contains(track.filepath()))
		{
			spLog(Log::Warning, this) << "found double path: " << track.filepath();
			const auto trackIndex = trackIndexMap[track.filepath()];
			const auto& knownTrack = tracks[trackIndex];

			toDelete << track.id() << knownTrack.id();
			const auto contains = Util::Algorithm::contains(doubleMetadata, [&](const auto& doubleTrack) {
				return (knownTrack.id() == doubleTrack.id());
			});
			if(!contains)
			{
				doubleMetadata << knownTrack;
			}
		}

		else
		{
			trackIndexMap.insert(track.filepath(), index);
		}

		if(!libraryPath.isEmpty() && !track.filepath().contains(libraryPath))
		{
			toDelete << track.id();
		}

		index++;
	}

	spLog(Log::Debug, this) << "Deleting " << toDelete.size() << " tracks";
	auto success = deleteTracks(toDelete.toList());
	spLog(Log::Debug, this) << "delete tracks: " << success;

	return false;
}

Util::Set<Genre> Tracks::getAllGenres() const
{
	const auto query = QString("SELECT genre FROM %1 GROUP BY genre;").arg(trackView());
	auto q = module()->runQuery(query, "Cannot fetch genres");
	if(hasError(q))
	{
		return {};
	}

	Util::Set<Genre> genres;
	while(q.next())
	{
		const auto genre = q.value(0).toString();
		const auto subgenres = genre.split(",");

		for(const auto& subgenre: subgenres)
		{
			genres.insert(Genre(subgenre));
		}
	}

	return genres;
}

void Tracks::updateTrackCissearch()
{
	MetaDataList tracks;
	getAllTracks(tracks);

	module()->db().transaction();

	for(const auto& track: tracks)
	{
		module()->update(
			"tracks",
			{
				{"cissearch",      ::Library::convertSearchstring(track.title())},
				{"fileCissearch",  ::Library::convertSearchstring(track.filepath())},
				{"genreCissearch", ::Library::convertSearchstring(track.genresToString())}
			},
			{"trackId", track.id()},
			"Cannot update album cissearch");
	}

	module()->db().commit();
}

void Tracks::deleteAllTracks(bool alsoViews)
{
	if(libraryId() >= 0)
	{
		if(alsoViews)
		{
			dropTrackView(module(), libraryId(), trackView());
			dropTrackSearchView(module(), trackSearchView());
		}

		const auto query = QStringLiteral("DELETE FROM tracks WHERE libraryId=:libraryId;");
		module()->runQuery(
			query,
			{":libraryId", libraryId()},
			"Cannot delete library tracks");
	}
}

bool Tracks::updateTrack(const MetaData& track)
{
	if(track.id() < 0 || track.albumId() < 0 || track.artistId() < 0 || track.libraryId() < 0)
	{
		spLog(Log::Warning, this) << "Cannot update track (value negative): "
		                          << " ArtistID: " << track.artistId()
		                          << " AlbumID: " << track.albumId()
		                          << " TrackID: " << track.id()
		                          << " LibraryID: " << track.libraryId();
		return false;
	}

	auto bindings = getTrackBindings(track, track.artistId(), track.albumId(), track.albumArtistId());
	bindings["modifydate"] = QVariant::fromValue(Util::currentDateToInt());

	const auto q = module()->update(
		"tracks",
		bindings,
		{"trackId", track.id()},
		QString("Cannot update track %1").arg(track.filepath()));

	return wasUpdateSuccessful(q);
}

bool Tracks::renameFilepaths(const QMap<QString, QString>& paths, LibraryId targetLibrary)
{
	module()->db().transaction();

	const auto originalPaths = paths.keys();
	for(const auto& originalPath: originalPaths)
	{
		MetaDataList tracks;
		getAllTracksByPaths({originalPath}, tracks);

		const auto newPath = paths[originalPath];
		for(const auto& track: tracks)
		{
			const auto oldFilepath = track.filepath();
			auto newFilepath = oldFilepath;
			newFilepath.replace(originalPath, newPath);

			renameFilepath(oldFilepath, newFilepath, targetLibrary);
		}
	}

	return module()->db().commit();
}

bool Tracks::renameFilepath(const QString& oldPath, const QString& newPath, LibraryId targetLibrary)
{
	const auto q = module()->update(
		"Tracks",
		{
			{"filename",  newPath},
			{"libraryID", targetLibrary}
		},

		{"filename", oldPath},
		"Could not rename Filepath");

	return wasUpdateSuccessful(q);
}

bool Tracks::insertTrackIntoDatabase(const MetaData& track, ArtistId artistId, AlbumId albumId, ArtistId albumArtistId)
{
	if(albumArtistId == -1)
	{
		albumArtistId = artistId;
	}

	const auto createdTime = !track.createdDateTime().isValid()
	                         ? Util::currentDateToInt()
	                         : track.createdDate();

	const auto modifiedTime = (!track.modifiedDateTime().isValid())
	                          ? Util::currentDateToInt()
	                          : track.modifiedDate();

	auto bindings = getTrackBindings(track, artistId, albumId, albumArtistId);
	bindings["createdate"] = QVariant::fromValue(createdTime);
	bindings["modifydate"] = QVariant::fromValue(modifiedTime);

	const auto q = module()->insert("tracks", bindings, QString("Cannot insert track %1").arg(track.filepath()));

	return !hasError(q);
}
