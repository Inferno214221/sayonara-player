/* LocalLibraryDatabase.cpp, (Created on 04.03.2024) */

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
#include "LibraryDatabaseProvider.h"
#include "DatabaseUtils.h"

#include "Database/Connector.h"
#include "Database/LibraryDatabase.h"
#include "Utils/Algorithm.h"
#include "Utils/MetaData/MetaData.h"
#include "Utils/MetaData/MetaDataList.h"

namespace
{
	MetaData createTestTrack(const Test::MetaDataBlock& data, const QString& libraryPath)
	{
		auto track = MetaData {};
		track.setTitle(data.title);
		track.setAlbum(data.album);
		track.setArtist(data.artist);

		const auto path = QString("%1/%2/%3/%4.mp3")
			.arg(libraryPath)
			.arg(data.artist)
			.arg(data.album)
			.arg(data.title);
		track.setFilepath(path);

		return track;
	}

	void cleanLibraryDatabase(DB::LibraryDatabase* db)
	{
		auto tracks = MetaDataList {};
		db->getAllTracks(tracks);
		auto ids = IdList {};
		Util::Algorithm::transform(tracks, ids, [](const auto& track) {
			return track.id();
		});

		Test::DB::deleteAllAlbums(db);
		Test::DB::deleteAllArtists(db);
		Test::DB::deleteAllTracks(db);
	}

	::DB::LibraryDatabase* createLibraryDatabase(const LibraryId libraryId)
	{
		auto* dbConnector = ::DB::Connector::instance();
		dbConnector->deleteLibraryDatabase(libraryId);
		dbConnector->registerLibraryDatabase(libraryId);

		auto* db = dbConnector->libraryDatabase(libraryId, 0);
		cleanLibraryDatabase(db);

		return db;
	}
}

namespace Test
{
	LibraryDatabaseProvider::LibraryDatabaseProvider(
		const LibraryId libraryId,
		const QString& libraryPath,
		const QList<MetaDataBlock>& data,
		const ::DB::ArtistIdInfo::ArtistIdField artistIdField) :
		m_libraryDatabase {createLibraryDatabase(libraryId)}
	{
		m_libraryDatabase->changeArtistIdField(artistIdField);

		auto tracks = MetaDataList {};
		Util::Algorithm::transform(data, tracks, [&](const auto& dataItem) {
			return createTestTrack(dataItem, libraryPath);
		});

		m_libraryDatabase->storeMetadata(tracks);

		const auto trackCount = m_libraryDatabase->getNumTracks();
		if(trackCount != data.count())
		{
			const auto errorMessage = QString("Wrong number of entries in DB: actual %1 vs desired %2")
				.arg(trackCount)
				.arg(data.count());
			throw std::runtime_error {errorMessage.toStdString()};
		}
	}

	LibraryDatabaseProvider::~LibraryDatabaseProvider()
	{
		cleanLibraryDatabase(m_libraryDatabase);
	}

	::DB::LibraryDatabase* LibraryDatabaseProvider::libraryDatabase() const { return m_libraryDatabase; }
} // Test