/* Connector.h */

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

#ifndef DatabaseConnector_H
#define DatabaseConnector_H

#include "Database/Base.h"

#include "Utils/Singleton.h"
#include "Utils/Pimpl.h"

#define INDEX_SIZE 3

namespace DB
{
	class LibraryDatabase;
	class Bookmarks;
	class Equalizer;
	class Playlist;
	class LibraryDatabase;
	class Podcasts;
	class Streams;
	class VisualStyles;
	class Settings;
	class Library;
	class Shortcuts;
	class Covers;
	class Session;
	class SmartPlaylists;

	using LibraryDatabases = QList<LibraryDatabase*>;

	class Connector :
		public Base
	{
		PIMPL(Connector)

		public:
			Connector(const QString& sourceDirectory, const QString& targetDirectory, const QString& databseFilename);
			~Connector() override;

			static Connector* instance();
			static Connector* customInstance(QString sourceDirectory, QString targetDirectory, QString databseFilename);

			[[nodiscard]] LibraryDatabases libraryDatabases() const;
			DB::LibraryDatabase* libraryDatabase(LibraryId libraryId, DbId databaseId);
			DB::LibraryDatabase* registerLibraryDatabase(LibraryId libraryId);
			void deleteLibraryDatabase(LibraryId libraryId);

			DB::Bookmarks* bookmarkConnector();
			DB::Equalizer* equalizerConnector();
			DB::Playlist* playlistConnector();
			DB::Podcasts* podcastConnector();
			DB::Streams* streamConnector();
			DB::VisualStyles* visualStyleConnector();
			DB::Settings* settingsConnector();
			DB::Shortcuts* shortcutConnector();
			DB::Covers* coverConnector();
			DB::Library* libraryConnector();
			DB::Session* sessionConnector();
			DB::SmartPlaylists* smartPlaylistsConnector();
	};
}
#endif // DatabaseConnector_H
