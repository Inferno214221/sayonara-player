/* DatabasePlaylist.cpp */

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
#include "Database/Playlist.h"

#include "Utils/Logger/Logger.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Playlist/CustomPlaylist.h"
#include "Utils/MetaData/Genre.h"
#include "Utils/Utils.h"

using DB::Query;

DB::Playlist::Playlist(const QString& connection_name, DbId databaseId) :
	Module(connection_name, databaseId) {}

DB::Playlist::~Playlist() {}

bool DB::Playlist::getAllPlaylistSkeletons(CustomPlaylistSkeletons& skeletons, ::Playlist::StoreType type, ::Playlist::SortOrder sortorder)
{
	skeletons.clear();

	QString sortorderString;
	switch(sortorder){
		case ::Playlist::SortOrder::IDAsc:
			sortorderString = " ORDER BY playlists.playlistID ASC ";
			break;
		case ::Playlist::SortOrder::IDDesc:
			sortorderString = " ORDER BY playlists.playlistID DESC ";
			break;
		case ::Playlist::SortOrder::NameAsc:
			sortorderString = " ORDER BY playlists.playlist ASC ";
			break;
		case ::Playlist::SortOrder::NameDesc:
			sortorderString = " ORDER BY playlists.playlist DESC ";
			break;
		default:
			break;
	}

	QString typeClause;
	switch(type)
	{
		case ::Playlist::StoreType::OnlyTemporary:
			typeClause = " WHERE playlists.temporary = 1 ";
			break;
		case ::Playlist::StoreType::OnlyPermanent:
			typeClause = " WHERE playlists.temporary = 0 ";
			break;
		default:
			break;
	}

	const QStringList fields
	{
		"playlists.playlistID	AS playlistID",
		"playlists.playlist		AS playlistName",
		"playlists.temporary	AS temporary",
		"COUNT(ptt.trackID)		AS trackCount"
	};

	Query q = runQuery
	(
		"SELECT " + fields.join(", ") + " "
		"FROM playlists LEFT OUTER JOIN playlistToTracks ptt "
		"ON playlists.playlistID = ptt.playlistID "
		+ typeClause +
		"GROUP BY playlists.playlistID " +
		sortorderString + ";",

		"Cannot fetch all playlists"
	);

	if(q.hasError()){
		return false;
	}

	while(q.next())
	{
		CustomPlaylistSkeleton skeleton;
		if(q.value(0).isNull()){
			continue;
		}

		skeleton.setId(q.value(0).toInt());
		skeleton.setName(q.value(1).toString());

		bool temporary = (q.value(2) == 1);
		skeleton.setTemporary(temporary);
		skeleton.setTrackCount(q.value(3).toInt());

		skeletons << skeleton;
	}

	return true;
}

bool DB::Playlist::getPlaylistSkeletonById(CustomPlaylistSkeleton& skeleton)
{
	if(skeleton.id() < 0) {
		spLog(Log::Warning, this) << "Cannot fetch playlist -1";
		return false;
	}

	const QStringList fields
	{
		"playlists.playlistID		AS playlistID",
		"playlists.playlist			AS playlistName",
		"playlists.temporary		AS temporary",
		"COUNT(ptt.trackID)			AS trackCount"
	};

	Query q = runQuery
	(
		"SELECT " + fields.join(", ") + " "
		"FROM playlists LEFT OUTER JOIN playlistToTracks ptt "
		"ON playlists.playlistID = ptt.playlistID "
		"WHERE playlists.playlistid = :playlist_id "
		"GROUP BY playlists.playlistID;",

		{{":playlist_id", skeleton.id()}},
		"Cannot fetch all playlists"
	);

	if(q.hasError())
	{
		return false;
	}

	if(q.next())
	{
		skeleton.setId(q.value(0).toInt());
		skeleton.setName(q.value(1).toString());

		bool temporary = (q.value(2) == 1);
		skeleton.setTemporary(temporary);
		skeleton.setTrackCount(q.value(3).toInt());

		return true;
	}

	return false;
}

