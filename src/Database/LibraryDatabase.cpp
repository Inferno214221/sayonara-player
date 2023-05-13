/* LibraryDatabase.cpp */

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

	AlbumHash calcAlbumHash(const QString& albumName, const QString& albumArtist)
	{
		return albumName.toLower() + albumArtist.toLower();
	}

	AlbumHash calcAlbumHash(const Album& album)
	{
		return calcAlbumHash(album.name(), album.albumArtist());
	}

	QMap<QString, MetaData> createTrackMap(const MetaDataList& tracks)
	{
		auto result = QMap<QString, MetaData> {};
		for(const auto& track: tracks)
		{
			result[track.filepath()] = track;
		}

		return result;
	}
}

struct LibraryDatabase::Private
{
	QString artistIdField {"artistID"};
	QString artistNameField {"artistName"};
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
	DB::Albums(),
	DB::Artists(),
	DB::Tracks(),
	DB::Module(connectionName, databaseId)
{
	m = Pimpl::make<Private>(connectionName, databaseId, libraryId);

	DB::Tracks::initViews();
	DB::Albums::initViews();

	{ // set artistId field
		AbstrSetting* s = Settings::instance()->setting(SettingKey::Lib_ShowAlbumArtists);
		QString dbKey = s->dbKey();

		auto q = QSqlQuery(DB::Module::db());
		const auto querytext = "SELECT value FROM settings WHERE key = '" + dbKey + "';";

		auto showAlbumArtists = false;

		q.prepare(querytext);
		if(q.exec())
		{
			if(q.next())
			{
				QVariant var = q.value("value");
				showAlbumArtists = var.toBool();
			}
		}

		if(showAlbumArtists)
		{
			changeArtistIdField(LibraryDatabase::ArtistIDField::AlbumArtistID);
		}

		else
		{
			changeArtistIdField(LibraryDatabase::ArtistIDField::ArtistID);
		}
	}
}

LibraryDatabase::~LibraryDatabase() = default;

void LibraryDatabase::changeArtistIdField(LibraryDatabase::ArtistIDField field)
{
	if(field == LibraryDatabase::ArtistIDField::AlbumArtistID)
	{
		m->artistIdField = "albumArtistID";
		m->artistNameField = "albumArtistName";
	}

	else
	{
		m->artistIdField = "artistID";
		m->artistNameField = "artistName";
	}
}

QString LibraryDatabase::artistIdField() const
{
	return m->artistIdField;
}

QString LibraryDatabase::artistNameField() const
{
	return m->artistNameField;
}

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
	auto currentSearchModeMask = GetSetting(Set::Lib_SearchMode);

	if(m->searchMode != currentSearchModeMask)
	{
		DB::Albums::updateAlbumCissearch();
		DB::Artists::updateArtistCissearch();
		DB::Tracks::updateTrackCissearch();
	}

	m->searchMode = currentSearchModeMask;
}

MetaDataList LibraryDatabase::insertMissingArtistsAndAlbums(const MetaDataList& tracks)
{
	if(tracks.isEmpty())
	{
		return tracks;
	}

	spLog(Log::Develop, this) << " Search for already known albums and artists.";

	// gather all albums in a map
	QHash<AlbumHash, Album> albumMap;
	{
		AlbumList albums;
		DB::Albums::getAllAlbums(albums, true);

		for(const auto& album: albums)
		{
			const auto hash = calcAlbumHash(album);
			albumMap[hash] = album;
		}
	}

	// gather all artists in a map
	QHash<QString, Artist> artistMap;
	{
		ArtistList artists;
		DB::Artists::getAllArtists(artists, true);

		for(const auto& artist: artists)
		{
			artistMap[artist.name()] = artist;
		}
	}

	// gather all metadata in a map
	MetaDataList knownTracks;
	DB::Tracks::getAllTracks(knownTracks);
	const auto trackMap = createTrackMap(knownTracks);

	db().transaction();

	auto ret = tracks;
	for(auto& track: ret)
	{
		if(track.libraryId() < 0)
		{
			track.setLibraryid(m->libraryId);
		}

		{ // check album id
			const auto hash = calcAlbumHash(track.album(), {track.albumArtist()});
			auto album = albumMap[hash];
			if(album.id() < 0)
			{
				const auto id = DB::Albums::insertAlbumIntoDatabase(track.album());
				spLog(Log::Debug, this) << "Insert new album " << hash << " (" << track.album() << "): " << id;
				album.setId(id);
				albumMap[hash] = album;
			}

			track.setAlbumId(album.id());
		}

		{ // check artist id
			auto artist = artistMap[track.artist()];
			if(artist.id() < 0)
			{
				const auto id = DB::Artists::insertArtistIntoDatabase(track.artist());
				spLog(Log::Debug, this) << "Insert new artist " << track.artist() << ": " << id;
				artist.setId(id);
				artistMap[track.artist()] = artist;
			}

			track.setArtistId(artist.id());
		}

		{ // check album artist ...
			auto albumArtist = artistMap[track.albumArtist()];
			if(albumArtist.id() < 0)
			{
				const auto id = DB::Artists::insertArtistIntoDatabase(track.albumArtist());
				spLog(Log::Debug, this) << "Insert new albumArtist " << track.albumArtist() << ": " << id;
				albumArtist.setId(id);
				artistMap[track.albumArtist()] = albumArtist;
			}

			track.setAlbumArtistId(albumArtist.id());
		}

		{ // check track id
			const auto id = trackMap[track.filepath()].id();
			track.setId(id);
		}
	}

	db().commit();

	return ret;
}

bool LibraryDatabase::fixEmptyAlbums()
{
	AlbumId id = DB::Albums::insertAlbumIntoDatabase(QString(""));

	const QStringList queries {
		QString("UPDATE tracks SET albumID=:albumID WHERE albumID IN (SELECT albumID FROM albums WHERE name IS NULL);"),
		QString("UPDATE tracks SET albumID=:albumID WHERE albumID NOT IN (SELECT albumID FROM albums);"),
		QString("DELETE FROM artists WHERE name IS NULL;")
	};

	db().transaction();
	for(const auto& query: queries)
	{
		auto q = QSqlQuery(db());
		q.prepare(query);
		q.bindValue(":albumID", id);
		bool success = q.exec();
		if(!success)
		{
			db().rollback();
			return false;
		}
	}

	return db().commit();
}

DB::Module* LibraryDatabase::module()
{
	return this;
}

const DB::Module* LibraryDatabase::module() const
{
	return this;
}

void LibraryDatabase::clear()
{
	DB::Tracks::deleteAllTracks(false);
	DB::Albums::deleteAllAlbums();
	DB::Artists::deleteAllArtists();
}

LibraryId LibraryDatabase::libraryId() const
{
	return m->libraryId;
}

bool DB::LibraryDatabase::storeMetadata(const MetaDataList& tracks)
{
	if(tracks.isEmpty())
	{
		return true;
	}

	MetaDataList modifiedTracks = insertMissingArtistsAndAlbums(tracks);

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

		// check, if the track was known before
		{
			if(track.id() < 0)
			{
				DB::Tracks::insertTrackIntoDatabase(track, track.artistId(), track.albumId(), track.albumArtistId());
			}

			else
			{
				DB::Tracks::updateTrack(track);
			}
		}
	}

	spLog(Log::Develop, this) << "Commit " << tracks.size() << " tracks to database";

	return db().commit();
}
