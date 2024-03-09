/* DatabasePlaylist.cpp */

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

#include "Database/Query.h"
#include "Database/Playlist.h"

#include "Utils/Algorithm.h"
#include "Utils/Logger/Logger.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Playlist/CustomPlaylist.h"
#include "Utils/MetaData/Genre.h"
#include "Utils/Utils.h"

namespace
{
	constexpr const auto PositionKey = "position";

	MetaDataList filterDisabledTracks(MetaDataList tracks)
	{
		tracks.removeTracks([](const auto& track) {
			return track.isDisabled();
		});

		return tracks;
	}

	QStringList variantToStringList(const QVariant& value, const QChar splitter)
	{
		return value.toString().split(splitter);
	}

	QString createSortorderStatement(const PlaylistSortOrder& sortorder)
	{
		switch(sortorder)
		{
			case ::Playlist::SortOrder::IDAsc:
				return QStringLiteral("playlists.playlistID ASC");
			case ::Playlist::SortOrder::IDDesc:
				return QStringLiteral("playlists.playlistID DESC");
			case ::Playlist::SortOrder::NameAsc:
				return QStringLiteral("playlists.playlist ASC");
			case ::Playlist::SortOrder::NameDesc:
				return QStringLiteral("playlists.playlist DESC");
			default:
				return {};
		}
	}

	QString createStoreTypeStatement(const ::Playlist::StoreType storeType)
	{
		switch(storeType)
		{
			case ::Playlist::StoreType::OnlyTemporary:
				return QStringLiteral("playlists.temporary = 1");
			case ::Playlist::StoreType::OnlyPermanent:
				return QStringLiteral("playlists.temporary = 0");
			default:
				return QStringLiteral("1");
		}
	}

	MetaDataList mergeTracks(MetaDataList tracks1, MetaDataList tracks2)
	{
		auto tracks = MetaDataList() << std::move(tracks1) << std::move(tracks2);

		Util::Algorithm::sort(tracks, [](const auto& track1, const auto& track2) {
			const auto pos1 = track1.customField(PositionKey).toInt();
			const auto pos2 = track2.customField(PositionKey).toInt();

			return (pos1 < pos2);
		});

		return tracks;
	}

	QString joinedPlaylistFields()
	{
		static const auto fields = QStringList
			{
				QStringLiteral("playlists.playlistID AS playlistID"),
				QStringLiteral("playlists.playlist   AS playlistName"),
				QStringLiteral("playlists.temporary  AS temporary"),
				QStringLiteral("playlists.isLocked   AS isLocked")
			};

		static const auto joinedFields = fields.join(", ");

		return joinedFields;
	}

	int insertOnlineTrack(DB::Module* module, const MetaData& track)
	{
		auto fieldBindings = QMap<QString, QVariant> {
			{"isUpdatable", track.isUpdatable()},
			{"description", Util::convertNotNull(track.customField("description"))},
			{"userAgent",   Util::convertNotNull(track.customField("user-agent"))},
			{"radioMode",   static_cast<int>(track.radioMode())}
		};

		if(track.radioMode() == RadioMode::Station)
		{
			fieldBindings.insert("stationName", Util::convertNotNull(track.radioStationName()));
			fieldBindings.insert("stationUrl", Util::convertNotNull(track.radioStation()));
		}

		if(track.radioMode() == RadioMode::Podcast)
		{
			fieldBindings.insert("stationName", Util::convertNotNull(track.title()));
			fieldBindings.insert("stationUrl", Util::convertNotNull(track.artist()));
		}

		const auto q = module->insert("OnlineTracks",
		                              fieldBindings,
		                              QString("Cannot insert %1 into OnlineTracks")
			                              .arg(track.filepath()));

		return DB::hasError(q) ? -1 : q.lastInsertId().toInt();
	}
}

DB::Playlist::Playlist(const QString& connection_name, const DbId databaseId) :
	Module(connection_name, databaseId) {}

DB::Playlist::~Playlist() = default;

