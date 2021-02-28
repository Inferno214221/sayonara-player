/* PlaylistLibraryInteractor.cpp */
/*
 * Copyright (C) 2011-2021 Michael Lugmair
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
#include "PlaylistLibraryInteractor.h"

#include "Components/Playlist/PlaylistHandler.h"
#include "Components/Playlist/Playlist.h"
#include "Components/Library/LocalLibrary.h"

#include "Interfaces/LibraryInfoAccessor.h"

#include "Utils/Algorithm.h"
#include "Utils/FileUtils.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/MetaData/MetaData.h"

#include <QStringList>

#include <unordered_map>

using Playlist::Handler;
using Playlist::LibraryInteractor;

struct LibraryInteractor::Private
{
	Handler* playlistHandler;
	LibraryInfoAccessor* libraryInfoAccessor;

	Private(Handler* playlistHandler, LibraryInfoAccessor* libraryInfoAccessor) :
		playlistHandler {playlistHandler},
		libraryInfoAccessor {libraryInfoAccessor} {}
};

LibraryInteractor::LibraryInteractor(Handler* playlistHandler, LibraryInfoAccessor* libraryInfoAccessor) :
	QObject(nullptr)
{
	m = Pimpl::make<Private>(playlistHandler, libraryInfoAccessor);

	connect(playlistHandler, &Handler::sigNewPlaylistAdded, this, [&](const auto index){
		auto playlist = playlistHandler->playlist(index);
		initPlaylistConnections(playlist);
	});

	for(int i=0; i<playlistHandler->count(); i++)
	{
		auto playlist = playlistHandler->playlist(i);
		initPlaylistConnections(playlist);
	}
}

LibraryInteractor::~LibraryInteractor() = default;

void LibraryInteractor::findTrackRequested(const MetaData& track)
{
	if(track.libraryId() >= 0 && track.id() >= 0)
	{
		auto* localLibrary = m->libraryInfoAccessor->libraryInstance(track.libraryId());
		localLibrary->findTrack(track.id());
	}
}

void LibraryInteractor::deleteTracksReqeuested(const MetaDataList& tracks)
{
	std::unordered_map<LibraryId, MetaDataList> libraryMap;

	for(const auto& track : tracks)
	{
		libraryMap[track.libraryId()].push_back(track);
	}

	for(const auto&[libraryId, tracks] : libraryMap)
	{
		if(libraryId >= 0)
		{
			auto* localLibrary = m->libraryInfoAccessor->libraryInstance(libraryId);
			if(localLibrary)
			{

				localLibrary->deleteTracks(tracks, Library::TrackDeletionMode::AlsoFiles);
			}
		}

		else {
			QStringList paths;
			Util::Algorithm::transform(tracks, paths, [](const auto& track){
				return track.filepath();
			});

			Util::File::deleteFiles(paths);
		}
	}
}

void LibraryInteractor::initPlaylistConnections(PlaylistPtr playlist)
{
	auto ptr = playlist.get();

	connect(ptr, &Playlist::sigFindTrackRequested, this, &LibraryInteractor::findTrackRequested);
	connect(ptr, &Playlist::sigDeleteFilesRequested, this, &LibraryInteractor::deleteTracksReqeuested);
}