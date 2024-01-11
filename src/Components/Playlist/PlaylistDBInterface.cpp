/* PlaylistDBInterface.cpp */

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

#include "PlaylistDBInterface.h"
#include "PlaylistChangeNotifier.h"
#include "Playlist.h"

#include "Database/Connector.h"
#include "Database/Playlist.h"

#include "Utils/Algorithm.h"
#include "Utils/Language/Language.h"
#include "Utils/Playlist/CustomPlaylist.h"
#include "Utils/MetaData/MetaDataList.h"

#include "Utils/Logger/Logger.h"

using Util::SaveAsAnswer;

namespace Playlist
{
	namespace
	{
		constexpr const auto ClassName = "PlaylistDbInterface";

		bool playlistExists(const QString& name, DB::Playlist* playlistConnector)
		{
			const auto playlistId = playlistConnector->getPlaylistIdByName(name);
			return (playlistId >= 0);
		}

		SaveAsAnswer isNameAllowedForPlaylist(int id, const QString& name, DB::Playlist* playlistConnector)
		{
			if(name.isEmpty())
			{
				return SaveAsAnswer::InvalidName;
			}

			const auto playlistId = playlistConnector->getPlaylistIdByName(name);
			return ((playlistId < 0) || (playlistId == id))
			       ? SaveAsAnswer::Success
			       : SaveAsAnswer::NameAlreadyThere;
		}

		bool updatePlaylistTracks(Playlist::DBInterface* playlist, DB::Playlist* playlistConnector)
		{
			const auto success = playlistConnector->updatePlaylistTracks(playlist->id(), playlist->tracks());
			if(!success)
			{
				spLog(Log::Warning, ClassName) << "Cannot update tracks: " << playlist->id() << ", "
				                               << playlist->name();
			}

			playlist->setChanged(!success);
			return success;
		}

		bool updatePlaylist(const QString& name, bool isTemporary, bool isLocked, Playlist::DBInterface* playlist,
		                    DB::Playlist* playlistConnector)
		{
			auto success = playlistConnector->updatePlaylist(playlist->id(), name, isTemporary, isLocked);
			if(!success)
			{
				spLog(Log::Warning, ClassName) << "Cannot update playlist " << playlist->id() << ": "
				                               << playlist->name();
				return false;
			}

			playlist->setName(name);
			playlist->setTemporary(isTemporary);
			playlist->setLocked(isLocked);

			return true;
		}

		bool createPlaylist(const QString& name, bool isTemporary, const bool isLocked, Playlist::DBInterface* playlist,
		                    DB::Playlist* playlistConnector)
		{
			const auto playlistId = playlistConnector->createPlaylist(name, isTemporary, isLocked);
			if(playlistId < 0)
			{
				spLog(Log::Warning, ClassName) << "Cannot insert new playlist " << playlist->name();
				return false;
			}

			playlist->setId(playlistId);
			playlist->setName(name);
			playlist->setTemporary(isTemporary);
			playlist->setLocked(isLocked);
			playlist->setChanged(false);

			return updatePlaylistTracks(playlist, playlistConnector);
		}
	}

	struct DBInterface::Private
	{
		ChangeNotifier* playlistChangeNotifier {ChangeNotifier::instance()};
		DB::Playlist* playlistConnector {DB::Connector::instance()->playlistConnector()};

		QString name;
		int id;
		bool isTemporary {true};
		bool isLocked {false};

		explicit Private(const QString& name) :
			name {name},
			id {playlistConnector->getPlaylistIdByName(name)} {}
	};

	DBInterface::DBInterface(const QString& name)
	{
		m = Pimpl::make<Private>(name);
	}

	DBInterface::~DBInterface() = default;

