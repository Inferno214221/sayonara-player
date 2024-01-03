/* DatabaseLibrary.cpp */

/* Copyright (C) 2011-2024 Michael Lugmair (Lucio Carreras)
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
#include "Database/Library.h"
#include "Database/Albums.h"
#include "Database/Artists.h"
#include "Database/Tracks.h"

#include "Utils/Utils.h"
#include "Utils/Algorithm.h"
#include "Utils/MetaData/MetaData.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/MetaData/Album.h"
#include "Utils/MetaData/Artist.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Library/LibraryInfo.h"

#include <QList>

namespace
{
	struct InfoOrder
	{
		int index;
		::Library::Info value;
	};

	QList<::Library::Info> orderedInfos(QList<InfoOrder> orders)
	{
		std::sort(orders.begin(), orders.end(), [](const InfoOrder& order1, const InfoOrder& order2) {
			return (order1.index < order2.index);
		});

		QList<::Library::Info> infos;
		Util::Algorithm::transform(orders, infos, [](const auto& order) {
			return order.value;
		});

		return infos;
	}
}

DB::Library::Library(const QString& connectionName, DbId databaseId) :
	Module(connectionName, databaseId) {}

DB::Library::~Library() = default;

QList<::Library::Info> DB::Library::getAllLibraries()
{
	const auto query = QStringLiteral("SELECT libraryID, libraryName, libraryPath, libraryIndex FROM Libraries;");

	QList<InfoOrder> orders;

	auto q = runQuery(query, "Cannot fetch all libraries");
	while(q.next())
	{
		const auto libraryId = q.value(0).value<LibraryId>();
		const auto name = q.value(1).toString();
		const auto path = q.value(2).toString();

		const auto order = InfoOrder {
			q.value(3).toInt(),
			::Library::Info(name, path, libraryId)
		};

		orders.push_back(order);
	}

	return orderedInfos(orders);
}

bool
DB::Library::insertLibrary(const LibraryId id, const QString& libraryName, const QString& libraryPath, const int index)
{
	if(libraryName.isEmpty() || libraryPath.isEmpty())
	{
		spLog(Log::Warning, this) << "Cannot insert library: Invalid parameters";
		return false;
	}

	const auto errorString = QString("Cannot insert library (name: %1, path: %2)")
		.arg(libraryName)
		.arg(libraryPath);

	const auto q = insert("Libraries",
	                      {
		                      {"libraryID",    id},
		                      {"libraryName",  Util::convertNotNull(libraryName)},
		                      {"libraryPath",  Util::convertNotNull(libraryPath)},
		                      {"libraryIndex", index}
	                      }, errorString);

	return !hasError(q);
}

bool DB::Library::editLibrary(const LibraryId libraryId, const QString& newName, const QString& newPath)
{
	if(newName.isEmpty() || newPath.isEmpty())
	{
		spLog(Log::Warning, this) << "Cannot update library: Invalid parameters";
		return false;
	}

	const auto errorString = QString("Cannot update library (name: %1, path: %2)")
		.arg(newName)
		.arg(newPath);

	const auto q = update("Libraries",
	                      {
		                      {"libraryName", Util::convertNotNull(newName)},
		                      {"libraryPath", Util::convertNotNull(newPath)}
	                      },
	                      {"libraryID", libraryId},
	                      errorString);

	return wasUpdateSuccessful(q);
}

bool DB::Library::removeLibrary(const LibraryId libraryId)
{
	const auto sql = QStringLiteral("DELETE FROM Libraries WHERE libraryID=:libraryId;");
	const auto q = runQuery(sql,
	                        {":libraryId", libraryId},
	                        QString("Cannot remove library %1").arg(libraryId));

	return !hasError(q);
}

bool DB::Library::reorderLibraries(const QMap<LibraryId, int>& order)
{
	if(order.isEmpty())
	{
		spLog(Log::Warning, this) << "Cannot reorder library: Invalid parameters";
		return false;
	}

	db().transaction();

	auto success = true;
	for(auto it = order.cbegin(); it != order.cend(); it++)
	{
		const auto q = update("Libraries",
		                      {{"libraryIndex", it.value()}},
		                      {"libraryID", it.key()},
		                      "Cannot reorder libraries");

		if(hasError(q))
		{
			success = false;
			break;
		}
	}

	if(success)
	{
		db().commit();
	}

	else
	{
		db().rollback();
	}

	return success;
}

void DB::Library::addAlbumArtists()
{
	runQuery("UPDATE tracks SET albumArtistID = artistID WHERE albumArtistID = -1;",
	         "Cannot add album artists");
}

void DB::Library::dropIndexes()
{
	const auto indexes = QStringList {
		"album_search",
		"artist_search",
		"track_search"
	};

	for(const auto& index: indexes)
	{
		const auto sql = QString("DROP INDEX IF EXISTS %1;").arg(index);
		runQuery(sql, QString("Cannot drop index %1").arg(index));
	}
}

void DB::Library::createIndexes()
{
	dropIndexes();

	struct IndexDescription
	{
		QString name;
		QString table;
		QString column;
	};

	const auto indexes = QList<IndexDescription> {
		{"album_search",  "albums",  "albumID"},
		{"artist_search", "artists", "artistID"},
		{"track_search",  "tracks",  "trackID"}};

	for(const auto& index: indexes)
	{
		const auto sql = QString("CREATE INDEX %1 ON %2 (cissearch, %3);")
			.arg(index.name)
			.arg(index.table)
			.arg(index.column);

		runQuery(sql, QString("Cannot create index %1").arg(index.name));
	}
}