QList<CustomPlaylist>
DB::Playlist::getAllPlaylists(::Playlist::StoreType storeType, const bool getTracks,
                              const ::Playlist::SortOrder sortOrder)
{
	QList<CustomPlaylist> result;

	const auto storeTypeStatement = createStoreTypeStatement(storeType);
	const auto sortingStatement = createSortorderStatement(sortOrder);

	const auto queryText =
		QString("SELECT %1 "
		        "FROM playlists "
		        "LEFT OUTER JOIN playlistToTracks ptt ON playlists.playlistID = ptt.playlistID "
		        "WHERE %2 "
		        "GROUP BY playlists.playlistID "
		        "ORDER BY %3;")
			.arg(joinedPlaylistFields())
			.arg(storeTypeStatement)
			.arg(sortingStatement);

	auto query = runQuery(queryText, "Cannot fetch all playlists");
	if(hasError(query))
	{
		return {};
	}

	while(query.next())
	{
		CustomPlaylist customPlaylist;
		if(query.value(0).isNull())
		{
			continue;
		}

		const auto playlistId = query.value(0).toInt();
		customPlaylist.setId(playlistId);
		customPlaylist.setName(query.value(1).toString());

		const auto isTemporary = (query.value(2) != 0);
		customPlaylist.setTemporary(isTemporary);
		customPlaylist.setLocked(query.value(3) != 0);

		if(getTracks)
		{
			auto tracks = getPlaylistWithDatabaseTracks(playlistId);
			auto nonDbTracks = getPlaylistWithNonDatabaseTracks(playlistId);
			auto mergedTracks = mergeTracks(tracks, nonDbTracks);

			customPlaylist.setTracks(std::move(mergedTracks));
		}

		result.push_back(customPlaylist);
	}

	return result;
}

CustomPlaylist DB::Playlist::getPlaylistById(const int playlistId, const bool getTracks)
{
	if(playlistId < 0)
	{
		return {};
	}

	const auto queryText = QString("SELECT %1 "
	                               "FROM playlists LEFT OUTER JOIN playlistToTracks ptt "
	                               "ON playlists.playlistID = ptt.playlistID "
	                               "WHERE playlists.playlistid = :playlist_id "
	                               "GROUP BY playlists.playlistID;").arg(joinedPlaylistFields());

	auto query = runQuery(queryText, {{":playlist_id", playlistId}}, "Cannot fetch all playlists");
	if(hasError(query))
	{
		return {};
	}

	if(query.next())
	{
		CustomPlaylist result;

		result.setId(query.value(0).toInt());
		result.setName(query.value(1).toString());

		const auto temporary = (query.value(2) != 0);
		result.setTemporary(temporary);
		result.setLocked(query.value(3) != 0);

		if(getTracks)
		{
			auto tracks = getPlaylistWithDatabaseTracks(playlistId);
			auto nonDbTracks = getPlaylistWithNonDatabaseTracks(playlistId);

			auto mergedTracks = mergeTracks(tracks, nonDbTracks);
			result.setTracks(std::move(mergedTracks));
		}

		return result;
	}

	return {};
}

MetaDataList DB::Playlist::getPlaylistWithDatabaseTracks(const int playlistId)
{
	MetaDataList result;

	static const auto fields = QStringList
		{
			"tsv.trackID          AS trackID",        // 0
			"tsv.title            AS title",          // 1
			"tsv.length           AS length",         // 2
			"tsv.year             AS year",           // 3
			"tsv.bitrate          AS bitrate",        // 4
			"tsv.filename         AS filename",       // 5
			"tsv.trackNum         AS trackNum",       // 6
			"tsv.albumID          AS albumID",        // 7
			"tsv.artistID         AS artistID",       // 8
			"tsv.albumName        AS albumName",      // 9
			"tsv.artistName       AS artistName",     // 10
			"tsv.genre            AS genrename",      // 11
			"tsv.filesize         AS filesize",       // 12
			"tsv.discnumber       AS discnumber",     // 13
			"tsv.rating           AS rating",         // 14
			"ptt.filepath         AS filepath",       // 15
			"ptt.db_id            AS databaseId",     // 16
			"tsv.trackLibraryId   AS libraryId",      // 17
			"tsv.createdate       AS createdate",     // 18
			"tsv.modifydate       AS modifydate",     // 19
			"ptt.coverDownloadUrl AS coverDownloadUrl", // 20
			"ptt.position         AS position" // 21
		};

	static const auto joinedFields = fields.join(", ");

	const auto queryText = QString("SELECT %1 "
	                               "FROM track_search_view tsv, playlists, playlistToTracks ptt "
	                               "WHERE playlists.playlistID = :playlist_id "
	                               "AND playlists.playlistID = ptt.playlistID "
	                               "AND ptt.trackID = tsv.trackID "
	                               "ORDER BY ptt.position ASC; ").arg(joinedFields);

	auto query = runQuery(queryText,
	                      {
		                      {":playlist_id", playlistId}
	                      },
	                      QString("Cannot get tracks for playlist %1").arg(playlistId)
	);

	if(!hasError(query))
	{
		while(query.next())
		{
			MetaData data;

			data.setId(query.value(0).toInt());
			data.setTitle(query.value(1).toString());
			data.setDurationMs(query.value(2).toInt());
			data.setYear(query.value(3).value<Year>());
			data.setBitrate(query.value(4).value<Bitrate>());
			data.setFilepath(query.value(5).toString());
			data.setTrackNumber(query.value(6).value<TrackNum>());
			data.setAlbumId(query.value(7).toInt());
			data.setArtistId(query.value(8).toInt());
			data.setAlbum(query.value(9).toString().trimmed());
			data.setArtist(query.value(10).toString().trimmed());
			data.setGenres(variantToStringList(query.value(11), ','));
			data.setFilesize(query.value(12).value<Filesize>());
			data.setDiscnumber(query.value(13).value<Disc>());
			data.setRating(query.value(14).value<Rating>());
			data.setLibraryid(query.value(17).value<LibraryId>());
			data.setCreatedDate(query.value(18).value<uint64_t>());
			data.setModifiedDate(query.value(19).value<uint64_t>());
			data.setCoverDownloadUrls(variantToStringList(query.value(20), ';'));
			data.addCustomField(PositionKey, QString(), QString::number(query.value(21).toInt()));
			data.setExtern(false);
			data.setDatabaseId(databaseId());

			if((query.value(16).toInt() == 0) || query.value(16).isNull())
			{
				result.push_back(std::move(data));
			}
		}
	}

	return result;
}