	SaveAsAnswer DBInterface::save()
	{
		const auto answer = isNameAllowedForPlaylist(id(), name(), m->playlistConnector);
		if(answer != SaveAsAnswer::Success)
		{
			return answer;
		}

		const auto success = (id() < 0)
		                     ? createPlaylist(name(), isTemporary(), isLocked(), this, m->playlistConnector)
		                     : updatePlaylistTracks(this, m->playlistConnector);

		return (success)
		       ? SaveAsAnswer::Success
		       : SaveAsAnswer::OtherError;
	}

	Util::SaveAsAnswer DBInterface::saveAs(const QString& newName)
	{
		const auto answer = isNameAllowedForPlaylist(id(), newName, m->playlistConnector);
		if(answer != SaveAsAnswer::Success)
		{
			return answer;
		}

		const auto renamePermanentPlaylist = !isTemporary() && (name() != newName);
		const auto newPlaylistNeeded = (id() < 0) || renamePermanentPlaylist;
		const auto success = (newPlaylistNeeded)
		                     ? createPlaylist(newName, false, isLocked(), this, m->playlistConnector)
		                     : updatePlaylist(newName, false, isLocked(), this, m->playlistConnector);

		if(success)
		{
			ChangeNotifier::instance()->addPlaylist(id(), name());
			updatePlaylistTracks(this, m->playlistConnector);
		}

		return success
		       ? SaveAsAnswer::Success
		       : SaveAsAnswer::OtherError;
	}

	SaveAsAnswer DBInterface::rename(const QString& newName)
	{
		const auto answer = isNameAllowedForPlaylist(id(), newName, m->playlistConnector);
		if(answer != SaveAsAnswer::Success)
		{
			return answer;
		}

		const auto oldName = name();
		const auto success = updatePlaylist(newName, isTemporary(), isLocked(), this, m->playlistConnector);
		if(success)
		{
			m->playlistChangeNotifier->renamePlaylist(id(), oldName, name());
		}

		return success
		       ? SaveAsAnswer::Success
		       : SaveAsAnswer::OtherError;
	}

	bool DBInterface::lock()
	{
		return updatePlaylist(name(), isTemporary(), true, this, m->playlistConnector);
	}

	bool DBInterface::unlock()
	{
		return updatePlaylist(name(), isTemporary(), false, this, m->playlistConnector);
	}

	bool DBInterface::deletePlaylist()
	{
		const auto playlistId = (id() >= 0)
		                        ? id()
		                        : m->playlistConnector->getPlaylistIdByName(name());

		const auto success = m->playlistConnector->deletePlaylist(playlistId);
		if(success)
		{
			m->playlistChangeNotifier->deletePlaylist(id());
			setId(-1);
			setChanged(false);
			setTemporary(true);
		}

		return success;
	}

	int DBInterface::id() const { return m->id; }

	void DBInterface::setId(int id) { m->id = id; }

	QString DBInterface::name() const { return m->name; }

	void DBInterface::setName(const QString& name) { m->name = name; }

	bool DBInterface::isTemporary() const { return m->isTemporary; }

	void DBInterface::setTemporary(bool b) { m->isTemporary = b; }

	bool DBInterface::isLocked() const { return m->isLocked; }

	void DBInterface::setLocked(const bool b) { m->isLocked = b; }

	QString requestNewDatabaseName(QString prefix)
	{
		auto* db = DB::Connector::instance()->playlistConnector();
		if(prefix.isEmpty())
		{
			prefix = Lang::get(Lang::New);
		}

		for(auto idx = 1; idx < 1000; idx++)
		{
			auto name = QString("%1 %2")
				.arg(prefix)
				.arg(idx);

			if(!playlistExists(name, db))
			{
				return name;
			}
		}

		return QString();
	}

	void reloadFromDatabase(Playlist& playlist)
	{
		if(!playlist.isBusy())
		{
			playlist.modifyTracks([&](const auto /* tracks */) {
				auto* playlistConnector = DB::Connector::instance()->playlistConnector();
				const auto p = playlistConnector->getPlaylistById(playlist.id(), true);
				return p.tracks();
			});

			playlist.resetChangedStatus();
		}
	}
}

