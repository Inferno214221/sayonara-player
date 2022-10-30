/* Playlist.cpp */

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
 *
 *  Created on: Apr 6, 2011
 *      Author: Michael Lugmair (Lucio Carreras)
 */

#include "PlaylistHandler.h"

#include "Playlist.h"
#include "PlaylistChangeNotifier.h"
#include "PlaylistFromPathCreator.h"
#include "PlaylistLoader.h"
#include "PlaylistModifiers.h"
#include "PlaylistSaver.h"
#include "PlayManager/PlayManager.h"
#include "Utils/Algorithm.h"
#include "Utils/Logger/Logger.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Playlist/CustomPlaylist.h"
#include "Utils/Set.h"
#include "Utils/Settings/Settings.h"

#include <cassert>

using Playlist::Handler;
using Playlist::Loader;

struct Handler::Private
{
	QList<PlaylistPtr> playlists;
	PlayManager* playManager;
	int currentPlaylistIndex {-1};

	Private(PlayManager* playManager) :
		playManager {playManager} {}

	void initPlaylists(Handler* handler, std::shared_ptr<::Playlist::Loader> playlistLoader)
	{
		spLog(Log::Debug, this) << "Loading playlists...";

		const auto& playlists = playlistLoader->playlists();
		if(playlists.isEmpty())
		{
			handler->createEmptyPlaylist();
			return;
		}

		for(const auto& playlist: playlists)
		{
			handler->createPlaylist(playlist);
		}

		const auto lastIndex = playlistLoader->getLastPlaylistIndex();
		const auto currentIndex = std::max(0, lastIndex);
		handler->setCurrentIndex(currentIndex);

		auto lastTrackIndex = playlistLoader->getLastTrackIndex();
		if(lastTrackIndex >= 0)
		{
			auto lastPlaylist = handler->playlist(currentIndex);
			lastPlaylist->setCurrentTrack(lastTrackIndex);
		}

		else
		{
			this->playManager->stop();
		}
	}
};

Handler::Handler(PlayManager* playManager, std::shared_ptr<::Playlist::Loader> playlistLoader) :
	QObject(),
	PlaylistCreator()
{
	m = Pimpl::make<Private>(playManager);
	m->initPlaylists(this, playlistLoader);

	connect(m->playManager, &PlayManager::sigPlaystateChanged, this, &Handler::playstateChanged);
	connect(m->playManager, &PlayManager::sigNext, this, &Handler::next);
	connect(m->playManager, &PlayManager::sigWakeup, this, &Handler::wakeUp);
	connect(m->playManager, &PlayManager::sigPrevious, this, &Handler::previous);
	connect(m->playManager, &PlayManager::sigStreamFinished, this, &Handler::wwwTrackFinished);

	auto* playlistChangeNotifier = PlaylistChangeNotifier::instance();
	connect(playlistChangeNotifier, &PlaylistChangeNotifier::sigPlaylistRenamed, this, &Handler::playlistRenamed);
	connect(playlistChangeNotifier, &PlaylistChangeNotifier::sigPlaylistDeleted, this, &Handler::playlistDeleted);
}

Handler::~Handler() = default;

void Handler::shutdown()
{
	::Playlist::saveCurrentPlaylists(m->playlists);

	m->playlists.clear();
	m->currentPlaylistIndex = -1;
}

int Handler::addNewPlaylist(const QString& name, bool temporary)
{
	const auto index = exists(name);
	if(index >= 0)
	{
		return index;
	}

	auto playlist = std::make_shared<Playlist>(m->playlists.count(), name, m->playManager);
	playlist->setTemporary(temporary);

	m->playlists.push_back(playlist);

	emit sigNewPlaylistAdded(m->playlists.count() - 1);
	emit sigActivePlaylistChanged(activeIndex());

	connect(playlist.get(), &Playlist::Playlist::sigTrackChanged, this, &Handler::trackChanged);

	return playlist->index();
}

