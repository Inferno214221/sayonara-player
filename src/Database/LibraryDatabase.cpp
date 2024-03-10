/* LibraryDatabase.cpp */

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

#include "LibraryDatabase.h"
#include "Database/Query.h"
#include "Database/CoverConnector.h"
#include "Utils/Settings/Settings.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/MetaData/Album.h"
#include "Utils/MetaData/Artist.h"
#include "Utils/Set.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Library/SearchMode.h"

using DB::LibraryDatabase;

namespace
{
	using AlbumHash = QString;

	AlbumHash calcAlbumHash(const QString& albumName, const QString& albumArtist, const Year year)
	{
		return albumName.toLower() + albumArtist.toLower() + QString::number(year);
	}

	AlbumHash calcAlbumHash(const Album& album)
	{
		return calcAlbumHash(album.name(), album.albumArtist(), album.year());
	}

	QHash<QString, Artist> createArtistMap(LibraryDatabase* libraryDatabase)
	{
		auto result = QHash<QString, Artist> {};
		auto artists = ArtistList {};
		libraryDatabase->getAllArtists(artists, true);
		for(const auto& artist: artists)
		{
			result[artist.name()] = artist;
		}

		return result;
	}

	QHash<AlbumHash, Album> createAlbumMap(LibraryDatabase* libraryDatabase)
	{
		auto result = QHash<AlbumHash, Album> {};
		auto albums = AlbumList {};
		libraryDatabase->getAllAlbums(albums, true);

		for(const auto& album: albums)
		{
			const auto hash = calcAlbumHash(album);
			result[hash] = album;
		}

		return result;
	}

	QMap<QString, MetaData> createTrackMap(LibraryDatabase* libraryDatabase)
	{
		auto result = QMap<QString, MetaData> {};
		auto tracks = MetaDataList {};
		libraryDatabase->getAllTracks(tracks);
		for(const auto& track: tracks)
		{
			result[track.filepath()] = track;
		}

		return result;
	}

	DB::ArtistIdInfo createArtistIdInfo(const DB::ArtistIdInfo::ArtistIdField artistIdField)
	{
		return (artistIdField == DB::ArtistIdInfo::ArtistIdField::ArtistId)
		       ? DB::ArtistIdInfo {artistIdField, "artistId", "artistName"}
		       : DB::ArtistIdInfo {artistIdField, "albumArtistId", "albumArtistName"};
	}
}

struct LibraryDatabase::Private
{
	ArtistIdInfo artistIdInfo {createArtistIdInfo(ArtistIdInfo::ArtistIdField::ArtistId)};
	QString connectionName;

	::Library::SearchModeMask searchMode;
	DbId databaseId;

	LibraryId libraryId;

	Private(QString connectionName, const DbId databaseId, const LibraryId libraryId) :
		connectionName {std::move(connectionName)},
		searchMode {GetSetting(Set::Lib_SearchMode)},
		databaseId {databaseId},
		libraryId {libraryId} {}
};

LibraryDatabase::LibraryDatabase(const QString& connectionName, DbId databaseId, LibraryId libraryId) :
	DB::Module(connectionName, databaseId),
	m {Pimpl::make<Private>(connectionName, databaseId, libraryId)}
{
	DB::Tracks::initViews();
	DB::Albums::initViews();

	{ // set artistId field
		const auto* s = Settings::instance()->setting(SettingKey::Lib_ShowAlbumArtists);
		const auto dbKey = s->dbKey();
		auto showAlbumArtists = false;

		auto q = runQuery("SELECT value FROM settings WHERE key = :key;",
		                  {":key", dbKey},
		                  "Cannot fetch setting " + dbKey);

		if(!DB::hasError(q))
		{
			if(q.next())
			{
				showAlbumArtists = q.value("value").toBool();
			}
		}

		if(showAlbumArtists)
		{
			changeArtistIdField(ArtistIdInfo::ArtistIdField::AlbumArtistId);
		}

		else
		{
			changeArtistIdField(ArtistIdInfo::ArtistIdField::ArtistId);
		}
	}
}

LibraryDatabase::~LibraryDatabase() = default;

void LibraryDatabase::changeArtistIdField(const ArtistIdInfo::ArtistIdField field)
{
	m->artistIdInfo = createArtistIdInfo(field);
}

DB::ArtistIdInfo LibraryDatabase::artistIdInfo() const { return m->artistIdInfo; }

QString LibraryDatabase::trackView() const
{
	return (m->libraryId < 0)
	       ? "tracks"
	       : QString("track_view_%1").arg(m->libraryId);
}