bool DB::Playlist::getPlaylistById(CustomPlaylist& pl)
{
	if(!getPlaylistSkeletonById(pl))
	{
		spLog(Log::Warning, this) << "Get playlist by id: cannot fetch skeleton id " << pl.id();
		return false;
	}

	pl.clear();

	const QStringList fields
	{
		"tracks.trackID						AS trackID",		// 0
		"tracks.title						AS title",			// 1
		"tracks.length						AS length",			// 2
		"tracks.year						AS year",			// 3
		"tracks.bitrate						AS bitrate",		// 4
		"tracks.filename					AS filename",		// 5
		"tracks.track						AS trackNum",		// 6
		"albums.albumID						AS albumID",		// 7
		"artists.artistID					AS artistID",		// 8
		"albums.name						AS albumName",		// 9
		"artists.name						AS artistName",		// 10
		"tracks.genre						AS genrename",		// 11
		"tracks.filesize					AS filesize",		// 12
		"tracks.discnumber					AS discnumber",		// 13
		"tracks.rating						AS rating",			// 14
		"ptt.filepath						AS filepath",		// 15
		"ptt.db_id							AS databaseId",		// 16
		"tracks.libraryID					AS libraryId",		// 17
		"tracks.createdate					AS createdate",		// 18
		"tracks.modifydate					AS modifydate",		// 19
		"ptt.coverDownloadUrl				AS coverDownloadUrl"// 20
	};

	Query q = runQuery
	(
		"SELECT "
		+ fields.join(", ") + " " +
		"FROM tracks, albums, artists, playlists, playlistToTracks ptt "
		"WHERE playlists.playlistID = :playlist_id "
		"AND playlists.playlistID = ptt.playlistID "
		"AND ptt.trackID = tracks.trackID "
		"AND tracks.albumID = albums.albumID "
		"AND tracks.artistID = artists.artistID "
		"ORDER BY ptt.position ASC; ",

		{{":playlist_id", pl.id()}},
		QString("Cannot get tracks for playlist %1").arg(pl.id())
	);

	if(!q.hasError())
	{
		while (q.next())
		{
			MetaData data;

			data.setId(q.value(0).toInt());
			data.setTitle(q.value(1).toString());
			data.setDurationMs(q.value(2).toInt());
			data.setYear(q.value(3).value<Year>());
			data.setBitrate(q.value(4).value<Bitrate>());
			data.setFilepath(q.value(5).toString());
			data.setTrackNumber(q.value(6).value<TrackNum>());
			data.setAlbumId(q.value(7).toInt());
			data.setArtistId(q.value(8).toInt());
			data.setAlbum(q.value(9).toString().trimmed());
			data.setArtist(q.value(10).toString().trimmed());
			QStringList genres = q.value(11).toString().split(",");
			data.setGenres(genres);
			data.setFilesize(q.value(12).value<Filesize>());
			data.setDiscnumber(q.value(13).value<Disc>());
			data.setRating(q.value(14).value<Rating>());
			if(q.value(16).toInt() == 0 || q.value(16).isNull()) {
				pl.push_back(data);
			}
			data.setLibraryid(q.value(17).value<LibraryId>());
			data.setCreatedDate(q.value(18).value<uint64_t>());
			data.setModifiedDate(q.value(19).value<uint64_t>());
			data.setCoverDownloadUrls(q.value(20).toString().split(";"));
			data.setExtern(false);
			data.setDatabaseId(databaseId());
		}
	}

	// non database playlists
	Query q2 = runQuery
	(
		"SELECT "
		"ptt.filepath			AS filepath, "
		"ptt.position			AS position, "
		"ptt.stationName		AS radioStationName, "
		"ptt.station			AS radioStation, "
		"ptt.isRadio			AS isRadio, "
		"ptt.coverDownloadUrl	AS coverDownloadUrl "
		"FROM playlists pl, playlistToTracks ptt "
		"WHERE pl.playlistID = :playlistID "
		"AND pl.playlistID = ptt.playlistID "
		"AND ptt.trackID < 0 "
		"ORDER BY ptt.position ASC;",

		{{":playlistID", pl.id()}},
		QString("Playlist by id: Cannot fetch playlist %1").arg(pl.id())
	);

	if(q2.hasError()) {
		return false;
	}

	while (q2.next())
	{
		QString		filepath =			q2.value(0).toString();
		int			position =			q2.value(1).toInt();
		QString		radioStationName =	q2.value(2).toString();
		QString		radioStation =		q2.value(3).toString();
		bool		isRadio =			q2.value(4).toBool();
		QStringList	coverUrls =			q2.value(5).toString().split(";");

		MetaData md(filepath);
		md.setId(-1);
		md.setExtern(true);
		if(isRadio) {
			md.setRadioStation(radioStation, radioStationName);
		}

		else {
			md.setTitle(filepath);
			md.setArtist(filepath);
		}

		md.setDatabaseId(databaseId());
		md.setCoverDownloadUrls(coverUrls);

		for(int row=0; row<=pl.count(); row++)
		{
			if( row >= position)
			{
				pl.insertTrack(md, row);
				break;
			}
		}
	}

	return true;
}

