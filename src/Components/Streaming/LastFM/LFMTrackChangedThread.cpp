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

#include "Utils/Logger/Logger.h"
#include "Utils/MetaData/Artist.h"
#include "Utils/MetaData/MetaData.h"

namespace LastFM
{
	TrackChangedThread::TrackChangedThread(QObject* parent) :
		QObject(parent) {}

	TrackChangedThread::~TrackChangedThread() = default;

	void TrackChangedThread::updateNowPlaying(const QString& sessionKey, const MetaData& track)
	{
		if(track.title().trimmed().isEmpty() || track.artist().trimmed().isEmpty())
		{
			return;
		}

		spLog(Log::Debug, this) << "Update current_track " << track.title() + " by " << track.artist();

		auto* webAccess = new WebAccess();
		connect(webAccess, &WebAccess::sigError, this, &TrackChangedThread::updateErrorReceived);
		connect(webAccess, &WebAccess::sigFinished, this, &QObject::deleteLater);

		auto artist = track.artist();
		artist.replace("&", "&amp;");

		constexpr const auto* MethodName = "track.updatenowplaying";
		const auto urlParams = UrlParams {
			{"api_key",  ApiKey},
			{"artist",   artist},
			{"duration", QString::number(track.durationMs() / 1000)},
			{"method",   MethodName},
			{"sk",       sessionKey},
			{"track",    track.title()}};

		const auto postData = LastFM::createPostData(urlParams);
		webAccess->callPostUrl(BaseUrl, postData);
	}

	void TrackChangedThread::updateErrorReceived(const QString& error)
	{
		spLog(Log::Warning, this) << "Last.fm: Cannot update track";
		spLog(Log::Warning, this) << "Last.fm: " << error;
	}
}
