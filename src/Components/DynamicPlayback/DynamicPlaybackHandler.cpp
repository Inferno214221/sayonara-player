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
#include "ArtistMatch.h"
#include "ArtistMatchEvaluator.h"
#include "SimilarArtistFetcher.h"

#include "Components/PlayManager/PlayManager.h"
#include "Components/Playlist/Playlist.h"
#include "Components/Playlist/PlaylistInterface.h"
#include "Components/Playlist/PlaylistModifiers.h"
#include "Database/Connector.h"
#include "Database/LibraryDatabase.h"
#include "Utils/Algorithm.h"
#include "Utils/FileSystem.h"
#include "Utils/MetaData/MetaData.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/RandomGenerator.h"
#include "Utils/Set.h"
#include "Utils/Settings/Settings.h"
#include "Utils/WebAccess/WebClientFactory.h"

#include <QTimer>
#include <QThread>
#include <Utils/FileUtils.h>

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

		for(auto& albumArtistTracks: result)
		{
			Util::Algorithm::shuffle(albumArtistTracks);
		}

		return result;
	}

	Util::Set<QString> getPlaylistFilepaths(Playlist::Accessor* playlistAccessor, const QList<ArtistId>& artistIds)
	{
		Util::Set<QString> playlistFilepaths;

		auto playlist = playlistAccessor->activePlaylist();

		for(const auto& track: playlist->tracks())
		{
			const auto hasInvalidArtist = (track.artistId() < 0) && (track.albumArtistId() < 0);

			if(hasInvalidArtist ||
			   artistIds.contains(track.artistId()) ||
			   artistIds.contains(track.albumArtistId()))
			{
				playlistFilepaths.insert(track.filepath());
			}
		}

		return playlistFilepaths;
	}

	MetaData findTrackNotInPlaylist(const MetaDataList& tracks, const Util::Set<QString>& playlistFilepaths,
	                                const Util::FileSystemPtr& fileSystem)
	{
		for(const auto& track: tracks)
		{
			const auto contains = playlistFilepaths.contains(track.filepath());
			if(!contains && fileSystem->exists(track.filepath()))
			{
				return track;
			}
		}

		return MetaData {};
	}
}

namespace DynamicPlayback
{
	struct Handler::Private
	{
		QString currentArtistName;
		LibraryId currentLibraryId {-1};
		Playlist::Accessor* playlistAccessor;
		SimilarArtistFetcherFactoryPtr similarArtistFetcherFactory;
		Util::FileSystemPtr fileSystem;
		std::shared_ptr<QTimer> timer {std::make_shared<QTimer>()};
		QMap<QString, ArtistMatch> similarArtistCache;

		Private(Playlist::Accessor* playlistAccessor, SimilarArtistFetcherFactoryPtr similarArtistFetcherFactory,
		        Util::FileSystemPtr fileSystem) :
			playlistAccessor {playlistAccessor},
			similarArtistFetcherFactory {std::move(similarArtistFetcherFactory)},
			fileSystem {std::move(fileSystem)}
		{
			timer->setSingleShot(true);
		}
	};

	Handler::Handler(PlayManager* playManager, Playlist::Accessor* playlistAccessor,
	                 const SimilarArtistFetcherFactoryPtr& similarArtistFetcherFactory,
	                 const Util::FileSystemPtr& fileSystem, QObject* parent) :
		QObject(parent),
		m {Pimpl::make<Private>(playlistAccessor, similarArtistFetcherFactory, fileSystem)}
	{
		connect(playManager, &PlayManager::sigCurrentTrackChanged, this, &Handler::currentTrackChanged);
		connect(playManager, &PlayManager::sigPlaystateChanged, this, [timer = m->timer](const auto playState) {
			if(playState == PlayState::Stopped)
			{
				timer->stop();
			}
		});

		connect(m->timer.get(), &QTimer::timeout, this, &Handler::timeout);
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

		m->timer->start(500); // NOLINT(*-magic-numbers)
		//timeout();
	}

	void Handler::similarArtistsAvailable()
	{
		auto* fetcher = dynamic_cast<SimilarArtistFetcher*>(sender());

		const auto& artistMatch = fetcher->similarArtists();
		if(artistMatch.isValid())
		{
			processArtistMatch(artistMatch);
		}

		fetcher->deleteLater();
	}

	void Handler::timeout()
	{
		if(const auto it = m->similarArtistCache.find(m->currentArtistName); it != m->similarArtistCache.end())
		{
			processArtistMatch(it.value());
			return;
		}

		auto* thread = new QThread();
		auto* fetcher = m->similarArtistFetcherFactory->create(m->currentArtistName);

		connect(fetcher, &SimilarArtistFetcher::sigFinished, this, &Handler::similarArtistsAvailable);
		connect(fetcher, &SimilarArtistFetcher::sigFinished, thread, &QThread::quit);
		connect(thread, &QThread::started, fetcher, &SimilarArtistFetcher::start);
		connect(thread, &QThread::finished, fetcher, &SimilarArtistFetcher::deleteLater);
		connect(thread, &QThread::finished, thread, &QThread::deleteLater);

		fetcher->moveToThread(thread);
		thread->start();
	}

	void Handler::processArtistMatch(const DynamicPlayback::ArtistMatch& artistMatch)
	{
		const auto correctedArtist = artistMatch.artistName();
		if(!m->similarArtistCache.contains(correctedArtist))
		{
			m->similarArtistCache[correctedArtist] = artistMatch;
		}

		const auto artistIds = evaluateArtistMatch(artistMatch, m->currentLibraryId);

		const auto artistTrackMap = getCandidateTracks(m->currentLibraryId, artistIds);
		const auto playlistFilepaths = getPlaylistFilepaths(m->playlistAccessor, artistIds);

		for(const auto& artistId: artistIds)
		{
			const auto track = findTrackNotInPlaylist(artistTrackMap[artistId], playlistFilepaths, m->fileSystem);
			if(track.isValid())
			{
				appendTrack(m->playlistAccessor, track);
				break;
			}
		}
	}
}