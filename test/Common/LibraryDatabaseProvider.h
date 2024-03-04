/* LocalLibraryDatabase.h, (Created on 04.03.2024) */

/* Copyright (C) 2011-2024 Michael Lugmair
 *
 * This file is part of Sayonara Player
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
#ifndef SAYONARA_PLAYER_LOCALLIBRARYDATABASE_H
#define SAYONARA_PLAYER_LOCALLIBRARYDATABASE_H

#include <QString>
#include "Database/LibraryDatabase.h"

class MetaData;

namespace Test
{
	struct MetaDataBlock
	{
		QString album;
		QString artist;
		QString title;
	};

	class LibraryDatabaseProvider
	{
		public:
			LibraryDatabaseProvider(
				LibraryId libraryId,
				const QString& libraryPath,
				const QList<MetaDataBlock>& data,
				::DB::ArtistIdInfo::ArtistIdField artistIdField);

			~LibraryDatabaseProvider();

			[[nodiscard]] ::DB::LibraryDatabase* libraryDatabase() const;

		private:
			::DB::LibraryDatabase* m_libraryDatabase;
	};

} // Test

#endif //SAYONARA_PLAYER_LOCALLIBRARYDATABASE_H
