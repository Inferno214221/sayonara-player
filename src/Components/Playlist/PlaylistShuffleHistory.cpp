/* PlaylistShuffleBehavior.cpp */
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

#include "PlaylistShuffleHistory.h"
#include "Playlist.h"
#include "PlaylistModifiers.h"

#include "Utils/Algorithm.h"
#include "Utils/Utils.h"
#include "Utils/MetaData/MetaData.h"
#include "Utils/MetaData/MetaDataList.h"

#include <QList>

namespace Playlist
{
	namespace
	{
		QList<int> getUnplayedTrackIndexes(Playlist* playlist, const QList<UniqueId>& shuffleHistory)
		{
			const auto& tracks = playlist->tracks();
			auto trackIndex = 0;
			auto unplayedTracks = QList<int>();
			for(const auto& track: tracks)
			{
				const auto uniqueId = track.uniqueId();
				if(!shuffleHistory.contains(uniqueId))
				{
					unplayedTracks << trackIndex;
				}

				trackIndex++;
			}

			return unplayedTracks;
		}
	}

	struct ShuffleHistory::Private
	{
		QList<UniqueId> shuffleHistory;
		Playlist* playlist;

		explicit Private(Playlist* playlist) :
			playlist {playlist} {}
	};

	ShuffleHistory::ShuffleHistory(Playlist* playlist)
	{
		m = Pimpl::make<Private>(playlist);
	}

	ShuffleHistory::~ShuffleHistory() = default;

	void ShuffleHistory::addTrack(const MetaData& track)
	{
		m->shuffleHistory << track.uniqueId();
	}

	int ShuffleHistory::previousShuffleIndex() const
	{
		for(auto historyIndex = m->shuffleHistory.size() - 2; historyIndex >= 0; historyIndex--)
		{
			const auto uniqueId = m->shuffleHistory[historyIndex];
			const auto index = Util::Algorithm::indexOf(m->playlist->tracks(), [&](const auto& track) {
				return (uniqueId == track.uniqueId());
			});

			if(index >= 0)
			{
				return index;
			}
		}

		return -1;
	}

	int ShuffleHistory::nextTrackIndex(bool isRepeatAllEnabled) const
	{
		const auto& tracks = m->playlist->tracks();
		if(tracks.isEmpty())
		{
			return -1;
		}

		const auto unplayedTracks = getUnplayedTrackIndexes(m->playlist, m->shuffleHistory);
		if(unplayedTracks.isEmpty())
		{
			if(!isRepeatAllEnabled)
			{
				return -1;
			}

			m->shuffleHistory.clear();
		}

		if(m->shuffleHistory.isEmpty() && ::Playlist::count(*m->playlist) > 1)
		{
			return Util::randomNumber(1, static_cast<int>(tracks.size() - 1));
		}

		const auto randomIndex = Util::randomNumber(0, unplayedTracks.size() - 1);
		return unplayedTracks[randomIndex];
	}

	void ShuffleHistory::replaceTrack(const MetaData& oldTrack, const MetaData& newTrack)
	{
		auto i = 0;
		for(const auto& uniqueId: m->shuffleHistory)
		{
			if(uniqueId == oldTrack.uniqueId())
			{
				m->shuffleHistory.replace(i, newTrack.uniqueId());
			}

			i++;
		}
	}

	void ShuffleHistory::popBack()
	{
		if(m->shuffleHistory.count() > 0)
		{
			m->shuffleHistory.pop_back();
		}
	}

	void ShuffleHistory::clear()
	{
		m->shuffleHistory.clear();
	}
}