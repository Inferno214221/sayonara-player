/* SmartPlaylist.h */
/*
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

#ifndef SAYONARA_PLAYER_SMARTPLAYLIST_H
#define SAYONARA_PLAYER_SMARTPLAYLIST_H

#include "Utils/Pimpl.h"
#include "StringConverter.h"

#include <optional>

class MetaDataList;
class QString;
struct SmartPlaylistDatabaseEntry;

namespace SmartPlaylists
{
	enum class Type
	{
		Rating = 0,
		Year,
		Created,
		CreatedRelative,
		LastPlayed,
		RandomTracks,
		RandomAlbums,
		NumEntries
	};

	enum class InputFormat
	{
		Text = 0,
		Calendar,
		TimeSpan
	};
}

class SmartPlaylist
{
	PIMPL(SmartPlaylist)

	public:
		SmartPlaylist(int id, const QList<int>& values, bool isRandomized, LibraryId libraryId);
		virtual ~SmartPlaylist();

		[[nodiscard]] int id() const;
		void setId(int id);

		[[nodiscard]] virtual int minimumValue() const = 0;
		[[nodiscard]] virtual int maximumValue() const = 0;

		[[nodiscard]] int count() const;
		[[nodiscard]] int value(int index) const;
		void setValue(int index, int value);

		[[nodiscard]] bool isRandomized() const;
		void setRandomized(bool b);
		[[nodiscard]] virtual bool isRandomizable() const;

		[[nodiscard]] LibraryId libraryId() const;
		void setLibraryId(LibraryId libraryId);

		virtual MetaDataList filterTracks(MetaDataList tracks) = 0;

		[[nodiscard]] virtual QString classType() const = 0;        // for database
		[[nodiscard]] virtual QString displayClassType() const = 0; // for ui
		[[nodiscard]] virtual QString name() const = 0;             // for plugin
		[[nodiscard]] virtual QString text(int value) const;
		[[nodiscard]] virtual SmartPlaylists::Type type() const = 0;
		[[nodiscard]] virtual SmartPlaylists::InputFormat inputFormat() const;
		[[nodiscard]] virtual bool canFetchTracks() const;

		[[nodiscard]] SmartPlaylistDatabaseEntry toDatabaseEntry() const;
		[[nodiscard]] virtual SmartPlaylists::StringConverterPtr stringConverter() const final;

	protected:
		[[nodiscard]] virtual SmartPlaylists::StringConverterPtr createConverter() const;

	private:
		[[nodiscard]] QString attributesToString() const;
};

#endif //SAYONARA_PLAYER_SMARTPLAYLIST_H
