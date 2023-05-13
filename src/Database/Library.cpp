/* DatabaseLibrary.cpp */

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

namespace Algorithm = Util::Algorithm;

struct DB::Library::Private
{
};

DB::Library::Library(const QString& connectionName, DbId databaseId) :
	Module(connectionName, databaseId) {}

DB::Library::~Library() = default;

template<typename T>
struct Order
{
	int index;
	T value;
};

using InfoOrder = Order<::Library::Info>;

QList<::Library::Info> DB::Library::getAllLibraries()
{
	const auto query = "SELECT libraryID, libraryName, libraryPath, libraryIndex FROM Libraries;";

	QList<::Library::Info> infos;
	QList<InfoOrder> orders;

	Query q(this);
	q.prepare(query);

	bool success = q.exec();

	if(!success)
	{
		q.showError("Cannot fetch all libraries");
	}

	while(q.next())
	{
		LibraryId id = q.value(0).toInt();
		QString name = q.value(1).toString();
		QString path = q.value(2).toString();

		InfoOrder order;
		order.value = ::Library::Info(name, path, id);
		order.index = q.value(3).toInt();

		orders << order;
	}

	if(orders.empty())
	{
		return {};
	}

	if(orders.size() == 1)
	{
		infos << orders.first().value;
	}

	else
	{
		std::sort(orders.begin(), orders.end(), [](const InfoOrder& order1, const InfoOrder& order2) {
			return (order1.index < order2.index);
		});

		for(const auto& order: Algorithm::AsConst(orders))
		{
			infos << order.value;
		}
	}

	return infos;
}

bool
DB::Library::insertLibrary(const LibraryId id, const QString& libraryName, const QString& libraryPath, const int index)
{
	if(libraryName.isEmpty() || libraryPath.isEmpty())
	{
		spLog(Log::Warning, this) << "Cannot insert library: Invalid parameters";
		return false;
	}

	QString query = "INSERT INTO Libraries "
	                "(libraryID, libraryName, libraryPath, libraryIndex) "
	                "VALUES "
	                "(:libraryId, :library_name, :library_path, :library_index);";

	Query q(this);

	q.prepare(query);
	q.bindValue(":libraryId", id);
	q.bindValue(":library_name", Util::convertNotNull(libraryName));
	q.bindValue(":library_path", Util::convertNotNull(libraryPath));
	q.bindValue(":library_index", index);

	bool success = q.exec();

	if(!success)
	{
		q.showError
			(
				QString("Cannot insert library (name: %1, path: %2)")
					.arg(libraryName, libraryPath)
			);
	}

	return success;
}

bool DB::Library::editLibrary(const LibraryId libraryId, const QString& newName, const QString& newPath)
{
	if(newName.isEmpty() || newPath.isEmpty())
	{
		spLog(Log::Warning, this) << "Cannot update library: Invalid parameters";
		return false;
	}

	QString query = "UPDATE Libraries "
	                "SET "
	                "libraryName=:library_name, "
	                "libraryPath=:library_path "
	                "WHERE "
	                "libraryID=:libraryId;";

	Query q(this);

	q.prepare(query);
	q.bindValue(":library_name", Util::convertNotNull(newName));
	q.bindValue(":library_path", Util::convertNotNull(newPath));
	q.bindValue(":libraryId", libraryId);

	bool success = q.exec();

	if(!success)
	{
		q.showError(
			QString("Cannot update library (name: %1, path: %2)")
				.arg(newName, newPath)
		);
	}

	return success;
}

bool DB::Library::removeLibrary(LibraryId libraryId)
{
	QString query = "DELETE FROM Libraries WHERE libraryID=:libraryId;";

	Query q(this);

	q.prepare(query);
	q.bindValue(":libraryId", libraryId);

	bool success = q.exec();

	if(!success)
	{
		q.showError(
			QString("Cannot remove library %1").arg(libraryId)
		);
	}

	return success;
}

bool DB::Library::reorderLibraries(const QMap<LibraryId, int>& order)
{
	if(order.isEmpty())
	{
		spLog(Log::Warning, this) << "Cannot reorder library: Invalid parameters";
		return false;
	}

	bool success = true;
	for(auto it = order.cbegin(); it != order.cend(); it++)
	{
		QString query = "UPDATE Libraries "
		                "SET "
		                "libraryIndex=:index "
		                "WHERE "
		                "libraryID=:libraryId;";

		Query q(this);
		q.prepare(query);
		q.bindValue(":index", it.value());
		q.bindValue(":libraryId", it.key());

		success = (success && q.exec());

		if(!success)
		{
			q.showError("Cannot reorder libraries");

		}
	}

	return success;
}

void DB::Library::addAlbumArtists()
{
	Query q(this);
	QString querytext = "UPDATE tracks SET albumArtistID = artistID WHERE albumArtistID = -1;";
	q.prepare(querytext);
	if(!q.exec())
	{
		q.showError("Cannot add album artists");
	}
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
		Query q(this);
		QString text = "DROP INDEX IF EXISTS " + index + ";";
		q.prepare(text);
		if(!q.exec())
		{
			q.showError("Cannot drop index " + index);
		}
	}
}

using IndexDescription = std::tuple<QString, QString, QString>;

void DB::Library::createIndexes()
{
	dropIndexes();

	QList<IndexDescription> indexes;

	indexes << std::make_tuple("album_search", "albums", "albumID");
	indexes << std::make_tuple("artist_search", "artists", "artistID");
	indexes << std::make_tuple("track_search", "tracks", "trackID");
	//indexes << std::make_tuple("track_file_search", "tracks", "trackID");

	for(const IndexDescription& idx: Algorithm::AsConst(indexes))
	{
		Query q(this);
		QString name = std::get<0>(idx);
		QString table = std::get<1>(idx);
		QString column = std::get<2>(idx);
		QString text = "CREATE INDEX " + name + " ON " + table + " (cissearch, " + column + ");";
		q.prepare(text);
		if(!q.exec())
		{
			q.showError("Cannot create index " + name);
		}
	}
}