int Handler::createPlaylist(const MetaDataList& tracks, const QString& name, bool temporary)
{
	const auto index = addNewPlaylist(name, temporary);

	auto& playlist = m->playlists[index];
	if(!playlist->isBusy())
	{
		playlist->createPlaylist(tracks);
		playlist->setTemporary(playlist->isTemporary() && temporary);
		if(playlist->isTemporary())
		{
			playlist->save();
		}
	}

	setCurrentIndex(index);
	return m->currentPlaylistIndex;
}

int Handler::createPlaylist(const CustomPlaylist& customPlaylist)
{
	const auto index = createPlaylist(customPlaylist.tracks(), customPlaylist.name(), customPlaylist.isTemporary());

	auto& playlist = m->playlists[index];
	playlist->setId(customPlaylist.id());
	playlist->setChanged(false);

	return m->currentPlaylistIndex;
}

int Handler::createPlaylist(const QStringList& paths, const QString& name, bool temporary)
{
	auto* playlistFromPathCreator = new PlaylistFromPathCreator(this);
	connect(playlistFromPathCreator, &PlaylistFromPathCreator::sigAllPlaylistsCreated, [playlistFromPathCreator](int) {
		playlistFromPathCreator->deleteLater();
	});

	const auto index = playlistFromPathCreator->createPlaylists(paths, name, temporary);
	if(index >= 0)
	{
		setCurrentIndex(index);
	}

	return m->currentPlaylistIndex;
}

int Handler::createCommandLinePlaylist(const QStringList& paths)
{
	auto* playlistFromPathCreator = new PlaylistFromPathCreator(this);
	connect(playlistFromPathCreator,
	        &PlaylistFromPathCreator::sigAllPlaylistsCreated,
	        [&, playlistFromPathCreator](auto firstIndex) {
		        if(m->playManager->initialPositionMs() > 0)
		        {
			        m->playManager->stop();
		        }

		        m->playManager->setCurrentPositionMs(0);
		        playlist(firstIndex)->setCurrentTrack(0);
		        setCurrentIndex(firstIndex);
		        playlistFromPathCreator->deleteLater();
	        });

	const auto filesystemPlaylistName = ::Playlist::filesystemPlaylistName();
	const auto playlistName = filesystemPlaylistName.isEmpty()
	                          ? requestNewPlaylistName()
	                          : filesystemPlaylistName;

	playlistFromPathCreator->createPlaylists(paths, playlistName, true);

	return m->currentPlaylistIndex;
}

int Handler::createEmptyPlaylist(bool override)
{
	const auto name = (override)
	                  ? QString()
	                  : requestNewPlaylistName();

	return createPlaylist(MetaDataList(), name, true);
}

void Handler::playstateChanged(PlayState state)
{
	if(state == PlayState::Playing)
	{
		activePlaylist()->play();
	}

	else if(state == PlayState::Stopped)
	{
		for(auto playlist: m->playlists)
		{
			playlist->stop();
		}
	}
}

void Handler::next()
{
	auto playlist = activePlaylist();
	playlist->next();
	if(playlist->currentTrackIndex() < 0)
	{
		m->playManager->stop();
	}
}

void Handler::previous()
{
	activePlaylist()->bwd();
}

void Handler::wakeUp()
{
	const auto restoreTrackAfterStop = GetSetting(Set::PL_RememberTrackAfterStop);
	if(restoreTrackAfterStop && activePlaylist()->wakeUp())
	{
		return;
	}

	next();
}

void Handler::trackChanged([[maybe_unused]] int oldIndex, [[maybe_unused]] int newIndex)
{
	auto* playlist = static_cast<Playlist*>(sender());
	if(playlist->currentTrackIndex() >= 0)
	{
		for(auto playlistPtr: m->playlists)
		{
			if(playlist->index() != playlistPtr->index())
			{
				playlistPtr->stop();
			}
		}
	}
}

