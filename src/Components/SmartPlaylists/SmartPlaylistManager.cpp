/* SmartPlaylistManager.cpp */
/*
 * Copyright (C) 2011-2022 Michael Lugmair
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

#include "SmartPlaylistManager.h"
#include "SmartPlaylistByRating.h"
#include "SmartPlaylistCreator.h"

#include "Database/Connector.h"
#include "Database/Library.h"
#include "Database/LibraryDatabase.h"
#include "Database/SmartPlaylists.h"
#include "Interfaces/PlaylistInterface.h"
#include "Utils/MetaData/MetaDataList.h"

struct SmartPlaylistManager::Private
{
	PlaylistCreator* playlistCreator;
	QList<SmartPlaylistPtr> smartPlaylists;

	explicit Private(PlaylistCreator* playlistCreator) :
		playlistCreator {playlistCreator}
	{
		auto* db = DB::Connector::instance()->smartPlaylistsConnector();
		const auto smartPlaylistDatabaseEntries = db->getAllSmartPlaylists();
		for(const auto& entry: smartPlaylistDatabaseEntries)
		{
			const auto smartPlaylist = SmartPlaylists::create(entry);
			if(smartPlaylist)
			{
				smartPlaylists << smartPlaylist;
			}
		}
	}
};

SmartPlaylistManager::SmartPlaylistManager(PlaylistCreator* playlistCreator)
{
	m = Pimpl::make<Private>(playlistCreator);
}

SmartPlaylistManager::~SmartPlaylistManager() = default;

SmartPlaylistPtr SmartPlaylistManager::smartPlaylist(const int index) const
{
	return m->smartPlaylists[index];
}

QList<SmartPlaylistPtr> SmartPlaylistManager::smartPlaylists() const
{
	return m->smartPlaylists;
}

void SmartPlaylistManager::selectPlaylist(const int index)
{
	const auto smartPlaylist = m->smartPlaylists[index];
	auto tracks = MetaDataList {};

	if(!smartPlaylist->canFetchTracks())
	{
		const auto* libraryDatabase = DB::Connector::instance()->libraryDatabase(-1, 0);
		libraryDatabase->getAllTracks(tracks);
	}

	const auto filteredTracks = smartPlaylist->filterTracks(std::move(tracks));

	m->playlistCreator->createPlaylist(filteredTracks, smartPlaylist->name(), true);
}

void SmartPlaylistManager::deletePlaylist(const int index)
{
	auto* db = DB::Connector::instance()->smartPlaylistsConnector();
	const auto success = db->deleteSmartPlaylist(m->smartPlaylists[index]->id());
	if(success)
	{
		m->smartPlaylists.removeAt(index);
		emit sigPlaylistsChanged();
	}
}

void SmartPlaylistManager::insertPlaylist(const SmartPlaylistPtr& smartPlaylist)
{
	auto* db = DB::Connector::instance()->smartPlaylistsConnector();
	const auto id = db->insertSmartPlaylist(smartPlaylist->toDatabaseEntry());
	if(id >= 0)
	{
		smartPlaylist->setId(id);
		m->smartPlaylists.append(smartPlaylist);
		emit sigPlaylistsChanged();
	}
}

void SmartPlaylistManager::updatePlaylist(const int index, const SmartPlaylistPtr& smartPlaylist)
{
	auto* db = DB::Connector::instance()->smartPlaylistsConnector();
	const auto id = m->smartPlaylists[index]->id();
	const auto success = db->updateSmartPlaylist(id, smartPlaylist->toDatabaseEntry());
	if(success)
	{
		m->smartPlaylists[index] = smartPlaylist;
		emit sigPlaylistsChanged();
	}
}
