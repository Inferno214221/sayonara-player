/* DynamicPlaybackHandler.cpp */
/*
 * Copyright (C) 2011-2024 Michael Lugmair
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

#include "DynamicPlaybackHandler.h"
#include "LfmSimilarArtistFetcher.h"
#include "ArtistMatch.h"
#include "ArtistMatchEvaluator.h"

#include "Components/PlayManager/PlayManager.h"
#include "Components/Playlist/Playlist.h"
#include "Components/Playlist/PlaylistInterface.h"
#include "Components/Playlist/PlaylistModifiers.h"
#include "Database/Connector.h"
#include "Database/LibraryDatabase.h"
#include "Utils/Algorithm.h"
#include "Utils/MetaData/MetaData.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/RandomGenerator.h"
#include "Utils/Set.h"
#include "Utils/Settings/Settings.h"

#include <QTimer>
#include <QThread>
#include <Utils/FileUtils.h>

using namespace DynamicPlayback;

namespace
{
	void appendTrack(Playlist::Accessor* playlistAccessor, const MetaData& track)
	{
		auto activePlaylist = playlistAccessor->activePlaylist();
		Playlist::appendTracks(*activePlaylist, MetaDataList {track}, Playlist::Reason::DynamicPlayback);
	}

	QMap<ArtistId, MetaDataList>
	getCandidateTracks(LibraryId libraryId, const QList<ArtistId>& artistIds)
	{
		auto* db = DB::Connector::instance();
		auto* libraryDatabase = db->libraryDatabase(libraryId, 0);

		MetaDataList tracks;
		libraryDatabase->getAllTracks(tracks);

		QMap<ArtistId, MetaDataList> result;
		for(auto& track: tracks)
		{
			if(!artistIds.contains(track.artistId()) &&
			   !artistIds.contains(track.albumArtistId()))
			{
				continue;
			}

			if(track.albumArtistId() != track.artistId())
			{
				result[track.artistId()].push_back(track);
			}

			result[track.albumArtistId()].push_back(std::move(track));
		}

		for(auto key: result.keys())
		{
			Util::Algorithm::shuffle(result[key]);
		}

		return result;
	}

	Util::Set<QString> getPlaylistFilepaths(Playlist::Accessor* playlistAccessor, const QList<ArtistId>& artistIds)
	{
		Util::Set<QString> playlistFilepaths;

		auto playlist = playlistAccessor->activePlaylist();

		for(const auto& track: playlist->tracks())
		{
			if((track.artistId() < 0) && (track.albumArtistId() < 0))
			{
				playlistFilepaths.insert(track.filepath());
			}

			else if(artistIds.contains(track.artistId()) ||
			        artistIds.contains(track.albumArtistId()))
			{
				playlistFilepaths.insert(track.filepath());
			}
		}

		return playlistFilepaths;
	}

	MetaData findTrackNotInPlaylist(const MetaDataList& tracks, const Util::Set<QString>& playlistFilepaths)
	{
		for(const auto& track: tracks)
		{
			const auto contains = playlistFilepaths.contains(track.filepath());
			if(!contains && Util::File::exists(track.filepath()))
			{
				return track;
			}
		}

		return MetaData {};
	}
}

struct Handler::Private
{
	QString currentArtistName;
	LibraryId currentLibraryId;
	Playlist::Accessor* playlistAccessor;
	QTimer* timer;

	Private(Playlist::Accessor* playlistAccessor) :
		currentLibraryId {-1},
		playlistAccessor(playlistAccessor),
		timer(new QTimer())
	{
		timer->setSingleShot(true);
	}
};

Handler::Handler(PlayManager* playManager, Playlist::Accessor* playlistAccessor, QObject* parent) :
	QObject(parent)
{
	m = Pimpl::make<Private>(playlistAccessor);

	connect(playManager, &PlayManager::sigCurrentTrackChanged, this, &Handler::currentTrackChanged);
	connect(playManager, &PlayManager::sigPlaystateChanged, this, [=](auto playState) {
		if(playState == PlayState::Stopped)
		{
			m->timer->stop();
		}
	});

	connect(m->timer, &QTimer::timeout, this, &Handler::timeout);
}

Handler::~Handler() = default;

void Handler::currentTrackChanged(const MetaData& track)
{
	const auto mode = GetSetting(Set::PL_Mode);
	if(!Playlist::Mode::isActiveAndEnabled(mode.dynamic()))
	{
		return;
	}

	m->timer->stop();

	if(track.databaseId() != 0 || track.albumArtist().isEmpty())
	{
		return;
	}

	m->currentLibraryId = track.libraryId();
	m->currentArtistName = track.albumArtist();

	m->timer->start(500);
}

void Handler::similarArtistsAvailable()
{
	auto* fetcher = dynamic_cast<SimilarArtistFetcher*>(sender());

	const auto& artistMatch = fetcher->similarArtists();
	if(!artistMatch.isValid())
	{
		fetcher->deleteLater();
		return;
	}

	const auto artistIds = evaluateArtistMatch(artistMatch, m->currentLibraryId);

	const auto artistTrackMap = getCandidateTracks(m->currentLibraryId, artistIds);
	const auto playlistFilepaths = getPlaylistFilepaths(m->playlistAccessor, artistIds);

	for(const auto& artistId: artistIds)
	{
		const auto track = findTrackNotInPlaylist(artistTrackMap[artistId], playlistFilepaths);
		if(track.isValid())
		{
			appendTrack(m->playlistAccessor, track);
			break;
		}
	}

	fetcher->deleteLater();
}

void DynamicPlayback::Handler::timeout()
{
	auto* thread = new QThread();
	auto* lfmFetcher = new LfmSimilarArtistFetcher(m->currentArtistName);

	connect(lfmFetcher,
	        &LfmSimilarArtistFetcher::sigFinished,
	        this,
	        &Handler::similarArtistsAvailable);
	connect(lfmFetcher, &SimilarArtistFetcher::sigFinished, thread, &QThread::quit);
	connect(thread, &QThread::started, lfmFetcher, &SimilarArtistFetcher::start);
	connect(thread, &QThread::finished, lfmFetcher, &SimilarArtistFetcher::deleteLater);
	connect(thread, &QThread::finished, thread, &QThread::deleteLater);

	lfmFetcher->moveToThread(thread);
	thread->start();
}