// negative, if error
// nonnegative else
int DB::Playlist::getPlaylistIdByName(const QString& name)
{
	Query q = runQuery
	(
		"SELECT playlistid FROM playlists WHERE playlist = :playlistName;",
		{
			{":playlistName", Util::convertNotNull(name)}
		},
		QString("Playlist by name: Cannot fetch playlist %1").arg(name)
	);

	if(q.hasError()) {
		return -1;
	}

	else
	{
		if(q.next()) {
		  return q.value(0).toInt();
		}

		return -1;
	}
}

bool DB::Playlist::insertTrackIntoPlaylist(const MetaData& md, int playlistId, int pos)
{
	if(md.isDisabled()) {
		return false;
	}

	QMap<QString, QVariant> fieldBindings
	{
		{"playlistid",			playlistId},
		{"filepath",			Util::convertNotNull(md.filepath())},
		{"position",			pos},
		{"trackid",				md.id()},
		{"db_id",				md.databaseId()},
		{"coverDownloadUrl",	md.coverDownloadUrls().join(";")}
	};

	bool isRadio =
		(md.radioMode() == RadioMode::Station) ||
		(md.radioMode() == RadioMode::Podcast);

	if(isRadio)
	{
		fieldBindings.insert("stationName",	Util::convertNotNull(md.radioStationName()));
		fieldBindings.insert("station",	Util::convertNotNull(md.radioStation()));
		fieldBindings.insert("isRadio",	isRadio);
	}

	Query q = insert("playlistToTracks", fieldBindings, "Cannot insert track into playlist");
	return (!q.hasError());
}


// returns id if everything ok
// negative otherwise
int DB::Playlist::createPlaylist(QString playlist_name, bool temporary)
{
	Query q	= insert("playlists",
	{
		{"playlist", Util::convertNotNull(playlist_name)},
		{"temporary", (temporary == true) ? 1 : 0}
	}, "Cannot create playlist");

	if(q.hasError()){
		return false;
	}

	return q.lastInsertId().toInt();
}


bool DB::Playlist::renamePlaylist(int id, const QString& new_name)
{
	Query q = update("playlists",
		{{"playlist", Util::convertNotNull(new_name)}},
		{"playlistId", id}
	, "Cannot rename playlist");

	return (!q.hasError());
}


bool DB::Playlist::storePlaylist(const MetaDataList& tracks, QString playlistName, bool temporary)
{
	int playlistId;

	if(playlistName.isEmpty()){
		return false;
	}

	if(playlistName.isEmpty()){
		spLog(Log::Warning, this) << "Try to save empty playlist";
		return false;
	}

	playlistId = getPlaylistIdByName(playlistName);
	if(playlistId >= 0) {
		emptyPlaylist(playlistId);
	}

	else {
		playlistId = createPlaylist(playlistName, temporary);
		if( playlistId < 0) {
			return false;
		}
	}

	// fill playlist
	for(int i=0; i<tracks.count(); i++)
	{
		bool success = insertTrackIntoPlaylist(tracks[i], playlistId, i);

		if( !success ) {
			return false;
		}
	}

	return true;
}


bool DB::Playlist::storePlaylist(const MetaDataList& tracks, int playlistId, bool temporary)
{
	CustomPlaylist pl;
	pl.setId(playlistId);

	bool success = getPlaylistById(pl);
	if(!success){
		spLog(Log::Warning, this) << "Store: Cannot fetch playlist: " << pl.id();
		return false;
	}

	if(pl.name().isEmpty()){
		return false;
	}

	if( playlistId < 0) {
		playlistId = createPlaylist(pl.name(), temporary);
	}

	else{
		emptyPlaylist(playlistId);
	}

	// fill playlist
	for(int i=0; i<tracks.count(); i++)
	{
		bool success = insertTrackIntoPlaylist(tracks[i], playlistId, i);

		if( !success ) {
			return false;
		}
	}

	return true;
}

bool DB::Playlist::emptyPlaylist(int playlistId)
{
	Query q(this);
	QString querytext = QString("DELETE FROM playlistToTracks WHERE playlistID = :playlistID;");
	q.prepare(querytext);
	q.bindValue(":playlistID", playlistId);

	if(!q.exec()) {
		q.showError("DB: Playlist cannot be cleared");
		return false;
	}

	return true;
}

bool DB::Playlist::deletePlaylist(int playlistId)
{
	emptyPlaylist(playlistId);

	Query q(this);
	QString querytext = QString("DELETE FROM playlists WHERE playlistID = :playlistID;");

	q.prepare(querytext);
	q.bindValue(":playlistID", playlistId);

	if(!q.exec()){
		q.showError(QString("Cannot delete playlist ") + QString::number(playlistId));
		return false;
	}

	return true;
}
