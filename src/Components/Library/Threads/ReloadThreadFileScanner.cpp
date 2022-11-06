/* ReloadThreadFileScanner.cpp */
/*
 * Copyright (C) 2011-2022 Michael Lugmair
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

#include "ReloadThreadFileScanner.h"

#include "Utils/FileUtils.h"
#include "Utils/Utils.h"
#include "Utils/Language/Language.h"

#include <QDir>

namespace
{
	QString currentDirString(const QDir& baseDir)
	{
		QString parentDir, pureDirName;
		Util::File::splitFilename(baseDir.absolutePath(), parentDir, pureDirName);

		return Lang::get(Lang::Directory) + ": " + pureDirName;
	}

	QStringList filterValidFiles(const QDir& baseDir, const QStringList& soundFiles)
	{
		QStringList lst;
		for(const auto& filename: soundFiles)
		{
			const auto absolutePath = baseDir.absoluteFilePath(filename);
			const auto info = QFileInfo {absolutePath};
			if(info.exists() && info.isFile())
			{
				lst << absolutePath;
			}
		}

		return lst;
	}

	QStringList getSoundFiles(const QDir& baseDir)
	{
		const auto soundfileExtensions = ::Util::soundfileExtensions();
		const auto soundFiles = baseDir.entryList(soundfileExtensions, QDir::Files);
		auto validSoundFiles = filterValidFiles(baseDir, soundFiles);

		return validSoundFiles;
	}
}

class ReloadThreadFileScannerImpl :
	public Library::ReloadThreadFileScanner
{
	public:
		QStringList
		getFilesRecursive(const QDir& baseDir) override
		{
			emit sigCurrentDirectoryChanged(currentDirString(baseDir));

			const auto subDirs = baseDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
			auto soundFiles = getSoundFiles(baseDir);

			for(const auto& subDir: subDirs)
			{
				if(auto dir = baseDir; dir.cd(subDir))
				{
					soundFiles << getFilesRecursive(dir);
				}
			}

			return soundFiles;
		}

		bool exists(const QString& filename) override
		{
			return Util::File::exists(filename);
		}

		bool checkFile(const QString& filename) override
		{
			return Util::File::checkFile(filename);
		}
};

namespace Library
{
	ReloadThreadFileScanner* ReloadThreadFileScanner::create()
	{
		return new ReloadThreadFileScannerImpl {};
	}
}