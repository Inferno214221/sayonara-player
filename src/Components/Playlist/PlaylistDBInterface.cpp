/* PlaylistDBInterface.cpp */

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

#include "PlaylistDBWrapper.h"
#include "PlaylistDBInterface.h"
#include "PlaylistChangeNotifier.h"

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

		bool playlistExists(const QString& name)
		{
			const auto playlist = DBWrapper::getPlaylistByName(name, false);
			return (playlist.id() >= 0);
		}

		SaveAsAnswer isNameAllowedForPlaylist(int id, const QString& name)
		{
			if(name.isEmpty())
			{
				return SaveAsAnswer::InvalidName;
			}

			const auto playlist = DBWrapper::getPlaylistByName(name, false);
			return ((playlist.id() < 0) || (playlist.id() == id))
			       ? SaveAsAnswer::Success
			       : SaveAsAnswer::NameAlreadyThere;
		}

		bool updatePlaylistTracks(Playlist::DBInterface* playlist)
		{
			const auto success = DBWrapper::updatePlaylistTracks(playlist->id(), playlist->tracks());
			if(!success)
			{
				spLog(Log::Warning, ClassName) << "Cannot update tracks: " << playlist->id() << ", "
				                               << playlist->name();
			}

			playlist->setChanged(!success);
			return success;
		}

		bool updatePlaylist(const QString& name, bool isTemporary, Playlist::DBInterface* playlist)
		{
			auto success = DBWrapper::updatePlaylist(playlist->id(), name, isTemporary);
			if(!success)
			{
				spLog(Log::Warning, ClassName) << "Cannot update playlist " << playlist->id() << ": "
				                               << playlist->name();
				return false;
			}

			playlist->setName(name);
			playlist->setTemporary(isTemporary);

			return true;
		}

		bool createPlaylist(const QString& name, bool isTemporary, Playlist::DBInterface* playlist)
		{
			const auto playlistId = DBWrapper::createPlaylist(name, isTemporary);
			if(playlistId < 0)
			{
				spLog(Log::Warning, ClassName) << "Cannot insert new playlist " << playlist->name();
				return false;
			}

			playlist->setId(playlistId);
			playlist->setName(name);
			playlist->setTemporary(isTemporary);
			playlist->setChanged(false);

			return updatePlaylistTracks(playlist);
		}
	}

	struct DBInterface::Private
	{
		PlaylistChangeNotifier* playlistChangeNotifier {PlaylistChangeNotifier::instance()};

		QString name;
		int id;
		bool isTemporary {true};

		Private(const QString& name) :
			name {name},
			id {DBWrapper::getPlaylistByName(name, false).id()} {}
	};

	DBInterface::DBInterface(const QString& name)
	{
		m = Pimpl::make<Private>(name);
	}

	DBInterface::~DBInterface() = default;

	SaveAsAnswer DBInterface::save()
	{
		if(const auto answer = isNameAllowedForPlaylist(id(), name()); answer != SaveAsAnswer::Success)
		{
			return answer;
		}

		const auto success = (id() < 0)
		                     ? createPlaylist(name(), isTemporary(), this)
		                     : updatePlaylistTracks(this);

		return (success)
		       ? SaveAsAnswer::Success
		       : SaveAsAnswer::OtherError;
	}

	Util::SaveAsAnswer DBInterface::saveAs(const QString& newName)
	{
		if(const auto answer = isNameAllowedForPlaylist(id(), newName); (answer != SaveAsAnswer::Success))
		{
			return answer;
		}

		const auto renamePermanentPlaylist = !isTemporary() && (name() != newName);
		const auto newPlaylistNeeded = (id() < 0) || renamePermanentPlaylist;
		const auto success = (newPlaylistNeeded)
			? createPlaylist(newName, false, this)
			: updatePlaylist(newName, false, this);

		if(success)
		{
			PlaylistChangeNotifier::instance()->addPlaylist(id(), name());
			updatePlaylistTracks(this);
		}

		return success
		       ? SaveAsAnswer::Success
		       : SaveAsAnswer::OtherError;
	}

	SaveAsAnswer DBInterface::rename(const QString& newName)
	{
		if(const auto answer = isNameAllowedForPlaylist(id(), newName); answer != SaveAsAnswer::Success)
		{
			return answer;
		}

		const auto oldName = name();
		const auto success = updatePlaylist(newName, isTemporary(), this);
		if(success)
		{
			m->playlistChangeNotifier->renamePlaylist(id(), oldName, name());
		}

		return success
		       ? SaveAsAnswer::Success
		       : SaveAsAnswer::OtherError;
	}

	bool DBInterface::deletePlaylist()
	{
		const auto success = (id() >= 0)
		                     ? DBWrapper::deletePlaylist(id())
		                     : DBWrapper::deletePlaylist(name());

		if(success)
		{
			m->playlistChangeNotifier->deletePlaylist(id());
			setId(-1);
			setChanged(false);
			setTemporary(true);
		}

		return success;
	}

	MetaDataList DBInterface::fetchTracksFromDatabase() const
	{
		const auto playlist = DBWrapper::getPlaylistById(id(), true);
		return playlist.tracks();
	}

	int DBInterface::id() const { return m->id; }

	void DBInterface::setId(int id) { m->id = id; }

	QString DBInterface::name() const { return m->name; }

	void DBInterface::setName(const QString& name) { m->name = name; }

	bool DBInterface::isTemporary() const { return m->isTemporary; }

	void DBInterface::setTemporary(bool b) { m->isTemporary = b; }

	QString requestNewDatabaseName(QString prefix)
	{
		if(prefix.isEmpty())
		{
			prefix = Lang::get(Lang::New);
		}

		for(auto idx = 1; idx < 1000; idx++)
		{
			const auto name = QString("%1 %2")
				.arg(prefix)
				.arg(idx);

			if(!playlistExists(name))
			{
				return name;
			}
		}

		return QString();
	}
}