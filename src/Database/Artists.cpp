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

#include "Database/Artists.h"
#include "Database/Module.h"
#include "Database/Utils.h"

#include "Utils/MetaData/MetaData.h"
#include "Utils/MetaData/Artist.h"
#include "Utils/Library/Filter.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Utils.h"

using DB::Artists;
using DB::Query;

namespace
{
	QString getSearchSelectStatement(const QString& artistIdField, const QString& artistNameField)
	{
		return QString("%1, %2, %3")
			.arg(artistIdField)
			.arg(artistNameField)
			.arg(QStringLiteral("COUNT(DISTINCT trackID) AS trackCount "));
	}

	QString getSearchGroupByStatement(const QString& artistIdField, const QString& artistNameField)
	{
		return QString("%1, %2")
			.arg(artistIdField)
			.arg(artistNameField);
	}

	QString getJoinedArtistFields(const QString& trackView)
	{
		static const auto fields = QStringList
			{
				QStringLiteral("artists.artistID           AS artistID"),
				QStringLiteral("artists.name               AS artistName"),
				QStringLiteral("COUNT(DISTINCT %1.trackID) AS trackCount")
			};

		static const auto joinedFields = fields.join(", ");

		return joinedFields.arg(trackView);
	}
}

Artists::Artists() = default;
Artists::~Artists() = default;

QString Artists::fetchQueryArtists(bool alsoEmpty) const
{
	const auto joinedFields = getJoinedArtistFields(trackView());
	const auto joinType = (alsoEmpty)
	                      ? QStringLiteral("LEFT OUTER JOIN")
	                      : QStringLiteral("INNER JOIN");

	const auto joinStatementArtist = QString("%1 %2 ON %2.%3 = artists.artistID")
		.arg(joinType)
		.arg(trackView())
		.arg(artistIdField());

	const auto joinStatementAlbum = QString("%1 albums ON %2.albumID = albums.albumID")
		.arg(joinType)
		.arg(trackView());

	const auto queryText = QString("SELECT %1 FROM artists %2 %3")
		.arg(joinedFields)
		.arg(joinStatementArtist)
		.arg(joinStatementAlbum);

	return queryText;
}

bool Artists::dbFetchArtists(Query& q, ArtistList& result) const
{
	result.clear();

	if(!q.exec())
	{
		q.showError("Could not get all artists from database");
		return false;
	}

	while(q.next())
	{
		Artist artist;
		artist.setId(q.value(0).value<ArtistId>());
		artist.setName(q.value(1).toString());
		artist.setSongcount(q.value(2).value<uint16_t>());
		artist.setDatabaseId(module()->databaseId());

		result << artist;
	}

	return true;
}

bool Artists::getArtistByID(ArtistId id, Artist& artist) const
{
	return getArtistByID(id, artist, false);
}

bool Artists::getArtistByID(ArtistId id, Artist& artist, bool alsoEmpty) const
{
	if(id < 0)
	{
		return false;
	}

	const auto queryText = QString("%1 WHERE artists.artistID = ?;")
		.arg(fetchQueryArtists(alsoEmpty));

	auto query = Query(module());
	query.prepare(queryText);
	query.addBindValue(id);

	ArtistList artists;
	const auto success = dbFetchArtists(query, artists);
	if(!success || artists.empty())
	{
		return false;
	}

	artist = artists[0];
	return true;
}

ArtistId Artists::getArtistID(const QString& artist) const
{
	const auto queryText = QStringLiteral("SELECT artistID FROM artists WHERE name = :name;");
	auto query = module()->runQuery(
		queryText,
		{":name", Util::convertNotNull(artist)},
		QString("Cannot fetch artistID for artist %1").arg(artist)
	);

	if(query.hasError())
	{
		return -1;
	}

	return (query.next())
	       ? query.value(0).toInt()
	       : -1;
}

bool Artists::getAllArtists(ArtistList& result, bool alsoEmpty) const
{
	const auto queryText = QString("%1 GROUP BY artists.artistID, artists.name;")
		.arg(fetchQueryArtists(alsoEmpty));

	auto query = Query(module());
	query.prepare(queryText);

	return dbFetchArtists(query, result);
}

bool Artists::getAllArtistsBySearchString(const Library::Filter& filter, ArtistList& result) const
{
	static const auto cisPlaceholder = QStringLiteral(":cissearch");

	const auto searchSelectStatement = getSearchSelectStatement(artistIdField(), artistNameField());
	const auto filterWhereStatement = DB::getFilterWhereStatement(filter, cisPlaceholder);
	const auto groupByStatement = getSearchGroupByStatement(artistIdField(), artistNameField());
	const auto queryText = QString("SELECT %1 FROM %2 WHERE %3 GROUP BY %4;")
		.arg(searchSelectStatement)
		.arg(trackSearchView())
		.arg(filterWhereStatement)
		.arg(getSearchGroupByStatement(artistIdField(), artistNameField()));

	const auto searchFilters = filter.searchModeFiltertext(true, GetSetting(Set::Lib_SearchMode));
	for(const auto& searchFilter: searchFilters)
	{
		auto query = Query(module());
		query.prepare(queryText);
		query.bindValue(cisPlaceholder, Util::convertNotNull(searchFilter));

		ArtistList artists;
		dbFetchArtists(query, artists);
		result.appendUnique(artists);
	}

	return true;
}

bool Artists::deleteArtist(ArtistId id)
{
	const auto queryText = QStringLiteral("DELETE FROM artists WHERE artistId = :artistId;");
	auto query = module()->runQuery(
		queryText,
		{":artistId", id},
		QString("Cannot delete artist %1").arg(id));

	return (!query.hasError());
}

ArtistId Artists::insertArtistIntoDatabase(const QString& artist)
{
	const auto id = getArtistID(artist);
	if(id >= 0)
	{
		return id;
	}

	const auto searchString = Library::Utils::convertSearchstring(artist);

	const auto bindings = QMap<QString, QVariant>
		{
			{"name",      Util::convertNotNull(artist)},
			{"cissearch", Util::convertNotNull(searchString)}
		};

	auto query = module()->insert("artists", bindings, QString("Cannot insert artist %1").arg(artist));
	return (query.hasError())
	       ? -1
	       : query.lastInsertId().toInt();
}

ArtistId Artists::insertArtistIntoDatabase(const Artist& artist)
{
	return insertArtistIntoDatabase(artist.name());
}

void Artists::updateArtistCissearch()
{
	ArtistList artists;
	getAllArtists(artists, true);

	module()->db().transaction();

	for(const auto& artist: artists)
	{
		const auto searchString = Library::Utils::convertSearchstring(artist.name());

		module()->update(
			"artists",
			{
				{"cissearch", Util::convertNotNull(searchString)}
			},
			{"artistID", artist.id()},
			"Cannot update artist cissearch");
	}

	module()->db().commit();
}

void Artists::deleteAllArtists()
{
	module()->runQuery("DELETE FROM artists;", "Could not delete all artists");
}

