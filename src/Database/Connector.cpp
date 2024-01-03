/* DatabaseConnector.cpp */

/* Copyright (C) 2011-2024 Michael Lugmair (Lucio Carreras)
 *
 * This file is part of sayonara player
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General License for more details.

 * You should have received a copy of the GNU General License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "Database/StandardConnectorFixes.h"
#include "Database/Connector.h"
#include "Database/Query.h"
#include "Database/LibraryDatabase.h"
#include "Database/Bookmarks.h"
#include "Database/Equalizer.h"
#include "Database/Playlist.h"
#include "Database/Podcasts.h"
#include "Database/Streams.h"
#include "Database/Session.h"
#include "Database/Settings.h"
#include "Database/Shortcuts.h"
#include "Database/VisualStyles.h"
#include "Database/CoverConnector.h"
#include "Database/SmartPlaylists.h"
#include "Database/Fixes.h"

#include "Utils/Logger/Logger.h"
#include "Utils/Utils.h"
#include "Utils/StandardPaths.h"
#include "Utils/Algorithm.h"

#include <QFileInfo>
#include <QDateTime>
#include <QTime>

#include <tuple>
#include <algorithm>

namespace DB
{
	namespace
	{
		constexpr const auto ConnectorDatabaseId = 0;

		class DatabaseNotCreatedException :
			public std::exception
		{
			public:
				[[nodiscard]] const char* what() const noexcept override;
		};
	}

	struct Connector::Private
	{
		std::unique_ptr<Bookmarks> bookmarkConnector {nullptr};
		std::unique_ptr<Equalizer> equalizerConnector {nullptr};
		std::unique_ptr<Playlist> playlistConnector {nullptr};
		std::unique_ptr<Podcasts> podcastConnector {nullptr};
		std::unique_ptr<Streams> streamConnector {nullptr};
		std::unique_ptr<VisualStyles> visualStyleConnector {nullptr};
		std::unique_ptr<Session> sessionConnector {nullptr};
		std::unique_ptr<Settings> settingsConnector {nullptr};
		std::unique_ptr<Shortcuts> shortcutConnector {nullptr};
		std::unique_ptr<Covers> coverConnector {nullptr};
		std::unique_ptr<Library> libraryConnector {nullptr};
		std::unique_ptr<SmartPlaylists> smartPlaylistConnector {nullptr};

		QList<LibraryDatabase*> libraryDbs;
		LibraryDatabase* genericLibraryDatabase {nullptr};
	};

	Connector::Connector(const QString& sourceDirectory, const QString& targetDirectory,
	                     const QString& databaseFilename) :
		Base(ConnectorDatabaseId,
		     sourceDirectory,
		     targetDirectory,
		     databaseFilename,
		     new StandardConnectorFixes(createConnectionName(targetDirectory, databaseFilename), ConnectorDatabaseId),
		     nullptr)
	{
		m = Pimpl::make<Private>();

		if(!isInitialized())
		{
			throw DatabaseNotCreatedException();
		}

		m->genericLibraryDatabase = new LibraryDatabase(connectionName(), databaseId(), -1);
		m->libraryDbs << m->genericLibraryDatabase;
	}

	Connector::~Connector() = default;

	Connector* Connector::instance()
	{
		return customInstance({}, {}, {});
	}

	Connector* Connector::customInstance(QString sourceDirectory, QString targetDirectory, QString databseFilename)
	{
		if(sourceDirectory.isEmpty())
		{
			sourceDirectory = ":/Database";
		}

		if(targetDirectory.isEmpty())
		{
			targetDirectory = Util::xdgConfigPath();
		}

		if(databseFilename.isEmpty())
		{
			databseFilename = "player.db";
		}

		static auto connector = Connector(sourceDirectory, targetDirectory, databseFilename);
		return &connector;
	}

	LibraryDatabases Connector::libraryDatabases() const
	{
		return m->libraryDbs;
	}

	LibraryDatabase* Connector::libraryDatabase(LibraryId libraryId, DbId databaseId)
	{
		const auto it = Util::Algorithm::find(m->libraryDbs, [&](LibraryDatabase* db) {
			return (db->libraryId() == libraryId && db->databaseId() == databaseId);
		});

		if(it == m->libraryDbs.end())
		{
			spLog(Log::Warning, this) << "Could not find Library:"
			                          << " DB ID = " << static_cast<int>(databaseId)
			                          << " LibraryID = " << static_cast<int>(libraryId);

			return m->genericLibraryDatabase;
		}

		return *it;
	}

	LibraryDatabase* Connector::registerLibraryDatabase(LibraryId libraryId)
	{
		const auto it = Util::Algorithm::find(m->libraryDbs, [=](auto* db) {
			return (db->libraryId() == libraryId);
		});

		if(it == m->libraryDbs.end())
		{
			auto* libraryDatabase = new LibraryDatabase(connectionName(), databaseId(), libraryId);
			m->libraryDbs << libraryDatabase;
			return libraryDatabase;
		}

		return *it;
	}

	void Connector::deleteLibraryDatabase(LibraryId libraryId)
	{
		const auto it = Util::Algorithm::find(m->libraryDbs, [=](auto* db) {
			return (db->libraryId() == libraryId);
		});

		if(it != m->libraryDbs.end())
		{
			auto* libraryDatabase = *it;
			libraryDatabase->deleteAllTracks(true);
			m->libraryDbs.removeAll(libraryDatabase);

			delete libraryDatabase;
		}
	}

	Playlist* Connector::playlistConnector()
	{
		if(!m->playlistConnector)
		{
			m->playlistConnector = std::make_unique<Playlist>(connectionName(), databaseId());
		}

		return m->playlistConnector.get();
	}

	Bookmarks* Connector::bookmarkConnector()
	{
		if(!m->bookmarkConnector)
		{
			m->bookmarkConnector = std::make_unique<Bookmarks>(connectionName(), databaseId());
		}

		return m->bookmarkConnector.get();
	}

	Streams* Connector::streamConnector()
	{
		if(!m->streamConnector)
		{
			m->streamConnector = std::make_unique<Streams>(connectionName(), databaseId());
		}

		return m->streamConnector.get();
	}

	Podcasts* Connector::podcastConnector()
	{
		if(!m->podcastConnector)
		{
			m->podcastConnector = std::make_unique<Podcasts>(connectionName(), databaseId());
		}

		return m->podcastConnector.get();
	}

	VisualStyles* Connector::visualStyleConnector()
	{
		if(!m->visualStyleConnector)
		{
			m->visualStyleConnector = std::make_unique<VisualStyles>(connectionName(), databaseId());
		}

		return m->visualStyleConnector.get();
	}

	Settings* Connector::settingsConnector()
	{
		if(!m->settingsConnector)
		{
			m->settingsConnector = std::make_unique<Settings>(connectionName(), databaseId());
		}

		return m->settingsConnector.get();
	}

	Shortcuts* Connector::shortcutConnector()
	{
		if(!m->shortcutConnector)
		{
			m->shortcutConnector = std::make_unique<Shortcuts>(connectionName(), databaseId());
		}

		return m->shortcutConnector.get();
	}

	Library* Connector::libraryConnector()
	{
		if(!m->libraryConnector)
		{
			m->libraryConnector = std::make_unique<Library>(connectionName(), databaseId());
		}

		return m->libraryConnector.get();
	}

	Covers* Connector::coverConnector()
	{
		if(!m->coverConnector)
		{
			m->coverConnector = std::make_unique<Covers>(connectionName(), databaseId());
		}

		return m->coverConnector.get();
	}

	Session* Connector::sessionConnector()
	{
		if(!m->sessionConnector)
		{
			m->sessionConnector = std::make_unique<Session>(connectionName(), databaseId());
		}

		return m->sessionConnector.get();
	}

	Equalizer* Connector::equalizerConnector()
	{
		if(!m->equalizerConnector)
		{
			m->equalizerConnector = std::make_unique<Equalizer>(connectionName(), databaseId());
		}

		return m->equalizerConnector.get();
	}

	SmartPlaylists* Connector::smartPlaylistsConnector()
	{
		if(!m->smartPlaylistConnector)
		{
			m->smartPlaylistConnector = std::make_unique<SmartPlaylists>(connectionName(), databaseId());
		}

		return m->smartPlaylistConnector.get();
	}

	const char* DatabaseNotCreatedException::what() const noexcept
	{
		return "Database could not be created";
	}
}