QString LibraryDatabase::trackSearchView() const
{
	return (m->libraryId < 0)
	       ? "track_search_view"
	       : QString("track_search_view_%1").arg(m->libraryId);
}

void LibraryDatabase::updateSearchMode()
{
	const auto currentSearchModeMask = GetSetting(Set::Lib_SearchMode);
	if(m->searchMode != currentSearchModeMask)
	{
		DB::Albums::updateAlbumCissearch();
		DB::Artists::updateArtistCissearch();
		DB::Tracks::updateTrackCissearch();
	}

	m->searchMode = currentSearchModeMask;
}

int LibraryDatabase::checkArtist(const QString& name, QHash<QString, Artist>& artistMap)
{
	auto artist = artistMap[name];
	if(artist.id() < 0)
	{
		const auto id = DB::Artists::insertArtistIntoDatabase(name);
		spLog(Log::Debug, this) << "Insert new artist " << name << " (" << name << "): " << id;

		artist.setName(name);
		artist.setId(id);
		artistMap[name] = artist;
	}

	return artist.id();
}

int LibraryDatabase::checkAlbum(const QString& name, const QString& albumArtist, const Year year,
                                QHash<QString, Album>& albumMap)
{
	const auto hash = calcAlbumHash(name, albumArtist, year);
	auto album = albumMap[hash];
	if(album.id() < 0)
	{
		const auto id = DB::Albums::insertAlbumIntoDatabase(name);
		spLog(Log::Debug, this) << "Insert new album " << hash << " (" << name << "): " << id;

		album.setName(name);
		album.setId(id);
		albumMap[hash] = album;
	}

	return album.id();
}

MetaDataList LibraryDatabase::insertMissingArtistsAndAlbums(const MetaDataList& tracks)
{
	if(tracks.isEmpty())
	{
		return tracks;
	}

	spLog(Log::Develop, this) << " Search for already known albums and artists.";

	auto albumMap = createAlbumMap(this);
	auto artistMap = createArtistMap(this);
	auto trackMap = createTrackMap(this);

	db().transaction();

	auto result = tracks;
	for(auto& track: result)
	{
		if(track.libraryId() < 0)
		{
			track.setLibraryid(m->libraryId);
		}

		const auto albumId = checkAlbum(track.album(), track.albumArtist(), track.year(), albumMap);
		track.setAlbumId(albumId);

		const auto artistId = checkArtist(track.artist(), artistMap);
		track.setArtistId(artistId);

		const auto albumArtistId = checkArtist(track.albumArtist(), artistMap);
		track.setAlbumArtistId(albumArtistId);

		const auto trackId = trackMap[track.filepath()].id();
		track.setId(trackId);
	}

	db().commit();

	return result;
}

bool LibraryDatabase::fixEmptyAlbums()
{
	const auto albumId = DB::Albums::insertAlbumIntoDatabase(QString(""));
	const auto queries = QStringList {
		"UPDATE tracks SET albumID=:albumID WHERE albumID IN (SELECT albumID FROM albums WHERE name IS NULL);",
		"UPDATE tracks SET albumID=:albumID WHERE albumID NOT IN (SELECT albumID FROM albums);",
		"DELETE FROM artists WHERE name IS NULL;"
	};

	db().transaction();
	for(const auto& query: queries)
	{
		const auto q = runQuery(query, {":albumID", albumId}, "Query error");
		if(DB::hasError(q))
		{
			db().rollback();
			return false;
		}
	}

	return db().commit();
}

DB::Module* LibraryDatabase::module() { return this; }

const DB::Module* LibraryDatabase::module() const { return this; }

LibraryId LibraryDatabase::libraryId() const { return m->libraryId; }

bool DB::LibraryDatabase::storeMetadata(const MetaDataList& tracks)
{
	if(tracks.isEmpty())
	{
		return true;
	}

	const auto modifiedTracks = insertMissingArtistsAndAlbums(tracks);

	db().transaction();
	for(const auto& track: modifiedTracks)
	{
		// because all artists and albums should be in the db right now,
		// we should never reach the inner block
		if(track.albumId() < 0 || track.artistId() < 0 || track.libraryId() < 0)
		{
			spLog(Log::Warning, this) << "Cannot insert artist or album of " << track.filepath();
			continue;
		}

		if(track.id() < 0)
		{
			DB::Tracks::insertTrackIntoDatabase(track, track.artistId(), track.albumId(), track.albumArtistId());
		}

		else
		{
			DB::Tracks::updateTrack(track);
		}
	}

	spLog(Log::Develop, this) << "Commit " << tracks.size() << " tracks to database";

	DB::Albums::deleteOrphanedAlbums();
	return db().commit();
}
