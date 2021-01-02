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

	auto* db = DB::Connector::instance();
	auto* libraryDatabase = db->libraryDatabase(-1, 0);

	ArtistList artists;
	libraryDatabase->getAllArtists(artists, false);

#ifdef SMART_COMPARE
	_smart_comparison = new SmartCompare(artists);
#endif

}

TrackChangedThread::~TrackChangedThread() = default;

void TrackChangedThread::updateNowPlaying(const QString& sessionKey, const MetaData& track)
{
	if(track.title().trimmed().isEmpty() || track.artist().trimmed().isEmpty())
	{
		return;
	}

	spLog(Log::Debug, this) << "Update current_track " << track.title() + " by "
	                        << track.artist();

	auto* webAccess = new WebAccess();
	connect(webAccess, &WebAccess::sigResponse, this, &TrackChangedThread::updateResponseReceived);
	connect(webAccess, &WebAccess::sigError, this, &TrackChangedThread::updateErrorReceived);

	auto artist = track.artist();
	artist.replace("&", "&amp;");

	UrlParams signatureData;
	signatureData["api_key"] = LFM_API_KEY;
	signatureData["artist"] = artist;
	signatureData["duration"] = QString::number(track.durationMs() / 1000);
	signatureData["method"] = QString("track.updatenowplaying");
	signatureData["sk"] = sessionKey;
	signatureData["track"] = track.title();

	signatureData.appendSignature();

	QByteArray postData;
	const auto url = webAccess->createPostUrl(
		QString("http://ws.audioscrobbler.com/2.0/"),
		signatureData,
		postData);

	webAccess->callPostUrl(url, postData);
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
