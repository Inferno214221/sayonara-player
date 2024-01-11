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
				return QString();
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
			QStringLiteral("tracks.trackID          AS trackID"),        // 0
			QStringLiteral("tracks.title            AS title"),          // 1
			QStringLiteral("tracks.length           AS length"),         // 2
			QStringLiteral("tracks.year             AS year"),           // 3
			QStringLiteral("tracks.bitrate          AS bitrate"),        // 4
			QStringLiteral("tracks.filename         AS filename"),       // 5
			QStringLiteral("tracks.track            AS trackNum"),       // 6
			QStringLiteral("albums.albumID          AS albumID"),        // 7
			QStringLiteral("artists.artistID        AS artistID"),       // 8
			QStringLiteral("albums.name             AS albumName"),      // 9
			QStringLiteral("artists.name            AS artistName"),     // 10
			QStringLiteral("tracks.genre            AS genrename"),      // 11
			QStringLiteral("tracks.filesize         AS filesize"),       // 12
			QStringLiteral("tracks.discnumber       AS discnumber"),     // 13
			QStringLiteral("tracks.rating           AS rating"),         // 14
			QStringLiteral("ptt.filepath            AS filepath"),       // 15
			QStringLiteral("ptt.db_id               AS databaseId"),     // 16
			QStringLiteral("tracks.libraryID        AS libraryId"),      // 17
			QStringLiteral("tracks.createdate       AS createdate"),     // 18
			QStringLiteral("tracks.modifydate       AS modifydate"),     // 19
			QStringLiteral("ptt.coverDownloadUrl    AS coverDownloadUrl"), // 20
			QStringLiteral("ptt.position            AS position") // 21
		};

	static const auto joinedFields = fields.join(", ");

	const auto queryText = QString("SELECT %1 "
	                               "FROM tracks, albums, artists, playlists, playlistToTracks ptt "
	                               "WHERE playlists.playlistID = :playlist_id "
	                               "AND playlists.playlistID = ptt.playlistID "
	                               "AND ptt.trackID = tracks.trackID "
	                               "AND tracks.albumID = albums.albumID "
	                               "AND tracks.artistID = artists.artistID "
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

MetaDataList DB::Playlist::getPlaylistWithNonDatabaseTracks(const int playlistId)
{
	MetaDataList result;

	const auto static fields = QStringList {
		QStringLiteral("ptt.filepath          AS filepath"),
		QStringLiteral("ptt.position          AS position"),
		QStringLiteral("ptt.stationName       AS radioStationName"),
		QStringLiteral("ptt.station           AS radioStation"),
		QStringLiteral("ptt.isRadio           AS isRadio"),
		QStringLiteral("ptt.isUpdatable       AS isUpdatable"),
		QStringLiteral("ptt.coverDownloadUrl  AS coverDownloadUrl"),
		QStringLiteral("ptt.position          AS position")
	};

	const auto static joinedFields = fields.join(", ");

	const auto queryText = QString("SELECT %1 "
	                               "FROM playlists pl, playlistToTracks ptt "
	                               "WHERE pl.playlistID = :playlistID "
	                               "AND pl.playlistID = ptt.playlistID "
	                               "AND ptt.trackID < 0 "
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
		const auto radioStationName = query.value(2).toString();
		const auto radioStation = query.value(3).toString();
		const auto radioMode = query.value(4).value<RadioMode>();
		const auto isUpdatable = query.value(5).toBool();
		const auto coverUrls = variantToStringList(query.value(6), ';');

		auto track = MetaData(filepath);
		track.setId(-1);
		track.setExtern(true);
		track.setDatabaseId(databaseId());
		track.setCoverDownloadUrls(coverUrls);
		track.setUpdateable(isUpdatable);

		if(radioMode == RadioMode::Station)
		{
			track.setRadioStation(radioStation, radioStationName);
		}

		else if(radioMode == RadioMode::Podcast)
		{
			track.setTitle(radioStationName);
			track.setArtist(radioStation);
			track.setAlbum(radioStationName);
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

// negative, if error
// nonnegative else
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
		{"playlistid",       playlistId},
		{"filepath",         Util::convertNotNull(track.filepath())},
		{"position",         pos},
		{"trackid",          track.id()},
		{"db_id",            track.databaseId()},
		{"coverDownloadUrl", track.coverDownloadUrls().join(";")},
		{"isRadio",          static_cast<int>(track.radioMode())}, // for some reason QVariant::fromValue does not work here
		{"isUpdatable",      track.isUpdatable()}};

	if(track.radioMode() == RadioMode::Station)
	{
		fieldBindings.insert("stationName", Util::convertNotNull(track.radioStationName()));
		fieldBindings.insert("station", Util::convertNotNull(track.radioStation()));
	}

	if(track.radioMode() == RadioMode::Podcast)
	{
		fieldBindings.insert("stationName", Util::convertNotNull(track.title()));
		fieldBindings.insert("station", Util::convertNotNull(track.artist()));
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

bool DB::Playlist::clearPlaylist(int playlistId)
{
	const auto querytext = QStringLiteral("DELETE FROM playlistToTracks WHERE playlistID = :playlistID;");
	const auto query = runQuery(querytext, {":playlistID", playlistId}, "Playlist cannot be cleared");

	return !hasError(query);
}

bool DB::Playlist::deletePlaylist(int playlistId)
{
	clearPlaylist(playlistId);

	const auto querytext = QString("DELETE FROM playlists WHERE playlistID = :playlistID;");
	const auto query = runQuery(querytext, {":playlistID", playlistId}, "Playlist cannot be deleted");

	return !hasError(query);
}
