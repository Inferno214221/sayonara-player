/* FileSystem.cpp */
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

#include "FileSystem.h"
#include "FileUtils.h"

#include <QFile>

namespace
{
	class FileSystemImpl :
		public Util::FileSystem
	{
		public:
			~FileSystemImpl() noexcept override = default;

			bool isDir(const QString& filename) override
			{
				return Util::File::isDir(filename);
			}

			bool isFile(const QString& filename) override
			{
				return Util::File::isFile(filename);
			}

			bool createDirectories(const QString& path) override
			{
				return Util::File::createDirectories(path);
			}

			bool exists(const QString& filename) override
			{
				return Util::File::exists(filename);
			}

			bool writeFile(const QByteArray& data, const QString& filename) override
			{
				Util::File::writeFile(data, filename);
				return true;
			}

			bool copyFile(const QString& sourceFile, const QString& targetFile) override
			{
				const auto targetDir = Util::File::getParentDirectory(targetFile);
				const auto directoriesCreated = createDirectories(targetDir);

				return directoriesCreated
				       ? QFile::copy(sourceFile, targetFile)
				       : false;
			}

			void deleteFiles(const QStringList& files) override
			{
				Util::File::deleteFiles(files);
			}
	};
}

namespace Util
{
	FileSystem::FileSystem() = default;
	FileSystem::~FileSystem() noexcept = default;

	FileSystemPtr FileSystem::create()
	{
		return std::make_shared<FileSystemImpl>();
	}
}