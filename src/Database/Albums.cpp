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

#include "Database/Albums.h"
#include "Database/Module.h"
#include "Database/Utils.h"

#include "Utils/MetaData/Album.h"
#include "Utils/Library/Filter.h"
#include "Utils/Utils.h"
#include "Utils/Ranges.h"
#include "Utils/Settings/Settings.h"

using DB::Albums;
using DB::Query;
using ::Library::Filter;

namespace
{
	constexpr const auto CisPlaceholder = ":cissearch";

	QList<Disc> variantToDiscnumbers(const QVariant& variant)
	{
		QList<Disc> result;
		auto discs = variant.toString().split(',');
		discs.removeDuplicates();

		for(const auto& disc: discs)
		{
			result << static_cast<Disc>(disc.toInt());
		}

		if(result.isEmpty())
		{
			result << static_cast<Disc>(1U);
		}

		return result;
	}

	void dropAlbumView(DB::Module* module, const QString& albumView)
	{
		const auto query = QString("DROP VIEW IF EXISTS %1;").arg(albumView);
		module->runQuery(query, "Cannot drop album view");
	}

	void createAlbumView(DB::Module* module, const QString& trackView, const QString& albumView)
	{
		static const auto fields = QStringList
			{
				QStringLiteral("albums.albumID                           AS albumID"),            // 0
				QStringLiteral("albums.name                              AS albumName"),          // 1
				QStringLiteral("albums.rating                            AS albumRating"),        // 2
				QStringLiteral("GROUP_CONCAT(DISTINCT artists.name)      AS artistNames"),        // 3
				QStringLiteral("GROUP_CONCAT(DISTINCT albumArtists.name) AS albumArtistName"),    // 4
				QStringLiteral("SUM(%1.length) / 1000                    AS albumLength"),        // 5
				QStringLiteral("COUNT(DISTINCT %1.trackID)               AS trackCount"),         // 6
				QStringLiteral("MAX(%1.year)                             AS albumYear"),          // 7
				QStringLiteral("GROUP_CONCAT(DISTINCT %1.discnumber)     AS discnumbers"),        // 8
				QStringLiteral("GROUP_CONCAT(%1.filename, '#')           AS filenames")           // 9
			};

		const auto fieldStatement = fields.join(", ").arg(trackView);
		const auto joinStatement = QString("LEFT OUTER JOIN %1 ON %1.albumID = albums.albumID " // leave out empty albums
		                                   "LEFT OUTER JOIN artists ON %1.artistID = artists.artistID "
		                                   "LEFT OUTER JOIN artists albumArtists ON %1.albumArtistID = albumArtists.artistID")
			.arg(trackView);

		const auto query = QString("CREATE VIEW %1 AS SELECT %2 FROM albums %3 GROUP BY albums.albumID;")
			.arg(albumView)
			.arg(fieldStatement)
			.arg(joinStatement);

		module->runQuery(query, "Cannot create album view");
	}

	QString getAlbumFetchFields()
	{
		static const auto fields = QStringList
			{
				QStringLiteral("albumID"),
				QStringLiteral("albumName"),
				QStringLiteral("albumRating"),
				QStringLiteral("GROUP_CONCAT(DISTINCT artistName)"),
				QStringLiteral("GROUP_CONCAT(DISTINCT albumArtistName)"),
				QStringLiteral("SUM(length) / 1000 AS albumLength"),
				QStringLiteral("COUNT(DISTINCT trackID) AS trackCount"),
				QStringLiteral("MAX(year) AS albumYear"),
				QStringLiteral("GROUP_CONCAT(DISTINCT discnumber)"),
				QStringLiteral("GROUP_CONCAT(DISTINCT filename)")
			};

		static const auto joinedFields = fields.join(", ");
		return joinedFields;
	}

	QString getFetchQueryText(const QString& trackSearchView, const QString& whereStatement)
	{
		return QString("SELECT %1 FROM %2 WHERE %3 GROUP BY albumId, albumName")
			.arg(getAlbumFetchFields())
			.arg(trackSearchView)
			.arg(whereStatement);
	}
}

Albums::Albums() = default;
Albums::~Albums() = default;

static QString albumViewName(LibraryId libraryId)
{
	return (libraryId < 0)
	       ? QStringLiteral("album_view")
	       : QString("album_view_%1").arg(QString::number(libraryId));
}

void Albums::initViews()
{
	const auto& viewName = albumViewName(libraryId());

	dropAlbumView(module(), viewName);
	createAlbumView(module(), trackView(), viewName);
}

QString Albums::fetchQueryAlbums(bool alsoEmpty) const
{
	static const auto fields = QStringList
		{
			QStringLiteral("albumID"),            // 0
			QStringLiteral("albumName"),          // 1
			QStringLiteral("albumRating"),        // 2
			QStringLiteral("artistNames"),        // 3
			QStringLiteral("albumArtistName"),    // 4
			QStringLiteral("albumLength"),        // 5
			QStringLiteral("trackCount"),         // 6
			QStringLiteral("albumYear"),          // 7
			QStringLiteral("discnumbers"),        // 8
			QStringLiteral("filenames")           // 9
		};

	static const auto joinedFields = fields.join(", ");

	const auto whereStatement = alsoEmpty
	                            ? QStringLiteral("1")
	                            : QStringLiteral("trackCount > 0");

	return QString("SELECT %1 FROM %2 WHERE %3 ")
		.arg(joinedFields)
		.arg(albumViewName(libraryId()))
		.arg(whereStatement);
}