MetaDataList DB::Playlist::getPlaylistWithNonDatabaseTracks(int playlistId)
{
	MetaDataList result;

	const auto static fields = QStringList {
		"ptt.filepath",           // 0
		"ptt.position",           // 1
		"ptt.coverDownloadUrl",   // 2
		"ot.radioMode",           // 3
		"ot.stationName",         // 4
		"ot.stationUrl",          // 5
		"ot.isUpdatable",         // 6
		"ot.userAgent",           // 7
		"ot.description"          // 8
	};

	const auto static joinedFields = fields.join(", ");

	const auto queryText =
		QString("SELECT %1 "
		        "FROM playlists pl "
		        "JOIN playlistToTracks ptt ON pl.playlistId = ptt.playlistId " // ignore line if there are no tracks in ptt
		        "LEFT OUTER JOIN onlineTracks ot ON ptt.onlineTrackId = ot.onlineTrackId " // do not ignore line if it's no track in ot
		        "WHERE ptt.trackID < 0 "
		        "AND pl.playlistId = :playlistID "
		        "ORDER BY ptt.position ASC;").arg(joinedFields);

	// non database playlists
	auto query = runQuery(
		queryText,
		{
			{":playlistID", playlistId}
		},
		QString("Playlist by id: Cannot fetch playlist %1").arg(playlistId));

	if(hasError(query))
	{
		return result;
	}

	while(query.next())
	{
		const auto filepath = query.value(0).toString();
		const auto position = query.value(1).toInt();
		const auto coverUrls = variantToStringList(query.value(2), ';');
		const auto radioMode = query.value(3).value<RadioMode>();
		const auto stationName = query.value(4).toString();
		const auto stationUrl = query.value(5).toString();
		const auto isUpdatable = query.value(6).toBool();
		const auto userAgent = query.value(7).toString();
		const auto description = query.value(8).toString();

		auto track = MetaData(filepath);
		track.setId(-1);
		track.setExtern(true);
		track.setDatabaseId(databaseId());
		track.setCoverDownloadUrls(coverUrls);
		track.setUpdateable(isUpdatable);
		track.addCustomField("user-agent", "", userAgent);
		track.addCustomField("description", "", description);

		if(radioMode == RadioMode::Station)
		{
			track.setRadioStation(stationUrl, stationName);

		}

		else if(radioMode == RadioMode::Podcast)
		{
			track.setTitle(stationName);
			track.setArtist(stationUrl);
			track.setAlbum(stationName);
		}

		else
		{
			track.setTitle(filepath);
			track.setArtist(filepath);
		}

		track.changeRadioMode(radioMode);
		track.addCustomField(PositionKey, QString(), QString::number(position));

		result.push_back(std::move(track));
	}

	return result;
}

int DB::Playlist::getPlaylistIdByName(const QString& name)
{
	const auto queryText = QStringLiteral("SELECT playlistid FROM playlists WHERE playlist = :playlistName;");
	auto query = runQuery(
		queryText,
		{
			{":playlistName", Util::convertNotNull(name)}
		},
		QString("Playlist by name: Cannot fetch playlist %1").arg(name)
	);

	return (!hasError(query) && query.next())
	       ? query.value(0).toInt()
	       : -1;
}

