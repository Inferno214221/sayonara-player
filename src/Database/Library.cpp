/* DatabaseLibrary.cpp */

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
#include "Database/Library.h"
#include "Database/Albums.h"
#include "Database/Artists.h"
#include "Database/Tracks.h"

#include "Utils/Utils.h"
#include "Utils/MetaData/MetaData.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/MetaData/Album.h"
#include "Utils/MetaData/Artist.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Library/LibraryInfo.h"

#include <QList>

using DB::Query;

struct DB::Library::Private {};

DB::Library::Library(const QString& connection_name, DbId db_id) :
	Module(connection_name, db_id)
{}

DB::Library::~Library() = default;


template<typename T>
struct Order
{
	int index;
	T value;
};

using InfoOrder=Order<::Library::Info>;

QList<::Library::Info> DB::Library::get_all_libraries()
{
	QString query = "SELECT libraryID, libraryName, libraryPath, libraryIndex FROM Libraries;";

	QList<::Library::Info> infos;
	QList<InfoOrder> orders;

	Query q(this);
	q.prepare(query);

	bool success = q.exec();

	if(!success)
	{
		q.show_error("Cannot fetch all libraries");
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

	if(orders.size() == 0){
		return QList<::Library::Info>();
	}

	else if(orders.size() == 1){
		infos << orders.first().value;
	}

	else {

		std::sort(orders.begin(), orders.end(), [](const InfoOrder& order1, const InfoOrder& order2){
			return (order1.index < order2.index);
		});


		for(const InfoOrder& order : ::Util::AsConst(orders)){
			infos << order.value;
		}
	}

	return infos;
}

bool DB::Library::insert_library(LibraryId id, const QString& library_name, const QString& library_path, int index)
{
	if(library_name.isEmpty() || library_path.isEmpty())
	{
		sp_log(Log::Warning, this) << "Cannot insert library: Invalid parameters";
		return false;
	}

	QString query = "INSERT INTO Libraries "
					"(libraryID, libraryName, libraryPath, libraryIndex) "
					"VALUES "
					"(:library_id, :library_name, :library_path, :library_index);";

	Query q(this);

	q.prepare(query);
	q.bindValue(":library_id",		id);
	q.bindValue(":library_name",	Util::cvt_not_null(library_name));
	q.bindValue(":library_path",	Util::cvt_not_null(library_path));
	q.bindValue(":library_index",	index);

	bool success = q.exec();

	if(!success)
	{
		q.show_error
		(
			QString("Cannot insert library (name: %1, path: %2)")
					.arg(library_name, library_path)
		);
	}

	return success;
}

bool DB::Library::edit_library(LibraryId library_id, const QString& new_name, const QString& new_path)
{
	if(new_name.isEmpty() || new_path.isEmpty())
	{
		sp_log(Log::Warning, this) << "Cannot update library: Invalid parameters";
		return false;
	}

	QString query = "UPDATE Libraries "
					"SET "
					"libraryName=:library_name, "
					"libraryPath=:library_path "
					"WHERE "
					"libraryID=:library_id;";

	Query q(this);

	q.prepare(query);
	q.bindValue(":library_name",	Util::cvt_not_null(new_name));
	q.bindValue(":library_path",	Util::cvt_not_null(new_path));
	q.bindValue(":library_id",		library_id);

	bool success = q.exec();

	if(!success)
	{
		q.show_error(
			QString("Cannot update library (name: %1, path: %2)")
					.arg(new_name, new_path)
		);
	}

	return success;
}

bool DB::Library::remove_library(LibraryId library_id)
{
	QString query = "DELETE FROM Libraries WHERE libraryID=:library_id;";

	Query q(this);

	q.prepare(query);
	q.bindValue(":library_id", library_id);

	bool success = q.exec();

	if(!success)
	{
		q.show_error(
			QString("Cannot remove library %1").arg(library_id)
		);
	}

	return success;
}

bool DB::Library::reorder_libraries(const QMap<LibraryId, int>& order)
{
	if(order.isEmpty())
	{
		sp_log(Log::Warning, this) << "Cannot reorder library: Invalid parameters";
		return false;
	}

	bool success = true;
	for(auto it=order.cbegin(); it != order.cend(); it++)
	{
		QString query = "UPDATE Libraries "
						"SET "
						"libraryIndex=:index "
						"WHERE "
						"libraryID=:library_id;";

		Query q(this);
		q.prepare(query);
		q.bindValue(":index",		it.value());
		q.bindValue(":library_id",	it.key());

		success = (success && q.exec());

		if(!success)
		{
			q.show_error("Cannot reorder libraries");

		}
	}

	return success;
}

void DB::Library::add_album_artists()
{
	Query q(this);
	QString	querytext = "UPDATE tracks SET albumArtistID = artistID WHERE albumArtistID = -1;";
	q.prepare(querytext);
	if(!q.exec()){
		q.show_error("Cannot add album artists");
	}
}

void DB::Library::drop_indexes()
{
	QStringList indexes;
	indexes << "album_search";
	indexes << "artist_search";
	indexes << "track_search";
	//indexes << "track_file_search";

	for(const QString& idx : ::Util::AsConst(indexes))
	{
		Query q(this);
		QString text = "DROP INDEX IF EXISTS " + idx + ";";
		q.prepare(text);
		if(!q.exec()){
			q.show_error("Cannot drop index " + idx);
		}
	}
}

using IndexDescription=std::tuple<QString, QString, QString>;
void DB::Library::create_indexes()
{
	drop_indexes();

	QList<IndexDescription> indexes;

	indexes << std::make_tuple("album_search", "albums", "albumID");
	indexes << std::make_tuple("artist_search", "artists", "artistID");
	indexes << std::make_tuple("track_search", "tracks", "trackID");
	//indexes << std::make_tuple("track_file_search", "tracks", "trackID");

	for(const IndexDescription& idx : ::Util::AsConst(indexes))
	{
		Query q(this);
		QString name = std::get<0>(idx);
		QString table = std::get<1>(idx);
		QString column = std::get<2>(idx);
		QString text = "CREATE INDEX " + name + " ON " + table + " (cissearch, " + column + ");";
		q.prepare(text);
		if(!q.exec()){
			q.show_error("Cannot create index " + name);
		}
	}
}

