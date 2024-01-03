/* Session.cpp */

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

#include "Session.h"
#include "Query.h"

#include "Utils/Utils.h"
#include "Utils/MetaData/MetaData.h"
#include "Utils/Logger/Logger.h"

#include <QDateTime>

namespace DB
{
	Session::Session(const QString& connectionName, DbId databaseId) :
		Module(connectionName, databaseId) {}

	Session::~Session() = default;

	QList<::Session::Id> Session::getSessionKeys()
	{
		auto ids = QList<::Session::Id> {};
		auto query = this->runQuery(
			"SELECT DISTINCT sessionID FROM Sessions;",
			"Session: Cannot fetch session keys");

		while(!hasError(query) && query.next())
		{
			ids << query.value(0).value<::Session::Id>();
		}

		return ids;
	}

	::Session::Id Session::createNewSession() const // NOLINT(readability-convert-member-functions-to-static)
	{
		return ::Util::dateToInt(QDateTime::currentDateTime());
	}

	::Session::EntryListMap Session::getSessions(const QDateTime& dateTimeBegin, const QDateTime& dateTimeEnd)
	{
		const auto fields = QStringList {
			"sessions.sessionID",        // 0
			"sessions.date",            // 1
			"sessions.trackID",            // 2
			"sessions.title",            // 3
			"sessions.artist",            // 4
			"sessions.album",            // 5

			"tracks.title",                // 6
			"tracks.filename",            // 7
			"tracks.artistID",            // 8
			"tracks.artistName",        // 9
			"tracks.albumID",            // 10
			"tracks.albumName",            // 11
			"tracks.albumArtistID",        // 12
			"tracks.albumArtistName",    // 13
			"tracks.year",                // 14
			"tracks.comment",            // 15
			"tracks.trackNum",            // 16
			"tracks.bitrate",            // 17
			"tracks.length",            // 18
			"tracks.genre",                // 19
			"tracks.discnumber",        // 20
			"tracks.filesize",            // 21
			"tracks.trackLibraryID"// 22
		};

		const auto minTimecode = (dateTimeBegin.isNull())
		                         ? static_cast<uint64_t>(0)
		                         : Util::dateToInt(dateTimeBegin);

		const auto maxTimecode = (dateTimeEnd.isNull())
		                         ? Util::dateToInt(QDateTime::currentDateTime())
		                         : Util::dateToInt(dateTimeEnd);

		const auto queryText = QString(
			"SELECT %1 FROM Sessions sessions "
			"LEFT OUTER JOIN track_search_view tracks ON tracks.trackID = sessions.trackID "
			"WHERE Sessions.date >= :minDate AND Sessions.date <= :maxDate; "
		).arg(fields.join(", "));

		auto query = runQuery(queryText,
		                      {
			                      {":minDate", QVariant::fromValue<::Session::Timecode>(minTimecode)},
			                      {":maxDate", QVariant::fromValue<::Session::Timecode>(maxTimecode)}
		                      }, "Session: Cannot fetch session");

		auto ret = ::Session::EntryListMap {};

		while(!hasError(query) && query.next())
		{
			auto entry = ::Session::Entry {};
			entry.sessionId = query.value(0).value<::Session::Id>();
			entry.timecode = query.value(1).value<::Session::Timecode>();

			entry.track.setId(query.value(2).toInt());
			entry.track.setTitle(query.value(6).toString());
			entry.track.setFilepath(query.value(7).toString());
			entry.track.setArtistId(query.value(8).toInt());
			entry.track.setArtist(query.value(9).toString());
			entry.track.setAlbumId(query.value(10).toInt());
			entry.track.setAlbum(query.value(11).toString());
			entry.track.setAlbumArtist(query.value(13).toString(), query.value(12).toInt());
			entry.track.setYear(query.value(14).value<Year>());
			entry.track.setComment(query.value(15).toString());
			entry.track.setTrackNumber(query.value(16).value<TrackNum>());
			entry.track.setBitrate(query.value(17).value<Bitrate>());
			entry.track.setDurationMs(query.value(18).value<MilliSeconds>());
			entry.track.setGenres(query.value(19).toString().split(","));
			entry.track.setDiscnumber(query.value(20).value<Disc>());
			entry.track.setFilesize(query.value(21).value<Filesize>());
			entry.track.setLibraryid(LibraryId(query.value(22).toInt()));

			if(entry.track.title().isEmpty())
			{
				entry.track.setTitle(query.value(3).toString());
			}

			if(entry.track.artist().isEmpty())
			{
				entry.track.setArtist(query.value(4).toString());
			}

			if(entry.track.album().isEmpty())
			{
				entry.track.setAlbum(query.value(5).toString());
			}

			const auto it = ret.find(entry.sessionId);
			if(it != ret.end())
			{
				it.value().push_back(std::move(entry));
			}

			else
			{
				const auto sessionId = entry.sessionId;
				const auto entryList = ::Session::EntryList() << std::move(entry);
				ret.insert(sessionId, entryList);
			}
		}

		return ret;
	}

	bool Session::addTrack(::Session::Id sessionId, const MetaData& track, const QDateTime& dateTime)
	{
		const auto timecode = Util::dateToInt(dateTime);
		const auto query = insert(
			"sessions",
			{
				{"sessionID", QVariant::fromValue<::Session::Id>(sessionId)},
				{"date",      QVariant::fromValue<::Session::Timecode>(timecode)},
				{"trackID",   track.id()},
				{"title",     track.title()},
				{"artist",    track.artist()},
				{"album",     track.album()}
			},
			"Session: Cannot insert track");

		return !hasError(query);
	}

	bool Session::clear()
	{
		const auto query = runQuery(
			"DELETE FROM Sessions;",
			"Session: Cannot clear sessions");

		return !hasError(query);
	}

	bool Session::clearBefore(const QDateTime& datetime)
	{
		const auto timecode = Util::dateToInt(datetime.date().endOfDay());
		spLog(Log::Info, this) << "Delete all history <= " << timecode;

		const auto query = runQuery(
			"DELETE FROM Sessions WHERE sessionId <= :maxDate;",
			{":maxDate", QVariant::fromValue<::Session::Timecode>(timecode)},
			QString("Cannot clear before %1").arg(timecode));

		return !hasError(query);
	}
}