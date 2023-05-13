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
using DB::Query;

using SMM=::Library::SearchModeMask;

struct LibraryDatabase::Private
{
	QString artistIdField;
	QString artistNameField;
	QString connectionName;

	SMM		searchMode;
	DbId	databaseId;

	LibraryId libraryId;

	Private(const QString& connectionName, DbId databaseId, LibraryId libraryId) :
		connectionName(connectionName),
		databaseId(databaseId),
		libraryId(libraryId)
	{
		searchMode = GetSetting(Set::Lib_SearchMode);

		artistIdField = "artistID";
		artistNameField = "artistName";
	}
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

		Query q(connectionName, databaseId);
		QString querytext = "SELECT value FROM settings WHERE key = '" + dbKey + "';";

		bool show_album_artists = false;

		q.prepare(querytext);
		if(q.exec())
		{
			if(q.next())
			{
				QVariant var = q.value("value");
				show_album_artists = var.toBool();
			}
		}

		if(show_album_artists) {
			changeArtistIdField(LibraryDatabase::ArtistIDField::AlbumArtistID);
		}

		else {
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
	if(m->libraryId < 0) {
		return "tracks";
	}

	else {
		return QString("track_view_%1").arg(m->libraryId);
	}
}

QString LibraryDatabase::trackSearchView() const
{
	if(m->libraryId < 0) {
		return "track_search_view";
	}

	else {
		return QString("track_search_view_%1").arg(m->libraryId);
	}
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

using AlbumHash=QString;
static AlbumHash calcAlbumHash(const QString& albumName, const QString& albumArtist)
{
	return albumName.toLower() + albumArtist.toLower();
}
static AlbumHash calcAlbumHash(const Album& album)
{
	return calcAlbumHash(album.name(), album.albumArtist());
}

MetaDataList LibraryDatabase::insertMissingArtistsAndAlbums(const MetaDataList& tracks)
{
	if(tracks.isEmpty()){
		return tracks;
	}

	spLog(Log::Develop, this) << " Search for already known albums and artists.";

	// gather all albums in a map
	QHash<AlbumHash, Album> albumMap;
	{
		AlbumList albums;
		DB::Albums::getAllAlbums(albums, true);

		for(const Album& album : albums)
		{
			AlbumHash hash = calcAlbumHash(album);
			albumMap[hash] = album;
		}
	}

	// gather all artists in a map
	QHash<QString, Artist> artistMap;
	{
		ArtistList artists;
		DB::Artists::getAllArtists(artists, true);

		for(const Artist& artist : artists){
			artistMap[artist.name()] = artist;
		}
	}

	// gather all metadata in a map
	QHash<QString, MetaData> trackMap;
	{
		MetaDataList knownTracks;
		DB::Tracks::getAllTracks(knownTracks);
		for(const MetaData& md : knownTracks) {
			trackMap[md.filepath()] = md;
		}
	}

	db().transaction();

	MetaDataList ret(tracks);
	for(MetaData& md : ret)
	{
		if(md.libraryId() < 0) {
			md.setLibraryid(m->libraryId);
		}

		{ // check album id
			AlbumHash hash = calcAlbumHash(md.album(), {md.albumArtist()});
			Album album = albumMap[hash];
			if(album.id() < 0)
			{
				AlbumId id = DB::Albums::insertAlbumIntoDatabase(md.album());
				spLog(Log::Debug, this) << "Insert new album " << hash << " (" << md.album() << "): " << id;
				album.setId(id);
				albumMap[hash] = album;
			}

			md.setAlbumId(album.id());
		}

		{ // check artist id
			Artist artist = artistMap[md.artist()];
			if(artist.id() < 0)
			{
				ArtistId id = DB::Artists::insertArtistIntoDatabase(md.artist());
				spLog(Log::Debug, this) << "Insert new artist " << md.artist() << ": " << id;
				artist.setId(id);
				artistMap[md.artist()] = artist;
			}

			md.setArtistId(artist.id());
		}

		{ // check album artist ...
			Artist albumArtist = artistMap[md.albumArtist()];
			if(albumArtist.id() < 0)
			{
				ArtistId id = DB::Artists::insertArtistIntoDatabase(md.albumArtist());
				spLog(Log::Debug, this) << "Insert new albumArtist " << md.albumArtist() << ": " << id;
				albumArtist.setId(id);
				artistMap[md.albumArtist()] = albumArtist;
			}

			md.setAlbumArtistId(albumArtist.id());
		}

		{ // check track id
			TrackID id = trackMap[md.filepath()].id();
			md.setId(id);
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
	for(const QString& query : queries)
	{
		auto q = QSqlQuery(db());
		q.prepare(query);
		q.bindValue(":albumID", id);
		bool success = q.exec();
		if(!success){
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
	if(tracks.isEmpty()) {
		return true;
	}

	MetaDataList modifiedTracks = insertMissingArtistsAndAlbums(tracks);

	db().transaction();

	for(const MetaData& md : modifiedTracks)
	{
		// because all artists and albums should be in the db right now,
		// we should never reach the inner block
		if(md.albumId() < 0 || md.artistId() < 0 || md.libraryId() < 0)
		{
			spLog(Log::Warning, this) << "Cannot insert artist or album of " << md.filepath();
			continue;
		}

		// check, if the track was known before
		{
			if(md.id() < 0) {
				DB::Tracks::insertTrackIntoDatabase(md, md.artistId(), md.albumId(), md.albumArtistId());
			}

			else {
				DB::Tracks::updateTrack(md);
			}
		}
	}

	spLog(Log::Develop, this) << "Commit " << tracks.size() << " tracks to database";

	return db().commit();
}
