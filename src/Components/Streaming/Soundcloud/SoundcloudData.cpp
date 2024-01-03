/* SoundcloudData.cpp */

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

#include "SoundcloudData.h"
#include "Utils/Logger/Logger.h"
#include "Utils/StandardPaths.h"

#include "Database/Query.h"
#include "Database/Fixes.h"

namespace
{
	constexpr const auto SoundcloudDatabaseId = 25;
	constexpr const auto DatabaseSourceDir = ":/Database";
	constexpr const auto DatabaseFilename = "soundcloud.db";

	QString soundcloudConnectionName()
	{
		return Util::xdgConfigPath(DatabaseFilename);
	}

	QString loadSetting(DB::Module& module, const QString& key)
	{
		auto query = module.runQuery(
			"SELECT value FROM Settings WHERE key=:key;",
			{{":key", key}},
			QString("Cannot load setting %1").arg(key));

		if(DB::hasError(query))
		{
			return {};
		}

		if(query.next())
		{
			return query.value(0).toString();
		}

		return {};
	}

	bool insertSetting(DB::Module& module, const QString& key, const QString& value)
	{
		const auto query = module.insert(
			"settings",
			{
				{"key",   key},
				{"value", value}
			},
			QString("Cannot insert setting %1").arg(key));

		return !DB::hasError(query);
	}

	bool saveSetting(DB::Module& module, const QString& key, const QString& value)
	{
		const auto v = loadSetting(module, key);
		if(v.isNull())
		{
			return insertSetting(module, key, value);
		}

		const auto query = module.update(
			"settings",
			{
				{"key",   key},
				{"value", value}
			},
			{"key", key},
			QString("Cannot apply setting %1").arg(key));

		return !DB::hasError(query);
	}

	class SoundcloudDatabaseFixes :
		public DB::Fixes
	{
		public:
			explicit SoundcloudDatabaseFixes(const QString& connectionName) :
				DB::Fixes(connectionName, SoundcloudDatabaseId),
				m_connectionName {connectionName} {}

			~SoundcloudDatabaseFixes() noexcept override = default;

			void applyFixes() override
			{
				auto module = DB::Module(m_connectionName, SoundcloudDatabaseId);

				constexpr const auto creationString =
					"CREATE TABLE Settings "
					"( "
					"  key VARCHAR(100) PRIMARY KEY, "
					"  value TEXT "
					");";

				const auto settingsCreated = checkAndCreateTable("Settings", creationString);
				if(!settingsCreated)
				{
					spLog(Log::Error, this) << "Cannot create settings table for soundcloud";
					return;
				}

				int version;
				const auto versionString = loadSetting(module, "version");
				if(versionString.isEmpty())
				{
					saveSetting(module, "version", "1");
					version = 1;
				}

				else
				{
					version = versionString.toInt();
				}

				if(version < 2)
				{
					const auto success = checkAndInsertColumn("tracks", "albumArtistID", "integer", "-1");
					if(success)
					{
						saveSetting(module, "version", "2");
					}
				}

				if(version < 3)
				{
					const auto success = checkAndInsertColumn("tracks", "libraryID", "integer", "0");
					if(success)
					{
						saveSetting(module, "version", "3");
					}
				}

				if(version < 4)
				{
					const auto success = checkAndInsertColumn("tracks", "fileCissearch", "varchar(256)", "");
					if(success)
					{
						saveSetting(module, "version", "4");
					}
				}

				if(version < 5)
				{
					const auto success = checkAndInsertColumn("tracks", "genreCissearch", "varchar(512)", "");
					if(success)
					{
						saveSetting(module, "version", "5");
					}
				}

				if(version < 6)
				{
					auto query = QSqlQuery(module.db());
					constexpr const auto* queryText =
						"UPDATE tracks "
						"SET filename=substr(filename, 0, instr(filename, '?client')) "
						"WHERE instr(filename, '?client') > 0;";

					query.prepare(queryText);
					if(query.exec())
					{
						saveSetting(module, "version", "6");
					}
				}
			}

		private:
			QString m_connectionName;
	};
}

SC::Database::Database() :
	::DB::Base(SoundcloudDatabaseId, DatabaseSourceDir, Util::xdgConfigPath(), DatabaseFilename,
	           new SoundcloudDatabaseFixes(soundcloudConnectionName()), nullptr) {}

SC::Database::~Database()
{
	closeDatabase();
}
