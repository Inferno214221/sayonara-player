/* StandardPaths.cpp */
/*
 * Copyright (C) 2011-2020 Michael Lugmair
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
#include "StandardPaths.h"
#include "FileUtils.h"
#include "Utils.h"
#include "Logger/Logger.h"

#include <QDir>
#include <QStandardPaths>
#include <QString>

namespace
{
	QString operator/(const QString& first, const QString& second)
	{
		if(second.isEmpty())
		{
			return Util::File::cleanFilename(first);
		}

		return Util::File::cleanFilename(first + QDir::separator() + second);
	}

	QString createPath(const QString& path, const QString& appendPath)
	{
		if(!Util::File::exists(path))
		{
			if(!Util::File::createDirectories(path))
			{
				spLog(Log::Warning, "createPath") << "Cannot create path: " << path;
			}
		}

		return path / appendPath;
	}

	bool copyFromLegacyLocation(const QString& filename, const QString& targetDir)
	{
		if(!Util::File::exists(filename))
		{
			return true;
		}

		if(const auto pureFilename = Util::File::getFilenameOfPath(filename);
			Util::File::exists(targetDir / pureFilename))
		{
			return true;
		}

		auto success = true;

		const auto fileInfo = QFileInfo(filename);
		if(fileInfo.isFile())
		{
			auto newName = QString {};
			success = Util::File::moveFile(filename, targetDir, newName);
		}

		else if(fileInfo.isDir())
		{
			auto newName = QString {};
			success = Util::File::moveDir(filename, targetDir, newName);
		}

		if(!success)
		{
			spLog(Log::Warning, "Copy legacy file") << "Could not copy " << filename << " to " << targetDir;
		}

		return success;
	}
}

QString Util::xdgConfigPath(const QString& appendPath)
{
	const auto path = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
	return createPath(path, appendPath);
}

QString Util::xdgSharePath(const QString& appendPath)
{
	const auto path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
	return createPath(path, appendPath);
}

QString Util::xdgCachePath(const QString& appendPath)
{
	const auto path = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
	return createPath(path, appendPath);
}

QString Util::sharePath(const QString& appendPath)
{
#ifdef Q_OS_WIN
	return QCoreApplication::applicationDirPath() + "/share/";
#else
	auto path = Util::File::cleanFilename(getEnvironment("SAYONARA_SHARE_DIR"));
	if(!Util::File::exists(path))
	{
		path = SAYONARA_INSTALL_SHARE_PATH;
	}

	return path / appendPath;
#endif
}

QString Util::tempPath(const QString& appendPath)
{
	const auto path = QDir::temp().absoluteFilePath("sayonara");
	return createPath(path, appendPath);
}

QString Util::similarArtistsPath()
{
	return createPath(xdgCachePath("similar_artists"), QString());
}

QString Util::translationsPath()
{
	return createPath(xdgSharePath("translations"), QString());
}

QString Util::translationsSharePath()
{
	const auto shareDir = sharePath();
	return createPath(shareDir, "translations");
}

QString Util::coverDirectory(const QString& appendFilename)
{
	return createPath(Util::xdgCachePath("covers"), appendFilename);
}

QString Util::coverTempDirectory(const QString& appendFilename)
{
	return createPath(Util::tempPath("covers"), appendFilename);
}

QString Util::legacySayonaraPath(const QString& appendFilename)
{
	const auto sayonaraPath = createPath(QDir::homePath(), ".Sayonara");
	return sayonaraPath / appendFilename;
}

QString Util::lyricsPath(const QString& appendFilename)
{
	return createPath(Util::xdgCachePath("lyrics"), appendFilename);
}

void Util::copyFromLegacyLocations()
{
	const auto legacyPath = legacySayonaraPath();

	bool success = true;
	success &= copyFromLegacyLocation(legacySayonaraPath("player.db"), xdgConfigPath());
	success &= copyFromLegacyLocation(legacySayonaraPath("soundcloud.db"), xdgConfigPath());
	success &= copyFromLegacyLocation(legacySayonaraPath("somafm.ini"), xdgConfigPath());

	success &= copyFromLegacyLocation(legacySayonaraPath("standard.css"), xdgConfigPath());
	success &= copyFromLegacyLocation(legacySayonaraPath("dark.css"), xdgConfigPath());

	success &= copyFromLegacyLocation(legacySayonaraPath("translations"), xdgSharePath());
	success &= copyFromLegacyLocation(legacySayonaraPath("covers"), xdgCachePath());
	success &= copyFromLegacyLocation(legacySayonaraPath("similar_artists"), xdgCachePath());

	if(success)
	{
		Util::File::deleteFiles({legacyPath});
	}
}
