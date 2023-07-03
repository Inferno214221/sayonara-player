/* SmartPlaylists.cpp */
/*
 * Copyright (C) 2011-2022 Michael Lugmair
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

#include "SmartPlaylists.h"
#include "Query.h"

namespace DB
{
	namespace
	{
		constexpr const auto IdField = "id";
		constexpr const auto AttributesField = "attributes";
		constexpr const auto ClassTypeField = "classType";
		constexpr const auto RandomizedField = "isRandomized";
		constexpr const auto LibraryIdField = "libraryId";
	}

	SmartPlaylists::SmartPlaylists(const QString& connectionName, const DbId databaseId) :
		Module(connectionName, databaseId) {}

	QList<SmartPlaylistDatabaseEntry> SmartPlaylists::getAllSmartPlaylists() const
	{
		const auto querytext = QString(
			"SELECT %1, %2, %3, %4, %5 "
			"FROM SmartPlaylists")
			.arg(IdField)
			.arg(ClassTypeField)
			.arg(AttributesField)
			.arg(RandomizedField)
			.arg(LibraryIdField);

		auto query = runQuery(querytext, "Cannot fetch Smart Playlists");
		if(hasError(query))
		{
			return {};
		}

		auto result = QList<SmartPlaylistDatabaseEntry> {};

		while(query.next())
		{
			auto item = SmartPlaylistDatabaseEntry {
				query.value(0).toInt(),
				query.value(1).toString(),
				query.value(2).toString(),
				query.value(3).toBool(),
				static_cast<LibraryId>(query.value(4).toInt())
			};

			result << std::move(item);
		}

		return result;
	}

	int SmartPlaylists::insertSmartPlaylist(const SmartPlaylistDatabaseEntry& smartPlaylistDatabaseEntry)
	{
		const auto query = insert("SmartPlaylists", {
			{ClassTypeField,  smartPlaylistDatabaseEntry.classType},
			{AttributesField, smartPlaylistDatabaseEntry.attributes},
			{RandomizedField, smartPlaylistDatabaseEntry.isRandomized},
			{LibraryIdField,  smartPlaylistDatabaseEntry.libraryId}
		}, "Cannot insert into Smart Playlists");

		return (!hasError(query))
		       ? query.lastInsertId().toInt()
		       : -1;
	}

	bool SmartPlaylists::updateSmartPlaylist(const int id, const SmartPlaylistDatabaseEntry& smartPlaylistDatabaseEntry)
	{
		const auto q =
			update("SmartPlaylists", {
				       {ClassTypeField,  smartPlaylistDatabaseEntry.classType},
				       {AttributesField, smartPlaylistDatabaseEntry.attributes},
				       {RandomizedField, smartPlaylistDatabaseEntry.isRandomized},
				       {LibraryIdField,  smartPlaylistDatabaseEntry.libraryId}
			       }, {IdField, id},
			       "Cannot update Smart Playlists");

		return wasUpdateSuccessful(q);
	}

	bool SmartPlaylists::deleteSmartPlaylist(const int id)
	{
		const auto query = runQuery("DELETE FROM SmartPlaylists WHERE id = :id;",
		                            {{":id", id}},
		                            QString("Cannot delete playlist %1").arg(id));

		return !hasError(query);
	}
}