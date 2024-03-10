/* SoundcloudLibraryDatabase.cpp
 *
 * Copyright (C) 2011-2024 Michael Lugmair
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

#include "SoundcloudLibraryDatabase.h"
#include "SearchInformation.h"

#include "Database/Query.h"

#include "Utils/typedefs.h"
#include "Utils/MetaData/MetaData.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/MetaData/Album.h"
#include "Utils/MetaData/Artist.h"
#include "Utils/Language/Language.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Set.h"
#include "Utils/Utils.h"

namespace
{
	constexpr const auto PermalinkUrlKey = "permalink_url";
	constexpr const auto PurchaseUrlKey = "purchase_url";

	QString getCoverUrl(const LibraryItem& libraryItem)
	{
		return (!libraryItem.coverDownloadUrls().isEmpty())
		       ? libraryItem.coverDownloadUrls().constFirst()
		       : QString("");
	}

	QList<Disc> getDiscnumbersFromVariant(const QVariant& variant)
	{
		const auto discnumberList = variant.toString().split(',');
		auto discnumbers = Util::Set<Disc> {};
		for(const auto& discNumber: discnumberList)
		{
			discnumbers << static_cast<Disc>(discNumber.toInt());
		}

		return (discnumbers.isEmpty())
		       ? (QList<Disc>() << 1)
		       : discnumbers.toList();
	}
}

SC::LibraryDatabase::LibraryDatabase(const QString& connectionName, DbId databaseId, LibraryId libraryId) :
	::DB::LibraryDatabase(connectionName, databaseId, libraryId) {}

SC::LibraryDatabase::~LibraryDatabase() = default;

QString SC::LibraryDatabase::fetchQueryArtists(bool alsoEmpty) const
{
	static const auto fieldList = QStringList {
		QStringLiteral("artists.artistid            AS artistID"),                  // 0
		QStringLiteral("artists.name                AS artistName"),                // 1
		QStringLiteral("artists.permalink_url       AS permalink_url"),             // 2
		QStringLiteral("artists.description         AS description"),               // 3
		QStringLiteral("artists.followers_following AS followers_following"),       // 4
		QStringLiteral("artists.cover_url           AS cover_url"),                 // 5
		QStringLiteral("artists.name                AS albumArtistName"),           // 6
		QStringLiteral("COUNT(DISTINCT tracks.trackid)        AS trackCount"),      // 7
		QStringLiteral("GROUP_CONCAT(DISTINCT albums.albumid) AS artistAlbums")     // 8
	};

	static const auto fields = fieldList.join(", ");

	const auto joinType = (alsoEmpty)
	                      ? QStringLiteral("LEFT OUTER JOIN")
	                      : QStringLiteral("INNER JOIN");

	return QString("SELECT %1 FROM artists %2"
	               " tracks ON artists.artistID = tracks.artistID"
	               " %2"
	               " albums ON albums.albumID = tracks.albumID ")
		.arg(fields)
		.arg(joinType);
}

QString SC::LibraryDatabase::fetchQueryAlbums(bool alsoEmpty) const
{
	static const auto fieldList = QStringList {
		QStringLiteral("albums.albumID            AS albumID"),           // 0
		QStringLiteral("albums.name               AS albumName"),         // 1
		QStringLiteral("SUM(tracks.length) / 1000 AS albumLength"),       // 2
		QStringLiteral("albums.rating             AS albumRating"),       // 3
		QStringLiteral("albums.permalink_url      AS permalink_url"),     // 4
		QStringLiteral("albums.purchase_url       AS purchase_url"),      // 5
		QStringLiteral("albums.cover_url          AS cover_url"),         // 6
		QStringLiteral("COUNT(DISTINCT tracks.trackid) AS trackCount"),    // 7
		QStringLiteral("MAX(tracks.year)               AS albumYear"),     // 8
		QStringLiteral("GROUP_CONCAT(DISTINCT artists.name)      AS albumArtists"),  // 9
		QStringLiteral("GROUP_CONCAT(DISTINCT tracks.discnumber) AS discnumbers")     // 10
	};

	static const auto fields = fieldList.join(", ");

	const auto joinType = (alsoEmpty)
	                      ? QStringLiteral("LEFT OUTER JOIN")
	                      : QStringLiteral("INNER JOIN");

	return QString("SELECT %1 FROM albums %2 "
	               "tracks ON albums.albumID = tracks.albumID "
	               "%2 "
	               "artists ON artists.artistID = tracks.artistID ")
		.arg(fields)
		.arg(joinType);
}

QString SC::LibraryDatabase::fetchQueryTracks(const QString& where) const
{
	static const auto fieldList = QStringList {
		QStringLiteral("tracks.trackID      AS trackID"),       // 0
		QStringLiteral("tracks.title        AS trackTitle"),    // 1
		QStringLiteral("tracks.length       AS trackLength"),   // 2
		QStringLiteral("tracks.year         AS trackYear"),     // 3
		QStringLiteral("tracks.bitrate      AS trackBitrate"),  // 4
		QStringLiteral("tracks.filename     AS trackFilename"), // 5
		QStringLiteral("tracks.track        AS trackNum"),      // 6
		QStringLiteral("albums.albumID      AS albumID"),       // 7
		QStringLiteral("artists.artistID    AS artistID"),      // 8
		QStringLiteral("albums.name         AS albumName"),     // 9
		QStringLiteral("artists.name        AS artistName"),    // 10
		QStringLiteral("tracks.genre        AS genrename"),     // 11
		QStringLiteral("tracks.filesize     AS filesize"),      // 12
		QStringLiteral("tracks.discnumber   AS discnumber"),    // 13
		QStringLiteral("tracks.purchase_url AS purchase_url"),  // 14
		QStringLiteral("tracks.cover_url    AS cover_url"),     // 15
		QStringLiteral("tracks.rating       AS rating"),        // 16
		QStringLiteral("tracks.createdate   AS createdate"),    // 17
		QStringLiteral("tracks.modifydate   AS modifydate"),    // 18
		QStringLiteral("tracks.comment      AS comment")        // 19
	};

	static const auto fields = fieldList.join(", ");

	const auto joinStatement = QStringLiteral(
		"INNER JOIN albums ON tracks.albumID = albums.albumID "
		"INNER JOIN artists ON tracks.artistID = artists.artistID ");

	const auto whereStatement = (where.isEmpty()) ? "1" : where;

	return QString("SELECT %1 FROM tracks %2 WHERE %3 ")
		.arg(fields)
		.arg(joinStatement)
		.arg(whereStatement);
}

bool SC::LibraryDatabase::dbFetchTracks(QSqlQuery& query, MetaDataList& result) const
{
	result.clear();

	if(!query.exec())
	{
		DB::showError(query, "Cannot fetch tracks from database");
		return false;
	}

	if(!query.last())
	{
		return true;
	}

	for(auto isElement = query.first(); isElement; isElement = query.next())
	{
		MetaData track;

		track.setId(query.value(0).toInt());
		track.setTitle(query.value(1).toString());
		track.setDurationMs(query.value(2).toInt());
		track.setYear(query.value(3).value<Year>());
		track.setBitrate(query.value(4).value<Bitrate>());
		track.setFilepath(query.value(5).toString());
		track.setTrackNumber(query.value(6).value<TrackNum>());
		track.setAlbumId(query.value(7).toInt());
		track.setArtistId(query.value(8).toInt());
		track.setAlbum(query.value(9).toString().trimmed());
		track.setArtist(query.value(10).toString().trimmed());
		track.setGenres(query.value(11).toString().split(","));
		track.setFilesize(query.value(12).value<Filesize>());
		track.setDiscnumber(query.value(13).value<Disc>());
		track.addCustomField(PurchaseUrlKey, Lang::get(Lang::PurchaseUrl), query.value(14).toString());
		track.setCoverDownloadUrls({query.value(15).toString()});
		track.setRating(query.value(16).value<Rating>());
		track.setCreatedDate(query.value(17).value<uint64_t>());
		track.setModifiedDate(query.value(18).value<uint64_t>());
		track.setComment(query.value(19).toString());
		track.setDatabaseId(databaseId());

		result.push_back(std::move(track));
	}

	return true;
}

bool SC::LibraryDatabase::dbFetchAlbums(QSqlQuery& query, AlbumList& result) const
{
	result.clear();

	if(!query.exec())
	{
		DB::showError(query, "Could not get all albums from database");
		return false;
	}

	while(query.next())
	{
		Album album;

		album.setId(query.value(0).toInt());
		album.setName(query.value(1).toString().trimmed());
		album.setDurationSec(query.value(2).value<Seconds>());
		album.setRating(query.value(3).value<Rating>());
		album.addCustomField(PermalinkUrlKey, QStringLiteral("Permalink Url"), query.value(4).toString());
		album.addCustomField(PurchaseUrlKey, QStringLiteral("Purchase Url"), query.value(5).toString());
		album.setCoverDownloadUrls({query.value(6).toString()});
		album.setSongcount(query.value(7).value<TrackNum>());
		album.setYear(query.value(8).value<Year>());
		album.setArtists(query.value(9).toString().split(','));
		album.setDiscnumbers(getDiscnumbersFromVariant(query.value(10)));
		album.setDatabaseId(databaseId());

		result.push_back(std::move(album));
	}

	return true;
}

bool SC::LibraryDatabase::dbFetchArtists(QSqlQuery& query, ArtistList& result) const
{
	result.clear();

	if(!query.exec())
	{
		DB::showError(query, "Could not get all artists from database");
		return false;
	}

	if(!query.last())
	{
		return true;
	}

	for(auto isElement = query.first(); isElement; isElement = query.next())
	{
		Artist artist;

		artist.setId(query.value(0).toInt());
		artist.setName(query.value(1).toString().trimmed());

		artist.addCustomField(PermalinkUrlKey, QStringLiteral("Permalink Url"), query.value(2).toString());
		artist.addCustomField(QStringLiteral("description"),
		                      QStringLiteral("Description"),
		                      query.value(3).toString());
		artist.addCustomField(QStringLiteral("followers_following"),
		                      QStringLiteral("Followers/Following"),
		                      query.value(4).toString());

		artist.setCoverDownloadUrls({query.value(5).toString()});
		artist.setSongcount(query.value(7).value<uint16_t>());
		artist.setDatabaseId(databaseId());

		result.push_back(std::move(artist));
	}

	return true;
}

ArtistId SC::LibraryDatabase::updateArtist(const Artist& artist)
{
	const auto query = this->update(
		QStringLiteral("artists"),
		{
			{QStringLiteral("name"),                Util::convertNotNull(artist.name())},
			{QStringLiteral("cissearch"),           Util::convertNotNull(artist.name().toLower())},
			{PermalinkUrlKey,                       artist.customField(PermalinkUrlKey)},
			{QStringLiteral("description"),         artist.customField(QStringLiteral("description"))},
			{QStringLiteral("followers_following"), artist.customField(QStringLiteral("followers_following"))},
			{QStringLiteral("cover_url"),           getCoverUrl(artist)}
		},
		{QStringLiteral("sc_id"), artist.id()},
		QString("Soundcloud: Cannot update artist %1").arg(artist.name()));

	return DB::hasError(query)
	       ? -1
	       : getArtistID(artist.name());
}

ArtistId SC::LibraryDatabase::insertArtistIntoDatabase([[maybe_unused]] const QString& artist)
{
	return -1;
}

bool SC::LibraryDatabase::getAllAlbums(AlbumList& result, bool alsoEmpty) const
{
	auto query = QSqlQuery(db());
	const auto queryText =
		fetchQueryAlbums(alsoEmpty) +
		QStringLiteral(" GROUP BY albums.albumID, albums.name, albums.rating ");

	query.prepare(queryText);

	return dbFetchAlbums(query, result);
}

ArtistId SC::LibraryDatabase::insertArtistIntoDatabase(const Artist& artist)
{
	Artist foundArtist;
	if(getArtistByID(artist.id(), foundArtist) && (foundArtist.id() > 0))
	{
		return updateArtist(artist);
	}

	const auto query = this->insert(
		QStringLiteral("artists"),
		{
			{QStringLiteral("artistID"),            artist.id()},
			{QStringLiteral("name"),                Util::convertNotNull(artist.name())},
			{QStringLiteral("cissearch"),           Util::convertNotNull(artist.name().toLower())},
			{PermalinkUrlKey,                       artist.customField(PermalinkUrlKey)},
			{QStringLiteral("description"),         artist.customField(QStringLiteral("description"))},
			{QStringLiteral("followers_following"), artist.customField(QStringLiteral("followers_following"))},
			{QStringLiteral("cover_url"),           getCoverUrl(artist)}
		},
		QString("Soundcloud: Cannot insert artist %1").arg(artist.name()));

	return DB::hasError(query)
	       ? -1
	       : getArtistID(artist.name());
}

AlbumId SC::LibraryDatabase::updateAlbum(const Album& album)
{
	const auto query = this->update(
		QStringLiteral("albums"),
		{
			{QStringLiteral("name"),      Util::convertNotNull(album.name())},
			{QStringLiteral("cissearch"), Util::convertNotNull(album.name().toLower())},
			{PermalinkUrlKey,             album.customField(PermalinkUrlKey)},
			{PurchaseUrlKey,              album.customField(PurchaseUrlKey)},
			{QStringLiteral("cover_url"), getCoverUrl(album)}
		},
		{QStringLiteral("sc_id"), album.id()},
		QString("Soundcloud: Cannot update album %1").arg(album.name()));

	return DB::hasError(query) ? -1 : album.id();
}

AlbumId SC::LibraryDatabase::insertAlbumIntoDatabase([[maybe_unused]] const QString& album)
{
	return -1;
}

AlbumId SC::LibraryDatabase::insertAlbumIntoDatabase(const Album& album)
{
	auto query = this->insert(
		QStringLiteral("albums"),
		{
			{QStringLiteral("albumID"),   album.id()},
			{QStringLiteral("name"),      Util::convertNotNull(album.name())},
			{QStringLiteral("cissearch"), Util::convertNotNull(album.name().toLower())},
			{PermalinkUrlKey,             album.customField(PermalinkUrlKey)},
			{PurchaseUrlKey,              album.customField(PurchaseUrlKey)},
			{QStringLiteral("cover_url"), getCoverUrl(album)}
		},
		QString("Soundcloud: Cannot insert album %1").arg(album.name()));

	return DB::hasError(query) ? -1 : query.lastInsertId().toInt();
}

bool SC::LibraryDatabase::updateTrack(const MetaData& track)
{
	const auto query = this->update(
		QStringLiteral("tracks"),
		{
			{QStringLiteral("title"),      Util::convertNotNull(track.title())},
			{QStringLiteral("filename"),   Util::convertNotNull(track.filepath())},
			{QStringLiteral("albumID"),    track.albumId()},
			{QStringLiteral("artistID"),   track.artistId()},
			{QStringLiteral("length"),     QVariant::fromValue(track.durationMs())},
			{QStringLiteral("year"),       track.year()},
			{QStringLiteral("track"),      track.trackNumber()},
			{QStringLiteral("bitrate"),    track.bitrate()},
			{QStringLiteral("genre"),      Util::convertNotNull(track.genresToList().join("),"))},
			{QStringLiteral("filesize"),   QVariant::fromValue(track.filesize())},
			{QStringLiteral("discnumber"), track.discnumber()},
			{QStringLiteral("cissearch"),  Util::convertNotNull(track.title().toLower())},
			{PurchaseUrlKey,               track.customField(PurchaseUrlKey)},
			{QStringLiteral("cover_url"),  getCoverUrl(track)},
			{QStringLiteral("createdate"), QVariant::fromValue(track.createdDate())},
			{QStringLiteral("modifydate"), QVariant::fromValue(track.modifiedDate())},
			{QStringLiteral("comment"),    track.comment()}
		},
		{QStringLiteral("trackID"), track.id()},
		QString("Soundcloud: Cannot update track %1").arg(track.filepath()));

	return !DB::hasError(query);
}

bool SC::LibraryDatabase::insertTrackIntoDatabase(const MetaData& track, int artistId, int albumId,
                                                  [[maybe_unused]] int albumArtistId)
{
	if(const auto newId = getTrackById(track.id()).id(); (newId > 0))
	{
		return updateTrack(track);
	}

	const auto query = this->insert(
		QStringLiteral("tracks"),
		{
			{QStringLiteral("trackID"),    track.id()},
			{QStringLiteral("title"),      Util::convertNotNull(track.title())},
			{QStringLiteral("filename"),   Util::convertNotNull(track.filepath())},
			{QStringLiteral("albumID"),    albumId},
			{QStringLiteral("artistID"),   artistId},
			{QStringLiteral("length"),     QVariant::fromValue(track.durationMs())},
			{QStringLiteral("year"),       track.year()},
			{QStringLiteral("track"),      track.trackNumber()},
			{QStringLiteral("bitrate"),    track.bitrate()},
			{QStringLiteral("genre"),      track.genresToList().join(',')},
			{QStringLiteral("filesize"),   QVariant::fromValue(track.filesize())},
			{QStringLiteral("discnumber"), track.discnumber()},
			{QStringLiteral("cissearch"),  Util::convertNotNull(track.title().toLower())},
			{PurchaseUrlKey,               track.customField(PurchaseUrlKey)},
			{QStringLiteral("cover_url"),  getCoverUrl(track)},
			{QStringLiteral("createdate"), QVariant::fromValue(track.createdDate())},
			{QStringLiteral("modifydate"), QVariant::fromValue(track.modifiedDate())},
			{QStringLiteral("comment"),    track.comment()}
		},
		QString("Soundcloud: Cannot insert track %1").arg(track.filepath()));

	return !DB::hasError(query);
}

bool SC::LibraryDatabase::storeMetadata(const MetaDataList& tracks)
{
	if(tracks.isEmpty())
	{
		return true;
	}

	db().transaction();

	for(const auto& track: tracks)
	{
		spLog(Log::Debug, this) << "Looking for " << track.artist() << " and " << track.album();
		if((track.albumId() == -1) || (track.artistId() == -1))
		{
			spLog(Log::Warning, this) << "AlbumID = " << track.albumId() << " - ArtistID = " << track.artistId();
			continue;
		}

		insertTrackIntoDatabase(track, track.artistId(), track.albumId(), track.albumArtistId());
	}

	return db().commit();
}

bool SC::LibraryDatabase::searchInformation(SC::SearchInformationList& searchInformation)
{
	auto query = this->runQuery(
		QStringLiteral("SELECT artistId, albumId, trackId, allCissearch FROM track_search_view;"),
		QStringLiteral("Soundcloud: Cannot get search Information"));

	if(DB::hasError(query))
	{
		return false;
	}

	while(query.next())
	{
		searchInformation << SC::SearchInformation(
			query.value(0).toInt(),
			query.value(1).toInt(),
			query.value(2).toInt(),
			query.value(3).toString());
	}

	return true;
}