int Handler::activeIndex() const
{
	const auto index = Util::Algorithm::indexOf(m->playlists, [](const auto& playlist) {
		return (playlist->currentTrackIndex() >= 0);
	});

	if(Util::between(index, m->playlists))
	{
		return index;
	}

	if(Util::between(currentIndex(), m->playlists))
	{
		return currentIndex();
	}

	assert(count() > 0);
	return 0;
}

int Handler::currentIndex() const
{
	return m->currentPlaylistIndex;
}

void Handler::setCurrentIndex(int playlistIndex)
{
	if(Util::between(playlistIndex, m->playlists))
	{
		if(m->currentPlaylistIndex != playlistIndex)
		{
			m->currentPlaylistIndex = playlistIndex;
			emit sigCurrentPlaylistChanged(playlistIndex);
		}
	}
}

PlaylistPtr Handler::activePlaylist()
{
	return m->playlists[activeIndex()];
}

int Handler::count() const
{
	return m->playlists.size();
}

QString Handler::requestNewPlaylistName(const QString& prefix) const
{
	return ::Playlist::requestNewDatabaseName(prefix);
}

void Handler::closePlaylist(int playlistIndex)
{
	auto playlist = this->playlist(playlistIndex);
	if(playlist && playlist->isTemporary())
	{
		playlist->deletePlaylist();
	}

	m->playlists.removeAt(playlistIndex);

	for(auto remaningPlaylist: m->playlists)
	{
		if((remaningPlaylist->index() >= playlistIndex) &&
		   (remaningPlaylist->index() > 0))
		{
			remaningPlaylist->setIndex(remaningPlaylist->index() - 1);
		}
	}

	if(m->playlists.isEmpty())
	{
		addNewPlaylist(this->requestNewPlaylistName(), true);
	}

	if(m->currentPlaylistIndex >= m->playlists.count())
	{
		setCurrentIndex(m->currentPlaylistIndex - 1);
	}

	const auto activePlaylist = this->activePlaylist();
	const auto lastPlaylistId = (activePlaylist) ? activePlaylist->id() : -1;
	const auto lastTrack = (activePlaylist) ? currentTrackWithoutDisabled(*activePlaylist) : -1;

	SetSetting(Set::PL_LastTrack, lastTrack);
	SetSetting(Set::PL_LastPlaylist, lastPlaylistId);

	emit sigPlaylistClosed(playlistIndex);
}

PlaylistPtr Handler::playlist(int playlistIndex)
{
	return (Util::between(playlistIndex, m->playlists.count()))
	       ? m->playlists[playlistIndex]
	       : nullptr;
}

PlaylistPtr Handler::playlistById(int playlistId)
{
	const auto index =
		Util::Algorithm::indexOf(m->playlists, [&](const auto playlist) {
			return (playlist->id() == playlistId);
		});

	return playlist(index);
}

int Handler::exists(const QString& name) const
{
	if(name.isEmpty() && Util::between(m->currentPlaylistIndex, m->playlists))
	{
		return m->currentPlaylistIndex;
	}

	return Util::Algorithm::indexOf(m->playlists, [&name](PlaylistPtr pl) {
		return (pl->name().compare(name, Qt::CaseInsensitive) == 0);
	});
}

void Handler::playlistRenamed(int id, const QString& /*oldName*/, const QString& /*newName*/)
{
	const auto playlist = playlistById(id);
	if(playlist)
	{
		emit sigPlaylistNameChanged(playlist->index());
	}
}

void Handler::playlistDeleted(int id)
{
	auto playlist = playlistById(id);
	if(playlist)
	{
		playlist->setTemporary(true);
		playlist->setChanged(false);
	}
}

void Handler::wwwTrackFinished(const MetaData& track)
{
	if(GetSetting(Set::Stream_ShowHistory))
	{
		auto playlist = activePlaylist();
		::Playlist::insertTracks(*playlist, MetaDataList {track}, playlist->currentTrackIndex());
	}
}
