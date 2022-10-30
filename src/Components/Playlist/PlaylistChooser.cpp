/* PlaylistChooser.cpp */

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
 */

#include "PlaylistChooser.h"
#include "PlaylistChangeNotifier.h"
#include "PlaylistDBWrapper.h"

#include "Interfaces/PlaylistInterface.h"
#include "Database/Connector.h"
#include "Database/Playlist.h"

#include "Utils/Algorithm.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Playlist/Sorting.h"
#include "Utils/Playlist/CustomPlaylist.h"

#include <QStringList>

namespace Algorithm = Util::Algorithm;

namespace Playlist
{
	struct Chooser::Private
	{
		QList<CustomPlaylist> playlists;
		PlaylistCreator* playlistCreator;
		DB::Playlist* playlistConnector;

		explicit Private(PlaylistCreator* playlistCreator) :
			playlistCreator {playlistCreator},
			playlistConnector {DB::Connector::instance()->playlistConnector()}
		{
			playlists = loadPlaylists(StoreType::OnlyPermanent, SortOrder::NameAsc, false);
		}
	};

	Chooser::Chooser(PlaylistCreator* playlistCreator, QObject* parent) :
		QObject(parent)
	{
		m = Pimpl::make<Private>(playlistCreator);

		auto* pcn = ChangeNotifier::instance();
		connect(pcn, &ChangeNotifier::sigPlaylistAdded, this, &Chooser::playlistAdded);
		connect(pcn, &ChangeNotifier::sigPlaylistRenamed, this, &Chooser::playlistRenamed);
		connect(pcn, &ChangeNotifier::sigPlaylistDeleted, this, &Chooser::playlistDeleted);
	}

	Chooser::~Chooser() = default;

	const QList<CustomPlaylist>& Chooser::playlists()
	{
		return m->playlists;
	}

	Util::SaveAsAnswer Chooser::renamePlaylist(int id, const QString& newName)
	{
		using Util::SaveAsAnswer;

		if(newName.isEmpty())
		{
			return SaveAsAnswer::InvalidName;
		}

		if(id < 0)
		{
			return SaveAsAnswer::InvalidObject;
		}

		auto nameExists = false;
		QString oldName;
		for(const auto& playlist: m->playlists)
		{
			if(playlist.name() == newName)
			{
				nameExists = true;
			}

			if(playlist.id() == id)
			{
				oldName = playlist.name();
			}
		}

		if(nameExists)
		{
			spLog(Log::Warning, this) << "Playlist " << newName << " exists already";
			return SaveAsAnswer::NameAlreadyThere;
		}

		const auto success = m->playlistConnector->renamePlaylist(id, newName);
		if(success)
		{
			ChangeNotifier::instance()->renamePlaylist(id, oldName, newName);
			return SaveAsAnswer::Success;
		}

		return SaveAsAnswer::OtherError;
	}

	bool Chooser::deletePlaylist(int id)
	{
		const auto playlist = m->playlistConnector->getPlaylistById(id, false);
		const auto success = m->playlistConnector->updatePlaylist(id, playlist.name(), true);
		if(success)
		{
			ChangeNotifier::instance()->deletePlaylist(id);
		}

		return success;
	}

	void Chooser::loadSinglePlaylist(int id)
	{
		if(id >= 0)
		{
			const auto playlist = m->playlistConnector->getPlaylistById(id, true);
			m->playlistCreator->createPlaylist(playlist);
		}
	}

	int Chooser::findPlaylist(const QString& name) const
	{
		const auto it = Util::Algorithm::find(m->playlists, [&name](const auto& playlist) {
			return (playlist.name() == name);
		});

		return (it != m->playlists.end()) ? it->id() : -1;
	}

	void Chooser::playlistsChanged()
	{
		m->playlists.clear();
		m->playlists = loadPlaylists(StoreType::OnlyPermanent, SortOrder::NameAsc, false);

		emit sigPlaylistsChanged();
	}

	void Chooser::playlistDeleted(int /* id */)
	{
		playlistsChanged();
	}

	void Chooser::playlistAdded(int /* id */, const QString& /* name */)
	{
		playlistsChanged();
	}

	void Chooser::playlistRenamed(int /* id */, const QString& /* oldName */, const QString& /* newName*/ )
	{
		playlistsChanged();
	}
}