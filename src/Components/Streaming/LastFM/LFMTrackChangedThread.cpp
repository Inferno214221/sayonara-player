/* LFMTrackChangedThread.cpp

 * Copyright (C) 2011-2020 Michael Lugmair (Lucio Carreras)
 *
 * This file is part of sayonara-player
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * created by Michael Lugmair (Lucio Carreras),
 * Jul 18, 2012
 *
 */

#include "LFMTrackChangedThread.h"
#include "LFMWebAccess.h"
#include "LFMGlobals.h"
#include "DynamicPlayback/LfmSimiliarArtistsParser.h"

#include "Database/Connector.h"
#include "Database/LibraryDatabase.h"

#include "Utils/MetaData/Artist.h"
#include "Utils/MetaData/MetaData.h"
#include "Utils/Compressor/Compressor.h"
#include "Utils/Logger/Logger.h"

#ifdef SMART_COMPARE
#include "Utils/SmartCompare/SmartCompare.h"
#endif

#include <QMap>
#include <QStringList>
#include <QUrl>
#include <QHash>

using namespace LastFM;

struct TrackChangedThread::Private
{
	QString artist;

#ifdef SMART_COMPARE
	SmartCompare*				_smart_comparison=nullptr;
#endif
};

TrackChangedThread::TrackChangedThread(QObject* parent) :
	QObject(parent)
{
	m = Pimpl::make<TrackChangedThread::Private>();

	ArtistList artists;
	auto* db = DB::Connector::instance();
	DB::LibraryDatabase* lib_db = db->libraryDatabase(-1, 0);

	lib_db->getAllArtists(artists, false);

#ifdef SMART_COMPARE
	_smart_comparison = new SmartCompare(artists);
#endif

}

TrackChangedThread::~TrackChangedThread() = default;

void TrackChangedThread::updateNowPlaying(const QString& session_key,
                                          const MetaData& md)
{
	if(md.title().trimmed().isEmpty() || md.artist().trimmed().isEmpty())
	{
		return;
	}

	spLog(Log::Debug, this) << "Update current_track " << md.title() + " by "
	                        << md.artist();

	auto* lfm_wa = new WebAccess();
	connect(lfm_wa,
	        &WebAccess::sigResponse,
	        this,
	        &TrackChangedThread::updateResponseReceived);
	connect(lfm_wa,
	        &WebAccess::sigError,
	        this,
	        &TrackChangedThread::updateErrorReceived);

	QString artist = md.artist();
	artist.replace("&", "&amp;");

	UrlParams sig_data;
	sig_data["api_key"] = LFM_API_KEY;
	sig_data["artist"] = artist.toLocal8Bit();
	sig_data["duration"] = QString::number(
		md.durationMs() / 1000).toLocal8Bit();
	sig_data["method"] = QString("track.updatenowplaying").toLocal8Bit();
	sig_data["sk"] = session_key.toLocal8Bit();
	sig_data["track"] = md.title().toLocal8Bit();

	sig_data.appendSignature();

	QByteArray post_data;
	QString url = lfm_wa->createPostUrl(
		QString("http://ws.audioscrobbler.com/2.0/"),
		sig_data,
		post_data);

	lfm_wa->callPostUrl(url, post_data);
}

void TrackChangedThread::updateResponseReceived(const QByteArray& data)
{
	Q_UNUSED(data)
	if(sender())
	{
		sender()->deleteLater();
	}
}

void TrackChangedThread::updateErrorReceived(const QString& error)
{
	spLog(Log::Warning, this) << "Last.fm: Cannot update track";
	spLog(Log::Warning, this) << "Last.fm: " << error;

	if(sender())
	{
		sender()->deleteLater();
	}
}