bool Albums::dbFetchAlbums(Query& q, AlbumList& result) const
{
	result.clear();

	if(!q.exec())
	{
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
		album.setDiscnumbers(variantToDiscnumbers(q.value(8)));
		album.setDatabaseId(module()->databaseId());
		album.setPathHint(q.value(9).toString().split("#"));

		result.push_back(std::move(album));
	}

	return true;
}

AlbumId Albums::getAlbumID(const QString& album) const
{
	auto q = Query(module());

	q.prepare("SELECT albumID FROM albums WHERE name = ?;");
	q.addBindValue(Util::convertNotNull(album));

	if(!q.exec())
	{
		q.showError("Cannot fetch albumID");
		return -1;
	}

	return q.next()
	       ? q.value(0).toInt()
	       : -1;
}

bool Albums::getAlbumByID(AlbumId id, Album& album) const
{
	return getAlbumByID(id, album, false);
}

bool Albums::getAlbumByID(AlbumId id, Album& album, bool alsoEmpty) const
{
	if(id == -1)
	{
		return false;
	}

	const auto query = QString("%1 AND albumID = :id GROUP BY albumID, albumName, albumRating;")
		.arg(fetchQueryAlbums(alsoEmpty));

	auto q = Query(module());
	q.prepare(query);
	q.bindValue(":id", id);

	AlbumList albums;
	dbFetchAlbums(q, albums);

	if(!albums.empty())
	{
		album = std::move(albums[0]);
	}

	return (!albums.empty());
}

bool Albums::getAllAlbums(AlbumList& result, bool alsoEmpty) const
{
	const auto query = QString("%1 GROUP BY albumID, albumName, albumRating;")
		.arg(fetchQueryAlbums(alsoEmpty));

	auto q = Query(module());
	q.prepare(query);

	return dbFetchAlbums(q, result);
}

bool Albums::getAllAlbumsByArtist(const IdList& artistIds, AlbumList& result, const Library::Filter& filter) const
{
	if(artistIds.isEmpty())
	{
		return false;
	}

	const auto artistIdField = trackSearchView() + "." + this->artistIdField();

	const auto sortedIds = Util::prepareContainerForRangeCalculation(artistIds);
	const auto ranges = Util::getRangesFromList(sortedIds);
	const auto mapping = DB::convertRangesToMapping(ranges, artistIdField, "artistId");
	auto whereStatement = (filter.cleared())
	                      ? QString()
	                      : DB::getFilterWhereStatement(filter, CisPlaceholder) + " AND ";

	whereStatement += QString("(%1)").arg(mapping.sqlString);

	const auto query = getFetchQueryText(trackSearchView(), whereStatement);

	const auto searchFilters = filter.searchModeFiltertext(true, GetSetting(Set::Lib_SearchMode));
	for(const auto& searchFilter: searchFilters)
	{
		Query q(module());
		q.prepare(query);
		q.bindValue(CisPlaceholder, searchFilter);
		DB::bindMappingToQuery(q, mapping, sortedIds);

		AlbumList tmpAlbums;
		dbFetchAlbums(q, tmpAlbums);
		result.appendUnique(tmpAlbums);
	}

	return true;
}

bool Albums::getAllAlbumsBySearchString(const Library::Filter& filter, AlbumList& result) const
{
	const auto whereStatement = DB::getFilterWhereStatement(filter, CisPlaceholder);
	const auto query = getFetchQueryText(trackSearchView(), whereStatement);

	const auto searchFilters = filter.searchModeFiltertext(true, GetSetting(Set::Lib_SearchMode));
	for(const auto& searchFilter: searchFilters)
	{
		auto q = Query(module());
		q.prepare(query);
		q.bindValue(CisPlaceholder, searchFilter);

		AlbumList tmpList;
		dbFetchAlbums(q, tmpList);
		result.appendUnique(tmpList);
	}

	return true;
}

AlbumId Albums::updateAlbumRating(AlbumId id, Rating rating)
{
	const auto bindings = QMap<QString, QVariant>
		{
			{"rating", QVariant::fromValue(static_cast<int>(rating))}
		};

	const auto q = module()->update(
		"albums",
		bindings,
		{"albumID", id},
		QString("Cannot set album rating for id %1").arg(id));

	return (!q.hasError()) ? id : -1;
}

void Albums::updateAlbumCissearch()
{
	AlbumList albums;
	getAllAlbums(albums, true);

	module()->db().transaction();

	for(const Album& album: albums)
	{
		const auto cissearch = Library::convertSearchstring(album.name());

		module()->update(
			"albums",
			{
				{"cissearch", cissearch}
			},
			{"albumID", album.id()},
			"Cannot update album cissearch"
		);
	}

	module()->db().commit();
}

AlbumId Albums::insertAlbumIntoDatabase(const QString& name)
{
	const auto cissearch = Library::convertSearchstring(name);
	const auto bindings = QMap<QString, QVariant>
		{
			{"name",      Util::convertNotNull(name)},
			{"cissearch", cissearch},
			{"rating",    QVariant::fromValue(static_cast<int>(Rating::Zero))}
		};

	const auto q = module()->insert("albums", bindings, QString("2. Cannot insert album %1").arg(name));
	return (!q.hasError()) ? q.lastInsertId().toInt() : -1;
}

AlbumId Albums::insertAlbumIntoDatabase(const Album& album)
{
	return insertAlbumIntoDatabase(album.name());
}

void Albums::deleteAllAlbums()
{
	module()->runQuery("DELETE FROM albums;", "Could not delete all albums");
}
