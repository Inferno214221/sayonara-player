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

DB::Playlist::~Playlist() = default;

bool DB::Playlist::getAllPlaylistSkeletons(CustomPlaylistSkeletons& skeletons, ::Playlist::StoreType type,
                                           ::Playlist::SortOrder sortorder)
{
	skeletons.clear();

	QString sortorderString;
	switch(sortorder)
	{
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

	const auto fields = QStringList
		{
			"playlists.playlistID	AS playlistID",
			"playlists.playlist		AS playlistName",
			"playlists.temporary	AS temporary",
			"COUNT(ptt.trackID)		AS trackCount"
		};

	auto query = runQuery
		(
			"SELECT " + fields.join(", ") + " "
			                                "FROM playlists LEFT OUTER JOIN playlistToTracks ptt "
			                                "ON playlists.playlistID = ptt.playlistID "
			+ typeClause +
			"GROUP BY playlists.playlistID " +
			sortorderString + ";",

			"Cannot fetch all playlists"
		);

	if(query.hasError())
	{
		return false;
	}

	while(query.next())
	{
		CustomPlaylistSkeleton skeleton;
		if(query.value(0).isNull())
		{
			continue;
		}

		skeleton.setId(query.value(0).toInt());
		skeleton.setName(query.value(1).toString());

		const auto temporary = (query.value(2) != 0);
		skeleton.setTemporary(temporary);
		skeleton.setTrackCount(query.value(3).toInt());

		skeletons << skeleton;
	}

	return true;
}

bool DB::Playlist::getPlaylistSkeletonById(CustomPlaylistSkeleton& skeleton)
{
	if(skeleton.id() < 0)
	{
		spLog(Log::Warning, this) << "Cannot fetch playlist -1";
		return false;
	}

	const auto fields = QStringList
		{
			"playlists.playlistID		AS playlistID",
			"playlists.playlist			AS playlistName",
			"playlists.temporary		AS temporary",
			"COUNT(ptt.trackID)			AS trackCount"
		};

	auto query = runQuery
		(
			"SELECT " + fields.join(", ") + " "
			                                "FROM playlists LEFT OUTER JOIN playlistToTracks ptt "
			                                "ON playlists.playlistID = ptt.playlistID "
			                                "WHERE playlists.playlistid = :playlist_id "
			                                "GROUP BY playlists.playlistID;",

			{{":playlist_id", skeleton.id()}},
			"Cannot fetch all playlists"
		);

	if(query.hasError())
	{
		return false;
	}

	if(query.next())
	{
		skeleton.setId(query.value(0).toInt());
		skeleton.setName(query.value(1).toString());

		const auto temporary = (query.value(2) != 0);
		skeleton.setTemporary(temporary);
		skeleton.setTrackCount(query.value(3).toInt());

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
			"tracks.trackID				AS trackID",        // 0
			"tracks.title				AS title",          // 1
			"tracks.length				AS length",         // 2
			"tracks.year				AS year",           // 3
			"tracks.bitrate				AS bitrate",        // 4
			"tracks.filename			AS filename",       // 5
			"tracks.track				AS trackNum",       // 6
			"albums.albumID				AS albumID",        // 7
			"artists.artistID			AS artistID",       // 8
			"albums.name				AS albumName",      // 9
			"artists.name				AS artistName",     // 10
			"tracks.genre				AS genrename",      // 11
			"tracks.filesize			AS filesize",       // 12
			"tracks.discnumber			AS discnumber",     // 13
			"tracks.rating				AS rating",         // 14
			"ptt.filepath				AS filepath",       // 15
			"ptt.db_id					AS databaseId",     // 16
			"tracks.libraryID			AS libraryId",      // 17
			"tracks.createdate			AS createdate",     // 18
			"tracks.modifydate			AS modifydate",     // 19
			"ptt.coverDownloadUrl		AS coverDownloadUrl"// 20
		};

	auto query = runQuery
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

	if(!query.hasError())
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
			QStringList genres = query.value(11).toString().split(",");
			data.setGenres(genres);
			data.setFilesize(query.value(12).value<Filesize>());
			data.setDiscnumber(query.value(13).value<Disc>());
			data.setRating(query.value(14).value<Rating>());
			data.setLibraryid(query.value(17).value<LibraryId>());
			data.setCreatedDate(query.value(18).value<uint64_t>());
			data.setModifiedDate(query.value(19).value<uint64_t>());
			data.setCoverDownloadUrls(query.value(20).toString().split(";"));
			data.setExtern(false);
			data.setDatabaseId(databaseId());

			if(query.value(16).toInt() == 0 || query.value(16).isNull())
			{
				pl.push_back(std::move(data));
			}
		}
	}

	// non database playlists
	auto query2 = runQuery
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

	if(query2.hasError())
	{
		return false;
	}

	while(query2.next())
	{
		const auto filepath = query2.value(0).toString();
		const auto position = query2.value(1).toInt();
		const auto radioStationName = query2.value(2).toString();
		const auto radioStation = query2.value(3).toString();
		const auto isRadio = query2.value(4).toBool();
		const auto coverUrls = query2.value(5).toString().split(";");

		MetaData md(filepath);
		md.setId(-1);
		md.setExtern(true);
		md.setDatabaseId(databaseId());
		md.setCoverDownloadUrls(coverUrls);

		if(isRadio)
		{
			md.setRadioStation(radioStation, radioStationName);
		}

		else
		{
			md.setTitle(filepath);
			md.setArtist(filepath);
		}

		for(int row = 0; row <= pl.count(); row++)
		{
			if(row >= position)
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
	auto query = runQuery
		(
			"SELECT playlistid FROM playlists WHERE playlist = :playlistName;",
			{
				{":playlistName", Util::convertNotNull(name)}
			},
			QString("Playlist by name: Cannot fetch playlist %1").arg(name)
		);

	if(!query.hasError() && query.next())
	{
		return query.value(0).toInt();
	}

	return -1;
}

bool DB::Playlist::insertTrackIntoPlaylist(const MetaData& md, int playlistId, int pos)
{
	if(md.isDisabled())
	{
		return false;
	}

	auto fieldBindings = QMap<QString, QVariant>
		{
			{"playlistid",       playlistId},
			{"filepath",         Util::convertNotNull(md.filepath())},
			{"position",         pos},
			{"trackid",          md.id()},
			{"db_id",            md.databaseId()},
			{"coverDownloadUrl", md.coverDownloadUrls().join(";")}
		};

	const auto isRadio =
		(md.radioMode() == RadioMode::Station) ||
		(md.radioMode() == RadioMode::Podcast);

	if(isRadio)
	{
		fieldBindings.insert("stationName", Util::convertNotNull(md.radioStationName()));
		fieldBindings.insert("station", Util::convertNotNull(md.radioStation()));
		fieldBindings.insert("isRadio", isRadio);
	}

	auto query = insert("playlistToTracks", fieldBindings, "Cannot insert track into playlist");
	return (!query.hasError());
}

// returns id if everything ok
// negative otherwise
int DB::Playlist::createPlaylist(QString playlist_name, bool temporary)
{
	auto query = insert("playlists",
	                    {
		                 {"playlist",  Util::convertNotNull(playlist_name)},
		                 {"temporary", (temporary == true) ? 1 : 0}
	                 }, "Cannot create playlist");

	return (query.hasError())
		? -1
		: query.lastInsertId().toInt();
}

bool DB::Playlist::renamePlaylist(int id, const QString& new_name)
{
	auto query = update("playlists",
	                    {{"playlist", Util::convertNotNull(new_name)}},
	                    {"playlistId", id}, "Cannot rename playlist");

	return (!query.hasError());
}

bool DB::Playlist::storePlaylist(const MetaDataList& tracks, QString playlistName, bool temporary)
{
	if(playlistName.isEmpty())
	{
		return false;
	}

	if(playlistName.isEmpty())
	{
		spLog(Log::Warning, this) << "Try to save empty playlist";
		return false;
	}

	auto playlistId = getPlaylistIdByName(playlistName);
	if(playlistId >= 0)
	{
		emptyPlaylist(playlistId);
	}

	else
	{
		playlistId = createPlaylist(playlistName, temporary);
		if(playlistId < 0)
		{
			return false;
		}
	}

	// fill playlist
	for(int i = 0; i < tracks.count(); i++)
	{
		const auto success = insertTrackIntoPlaylist(tracks[i], playlistId, i);
		if(!success)
		{
			return false;
		}
	}

	return true;
}

bool DB::Playlist::storePlaylist(const MetaDataList& tracks, int playlistId, bool temporary)
{
	CustomPlaylist pl;
	pl.setId(playlistId);

	const auto success = getPlaylistById(pl);
	if(!success)
	{
		spLog(Log::Warning, this) << "Store: Cannot fetch playlist: " << pl.id();
		return false;
	}

	if(pl.name().isEmpty())
	{
		return false;
	}

	if(playlistId < 0)
	{
		playlistId = createPlaylist(pl.name(), temporary);
	}

	else
	{
		emptyPlaylist(playlistId);
	}

	// fill playlist
	for(int i = 0; i < tracks.count(); i++)
	{
		const auto success = insertTrackIntoPlaylist(tracks[i], playlistId, i);
		if(!success)
		{
			return false;
		}
	}

	return true;
}

bool DB::Playlist::emptyPlaylist(int playlistId)
{
	Query q(this);
	const auto querytext = QString("DELETE FROM playlistToTracks WHERE playlistID = :playlistID;");
	q.prepare(querytext);
	q.bindValue(":playlistID", playlistId);

	const auto success = q.exec();
	if(!success)
	{
		q.showError("DB: Playlist cannot be cleared");
	}

	return success;
}

bool DB::Playlist::deletePlaylist(int playlistId)
{
	emptyPlaylist(playlistId);

	Query q(this);
	QString querytext = QString("DELETE FROM playlists WHERE playlistID = :playlistID;");

	q.prepare(querytext);
	q.bindValue(":playlistID", playlistId);

	const auto success = q.exec();
	if(!success)
	{
		q.showError(QString("Cannot delete playlist ") + QString::number(playlistId));
	}

	return success;
}
