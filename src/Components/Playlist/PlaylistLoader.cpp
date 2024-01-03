/* PlaylistLoader.cpp */

/* Copyright (C) 2011-2024 Michael Lugmair (Lucio Carreras)
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

#include <Utils/Algorithm.h>
#include "PlaylistLoader.h"

#include "Database/Connector.h"
#include "Database/Playlist.h"

#include "Utils/FileUtils.h"
#include "Utils/Algorithm.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Playlist/CustomPlaylist.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Tagging/Tagging.h"

namespace
{
	MetaDataList applyTags(MetaDataList tracks)
	{
		for(auto& track: tracks)
		{
			if(track.isExtern() && Util::File::isFile(track.filepath()))
			{
				Tagging::Utils::getMetaDataOfFile(track);
			}
		}

		return tracks;
	}

	QList<CustomPlaylist> applyTagsToPlaylists(QList<CustomPlaylist> playlists)
	{
		for(auto& playlist: playlists)
		{
			auto tracks = playlist.tracks();
			auto changedTracks = applyTags(std::move(tracks));
			playlist.setTracks(std::move(changedTracks));
		}

		return playlists;
	}

	QList<CustomPlaylist> getPlaylists(Playlist::StoreType type, Playlist::SortOrder sortOrder, bool getTracks)
	{
		auto* playlistConnector = DB::Connector::instance()->playlistConnector();
		auto playlists = playlistConnector->getAllPlaylists(type, getTracks, sortOrder);
		return getTracks
		       ? applyTagsToPlaylists(std::move(playlists))
		       : playlists;
	}
}

namespace Playlist
{
	struct LoaderImpl::Private
	{
		QList<CustomPlaylist> playlists;
		int lastPlaylistId;
		int lastTrackIndex;

		Private() :
			lastPlaylistId(GetSetting(Set::PL_LastPlaylist)),
			lastTrackIndex {-1} {}
	};

	LoaderImpl::LoaderImpl() :
		Loader()
	{
		m = Pimpl::make<Private>();

		const auto loadTemporaryPlaylists = GetSetting(Set::PL_LoadTemporaryPlaylists);
		const auto loadSavedPlaylists = GetSetting(Set::PL_LoadSavedPlaylists);
		const auto loadLastTrackBeforeStop = GetSetting(Set::PL_RememberTrackAfterStop);

		m->lastTrackIndex = GetSetting(Set::PL_LastTrack);
		if(m->lastTrackIndex == -1 && loadLastTrackBeforeStop)
		{
			m->lastTrackIndex = GetSetting(Set::PL_LastTrackBeforeStop);
		}

		if(loadSavedPlaylists && loadTemporaryPlaylists)
		{
			m->playlists = getPlaylists(StoreType::TemporaryAndPermanent, Playlist::SortOrder::IDAsc, true);
		}

		else if(loadSavedPlaylists && !loadTemporaryPlaylists)
		{
			m->playlists = getPlaylists(StoreType::OnlyPermanent, Playlist::SortOrder::IDAsc, true);
		}

		else if(!loadSavedPlaylists && loadTemporaryPlaylists)
		{
			m->playlists = getPlaylists(StoreType::OnlyTemporary, Playlist::SortOrder::IDAsc, true);
		}

		else
		{
			m->playlists.clear();
		}

		if(m->playlists.isEmpty())
		{
			return;
		}

		if(m->lastPlaylistId >= 0)
		{
			const auto hasPlaylistId = Util::Algorithm::contains(m->playlists, [&](const auto& playlist) {
				return (m->lastPlaylistId == playlist.id());
			});

			if(!hasPlaylistId)
			{
				auto* playlistConnector = DB::Connector::instance()->playlistConnector();
				const auto playlist = playlistConnector->getPlaylistById(m->lastPlaylistId, true);
				m->playlists.prepend(playlist);
			}
		}
	}

	LoaderImpl::~LoaderImpl() = default;

	int LoaderImpl::getLastPlaylistIndex() const
	{
		return Util::Algorithm::indexOf(m->playlists, [&](const auto& playlist) {
			return (playlist.id() == m->lastPlaylistId);
		});
	}

	int LoaderImpl::getLastTrackIndex() const
	{
		if(!GetSetting(Set::PL_LoadLastTrack))
		{
			return -1;
		}

		const auto lastPlaylistIndex = getLastPlaylistIndex();
		if(!Util::between(lastPlaylistIndex, m->playlists.size()))
		{
			return -1;
		}

		const auto trackCount = m->playlists[lastPlaylistIndex].tracks().count();
		if((trackCount > 0) && (m->lastTrackIndex >= trackCount))
		{
			return 0;
		}

		if(!Util::between(m->lastTrackIndex, trackCount))
		{
			return -1;
		}

		return m->lastTrackIndex;
	}

	const QList<CustomPlaylist>& LoaderImpl::playlists() const
	{
		return m->playlists;
	}
}