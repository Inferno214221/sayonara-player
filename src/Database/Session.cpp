/* Session.cpp */

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

#include "Session.h"
#include "Query.h"

#include "Utils/Utils.h"
#include "Utils/MetaData/MetaData.h"
#include "Utils/Logger/Logger.h"

#include <QDateTime>

using DB::Module;

DB::Session::Session(const QString& connection_name, DbId databaseId) :
	Module(connection_name, databaseId)
{}

DB::Session::~Session() = default;

::Session::EntryList DB::Session::getSession(::Session::Id session_id)
{
	const QStringList fields
	{
		"sessionID",
		"date",
		"trackID",
		"title",
		"artist",
		"album",
	};

	DB::Query q = this->runQuery
	(
		QString("SELECT %1 FROM Sessions WHERE sessionID = :sessionId;").arg(fields.join(", ")),
		{":sessionId", QVariant::fromValue<::Session::Id>(session_id)},
		QString("Session: Cannot fetch session %1").arg(session_id)
	);

	if(q.hasError())
	{
		spLog(Log::Warning, this) << QString("Cannot fetch session %1").arg(session_id);
		return ::Session::EntryList();
	}

	::Session::EntryList ret;
	while(q.next())
	{
		MetaData md;
			md.setId(q.value(3).toInt());
			md.setTitle(q.value(4).toString());
			md.setArtist(q.value(5).toString());
			md.setAlbum(q.value(6).toString());

		::Session::Entry entry;
			entry.sessionId = q.value(1).value<::Session::Id>();
			entry.timecode = q.value(2).value<::Session::Timecode>();
			entry.md = md;

		ret << entry;
	}

	return ret;
}

QList<Session::Id> DB::Session::getSessionKeys()
{
	QList<::Session::Id> ids;
	DB::Query q = this->runQuery
	(
		QString("SELECT DISTINCT sessionID FROM Sessions;"),
		QString("Session: Cannot fetch session keys")
	);

	if(q.hasError()){
		return ids;
	}

	while(q.next())
	{
		ids << q.value(0).value<::Session::Id>();
	}

	return ids;
}

Session::Id DB::Session::createNewSession() const
{
	return ::Util::dateToInt(QDateTime::currentDateTime());
}

::Session::EntryListMap DB::Session::getSessions(const QDateTime& dt_begin, const QDateTime& dt_end)
{
	const QStringList fields
	{
		"sessions.sessionID",		// 0
		"sessions.date",			// 1
		"sessions.trackID",			// 2
		"sessions.title",			// 3
		"sessions.artist",			// 4
		"sessions.album",			// 5

		"tracks.title",				// 6
		"tracks.filename",			// 7
		"tracks.artistID",			// 8
		"tracks.artistName",		// 9
		"tracks.albumID",			// 10
		"tracks.albumName",			// 11
		"tracks.albumArtistID",		// 12
		"tracks.albumArtistName",	// 13
		"tracks.year",				// 14
		"tracks.comment",			// 15
		"tracks.trackNum",			// 16
		"tracks.bitrate",			// 17
		"tracks.length",			// 18
		"tracks.genre",				// 19
		"tracks.discnumber",		// 20
		"tracks.filesize",			// 21
		"tracks.trackLibraryID"		// 22
	};

	::Session::Timecode min_timecode = Util::dateToInt(dt_begin);
	::Session::Timecode max_timecode = Util::dateToInt(dt_end);

	if(dt_begin.isNull()) {
		min_timecode = 0;
	}

	if(dt_end.isNull()){
		max_timecode =  Util::dateToInt(QDateTime::currentDateTime());
	}

	QString query = QString
	(
		"SELECT %1 FROM Sessions sessions "
		"LEFT OUTER JOIN track_search_view tracks ON tracks.trackID = sessions.trackID "
		"WHERE Sessions.date > :minDate AND Sessions.date <= :maxDate; "
	).arg(fields.join(", "));

	DB::Query q(this);
	q.prepare(query);
	q.bindValue(":minDate", QVariant::fromValue<::Session::Timecode>(min_timecode));
	q.bindValue(":maxDate", QVariant::fromValue<::Session::Timecode>(max_timecode));
	q.exec();

	if(q.hasError())
	{
		spLog(Log::Error, this) << "Session: Cannot fetch sessions";
		return ::Session::EntryListMap();
	}

	::Session::EntryListMap ret;
	while(q.next())
	{
		::Session::Entry entry;
			entry.sessionId = q.value(0).value<::Session::Id>();
			entry.timecode = q.value(1).value<::Session::Timecode>();

			entry.md.setId(q.value(2).toInt());

			entry.md.setTitle(q.value(6).toString());
			entry.md.setFilepath(q.value(7).toString());
			entry.md.setArtistId(q.value(8).toInt());
			entry.md.setArtist(q.value(9).toString());
			entry.md.setAlbumId(q.value(10).toInt());
			entry.md.setAlbum(q.value(11).toString());
			entry.md.setAlbumArtist(q.value(13).toString(), q.value(12).toInt());
			entry.md.setYear(q.value(14).value<Year>());
			entry.md.setComment(q.value(15).toString());
			entry.md.setTrackNumber(q.value(16).value<TrackNum>());
			entry.md.setBitrate(q.value(17).value<Bitrate>());
			entry.md.setDurationMs(q.value(18).value<MilliSeconds>());
			entry.md.setGenres(q.value(19).toString().split(","));
			entry.md.setDiscnumber(q.value(20).value<Disc>());
			entry.md.setFilesize(q.value(21).value<Filesize>());
			entry.md.setLibraryid(LibraryId(q.value(22).toInt()));

			if(entry.md.title().isEmpty()) {
				entry.md.setTitle(q.value(3).toString());
			}

			if(entry.md.artist().isEmpty()) {
				entry.md.setArtist(q.value(4).toString());
			}

			if(entry.md.album().isEmpty()) {
				entry.md.setAlbum(q.value(5).toString());
			}

		auto it = ret.find(entry.sessionId);
		if(it != ret.end()){
			it.value() << entry;
		}

		else
		{
			::Session::EntryList lst;
			lst << entry;
			ret.insert(entry.sessionId, lst);
		}
	}

	return ret;
}

bool DB::Session::addTrack(::Session::Id session_id, const MetaData& md)
{
	const ::Session::Timecode timecode = Util::dateToInt(QDateTime::currentDateTime());

	const DB::Query q = this->insert
	(
		"sessions",
		{
			{"sessionID", QVariant::fromValue<::Session::Id>(session_id)},
			{"date", QVariant::fromValue<::Session::Timecode>(timecode)},
			{"trackID", md.id()},
			{"title", md.title()},
			{"artist", md.artist()},
			{"album", md.album()}
		},
		"Session: Cannot insert track"
	);

	return (!q.hasError());
}

bool DB::Session::clear()
{
	const DB::Query q = this->runQuery
	(
		"DELETE FROM Sessions;",
		"Session: Cannot clear sessions"
	);

	return (!q.hasError());
}

bool DB::Session::clearBefore(const QDateTime& datetime)
{
	const ::Session::Timecode timecode = Util::dateToInt(datetime);

	const DB::Query q = this->runQuery
	(
		"DELETE FROM Sessions WHERE date < :minDate;",
		{":minDate", QVariant::fromValue<::Session::Timecode>(timecode)},
		QString("Cannot clear before %1").arg(timecode)
	);

	return (!q.hasError());
}