bool DB::Playlist::insertTrackIntoPlaylist(const MetaData& track, const int playlistId, const int pos)
{
	if(track.isDisabled())
	{
		return false;
	}

	auto fieldBindings = QMap<QString, QVariant> {
		{"coverDownloadUrl", Util::convertNotNull(track.coverDownloadUrls().join(";"))},
		{"playlistid",       playlistId},
		{"filepath",         Util::convertNotNull(track.filepath())},
		{"position",         pos},
		{"trackid",          track.id()},
		{"db_id",            track.databaseId()}
	};

	if((track.radioMode() == RadioMode::Station) ||
	   (track.radioMode() == RadioMode::Podcast))
	{
		const auto onlineTrackId = insertOnlineTrack(this, track);
		if(onlineTrackId < 0)
		{
			return false;
		}

		fieldBindings.insert("onlineTrackId", onlineTrackId);
	}

	auto query = insert("playlistToTracks", fieldBindings, "Cannot insert track into playlist");
	return !hasError(query);
}

int DB::Playlist::createPlaylist(const QString& playlistName, const bool temporary, const bool isLocked)
{
	const auto query = insert("playlists",
	                          {
		                          {"playlist",  Util::convertNotNull(playlistName)},
		                          {"temporary", temporary ? 1 : 0},
		                          {"isLocked",  isLocked ? 1 : 0}
	                          }, "Cannot create playlist");

	return hasError(query)
	       ? -1
	       : query.lastInsertId().toInt();
}

bool DB::Playlist::updatePlaylist(const int playlistId, const QString& name, const bool temporary, const bool isLocked)
{
	const auto playlist = getPlaylistById(playlistId, false);
	const auto existingId = getPlaylistIdByName(name);
	const auto isIdValid = (playlistId >= 0);
	const auto otherPlaylistHasSameName = ((existingId >= 0) && (playlist.id() != existingId));
	if(!isIdValid || otherPlaylistHasSameName)
	{
		return false;
	}

	const auto q = update("playlists",
	                      {
		                      {"temporary", temporary ? 1 : 0},
		                      {"playlist",  Util::convertNotNull(name)},
		                      {"isLocked",  isLocked ? 1 : 0}
	                      },
	                      {"playlistId", playlistId}, "Cannot update playlist");

	return wasUpdateSuccessful(q);
}

bool DB::Playlist::renamePlaylist(const int playlistId, const QString& name)
{
	const auto playlist = getPlaylistById(playlistId, false);
	const auto existingId = getPlaylistIdByName(name);
	if((existingId != playlistId) || name.isEmpty() || (playlist.id() < 0))
	{
		return false;
	}

	const auto q = update("playlists",
	                      {
		                      {"playlist", Util::convertNotNull(name)}
	                      },
	                      {"playlistId", playlistId}, "Cannot update playlist");

	return wasUpdateSuccessful(q);
}

bool DB::Playlist::updatePlaylistTracks(int playlistId, const MetaDataList& tracks)
{
	if(const auto playlist = getPlaylistById(playlistId, false); playlist.id() < 0)
	{
		return false;
	}

	clearPlaylist(playlistId);
	if(tracks.isEmpty())
	{
		return true;
	}

	const auto enabledTracks = filterDisabledTracks(tracks);

	db().transaction();
	auto position = 0;
	for(const auto& track: enabledTracks)
	{
		const auto success = insertTrackIntoPlaylist(track, playlistId, position);
		if(success)
		{
			position++;
		}
	}
	db().commit();

	return (enabledTracks.isEmpty() || (position > 0));
}

bool DB::Playlist::clearPlaylist(const int playlistId)
{
	static const auto clearOnlineTracks = QStringLiteral(
		R"(DELETE FROM OnlineTracks WHERE onlineTrackId IN (
		       SELECT onlineTrackId from playlistToTracks WHERE playlistToTracks.playlistId = :playlistId);)");

	const auto clearOnlineTracksQuery =
		runQuery(clearOnlineTracks, {":playlistId", playlistId}, "Cannot clear online tracks");

	if(hasError(clearOnlineTracksQuery))
	{
		return false;
	}

	static const auto clearPlaylistToTracks = QStringLiteral(
		"DELETE FROM playlistToTracks WHERE playlistID = :playlistID;");
	const auto clearPlaylistToTracksQuery =
		runQuery(clearPlaylistToTracks, {":playlistID", playlistId}, "Playlist cannot be cleared");

	return !hasError(clearPlaylistToTracksQuery);
}

bool DB::Playlist::deletePlaylist(int playlistId)
{
	db().transaction();

	auto success = clearPlaylist(playlistId);

	static const auto querytext = QStringLiteral("DELETE FROM playlists WHERE playlistID = :playlistID;");
	const auto query = runQuery(querytext, {":playlistID", playlistId}, "Playlist cannot be deleted");

	success &= !hasError(query);

	if(!success)
	{
		db().rollback();
	}
	else
	{
		db().commit();
	}

	return success;
}
