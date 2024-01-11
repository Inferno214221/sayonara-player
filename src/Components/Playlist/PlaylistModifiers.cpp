/* PlaylistModifiers.cpp */
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

#include "PlaylistModifiers.h"

#include "Playlist.h"

#include "Utils/Algorithm.h"
#include "Utils/FileUtils.h"
#include "Utils/MetaData/MetaData.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/MetaData/MetaDataSorting.h"
#include "Utils/Settings/Settings.h"
#include "Utils/RandomGenerator.h"
#include "Utils/Set.h"

namespace Playlist
{
	void reverse(Playlist& playlist, const Reason reason)
	{
		playlist.modifyTracks([](auto tracks) {
			std::reverse(tracks.begin(), tracks.end());
			return tracks;
		}, reason, Operation::Arrange);
	}

	void randomize(Playlist& playlist, const Reason reason)
	{
		playlist.modifyTracks([](auto tracks) {
			Util::Algorithm::shuffle(tracks);
			return tracks;
		}, reason, Operation::Arrange);
	}

	void sortTracks(Playlist& playlist, const ::Library::SortOrder sortOrder, const Reason reason)
	{
		playlist.modifyTracks([&](auto tracks) {
			MetaDataSorting::sortMetadata(tracks, sortOrder, GetSetting(Set::Lib_SortModeMask));
			return tracks;
		}, reason, Operation::Arrange);
	}

	IndexSet moveTracks(Playlist& playlist, const IndexSet& indexes, int targetRow, const Reason reason)
	{
		const auto lineCountBeforeTarget = Util::Algorithm::count(indexes, [&](const auto index) {
			return (index < targetRow);
		});

		IndexSet newTrackPositions;
		for(auto i = targetRow; i < targetRow + indexes.count(); i++)
		{
			newTrackPositions.insert(i - lineCountBeforeTarget);
		}

		playlist.modifyTracks([&](auto tracks) {
			tracks.moveTracks(indexes, targetRow);
			return tracks;
		}, reason, Operation::Arrange);

		return newTrackPositions;
	}

	IndexSet copyTracks(Playlist& playlist, const IndexSet& indexes, const int targetRow, const Reason reason)
	{
		IndexSet newTrackPositions;
		for(auto i = 0; i < indexes.count(); i++)
		{
			newTrackPositions.insert(targetRow + i);
		}

		playlist.modifyTracks([&](auto tracks) {
			tracks.copyTracks(indexes, targetRow);
			return tracks;
		}, reason, Operation::Duplicate);

		return newTrackPositions;
	}

	void removeTracks(Playlist& playlist, const IndexSet& indexes, const Reason reason)
	{
		playlist.modifyTracks([&](auto tracks) {
			tracks.removeTracks(indexes);
			return tracks;
		}, reason, Operation::Remove);
	}

	void insertTracks(Playlist& playlist, const MetaDataList& newTracks, int targetIndex, const Reason reason)
	{
		playlist.modifyTracks([&](auto tracks) {
			tracks.insertTracks(newTracks, targetIndex);
			return tracks;
		}, reason, Operation::Insert);
	}

	void appendTracks(Playlist& playlist, const MetaDataList& newTracks, const Reason reason)
	{
		if(playlist.isBusy())
		{
			return;
		}

		const auto oldTrackCount = count(playlist);

		playlist.modifyTracks([&](auto tracks) {
			tracks << newTracks;
			std::for_each(tracks.begin() + oldTrackCount, tracks.end(), [](auto& track) {
				const auto isDisabled = track.isDisabled() || !Util::File::checkFile(track.filepath());
				track.setDisabled(isDisabled);
			});
			return tracks;
		}, reason, Operation::Append);
	}

	void enableAll(Playlist& playlist, const Reason reason)
	{
		playlist.modifyTracks([](auto tracks) {
			for(auto& track: tracks)
			{
				track.setDisabled(false);
			}

			return tracks;
		}, reason, Operation::EnableAll);
	}

	void clear(Playlist& playlist, const Reason reason)
	{
		if(!playlist.tracks().isEmpty())
		{
			playlist.modifyTracks([&](auto /*tracks*/) {
				return MetaDataList {};
			}, reason, Operation::Clear);
		}
	}

	int count(const Playlist& playlist)
	{
		return playlist.tracks().count();
	}

	void jumpToNextAlbum(Playlist& playlist)
	{
		const auto currentIndex = playlist.currentTrackIndex();
		if(currentIndex < 0)
		{
			playlist.next();
		}
		else
		{
			const auto& tracks = playlist.tracks();
			const auto& currentTrack = tracks.at(currentIndex);
			const auto albumId = currentTrack.albumId();
			const auto it = std::find_if(tracks.begin() + currentIndex, tracks.end(), [&](const auto& track) {
				return (track.albumId() != albumId);
			});
			if(it != tracks.end())
			{
				const auto index = static_cast<int>(std::distance(tracks.begin(), it));
				playlist.changeTrack(index);
			}
		}
	}

	MilliSeconds runningTime(const Playlist& playlist)
	{
		const auto& tracks = playlist.tracks();
		const auto durationMs =
			std::accumulate(tracks.begin(), tracks.end(), 0, [](const auto timeMs, const auto& track) {
				return timeMs + track.durationMs();
			});

		return durationMs;
	}

	int currentTrackWithoutDisabled(const Playlist& playlist)
	{
		const auto currentTrackIndex = playlist.currentTrackIndex();
		const auto& tracks = playlist.tracks();

		if(!Util::between(currentTrackIndex, count(playlist)) || tracks[currentTrackIndex].isDisabled())
		{
			return -1;
		}

		const auto disabled =
			std::count_if(tracks.begin(), tracks.begin() + currentTrackIndex, [](const auto& track) {
				return track.isDisabled();
			});

		return std::max(currentTrackIndex - static_cast<int>(disabled), -1);
	}
}
