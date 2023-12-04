/* FileSystem.h */
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

#ifndef SAYONARA_PLAYER_FILESYSTEM_H
#define SAYONARA_PLAYER_FILESYSTEM_H

#include <QDir>
#include <QString>
#include <QStringList>

#include <optional>
#include <memory>

namespace Util
{
	class FileSystem
	{
		public:
			FileSystem();
			virtual ~FileSystem() noexcept;

			FileSystem(const FileSystem& other) = delete;
			FileSystem(FileSystem&& other) = delete;
			FileSystem& operator=(const FileSystem& other) = delete;
			FileSystem& operator=(FileSystem&& other) = delete;

			virtual bool isDir(const QString& filename) = 0;
			virtual bool isFile(const QString& filename) = 0;
			virtual bool createDirectories(const QString& path) = 0;
			virtual bool exists(const QString& filename) = 0;
			virtual bool writeFile(const QByteArray& data, const QString& filename) = 0;
			virtual QString readFileIntoString(const QString& filename) = 0;
			virtual bool copyFile(const QString& sourceFile, const QString& targetFile) = 0;
			virtual void deleteFiles(const QStringList& files) = 0;
			[[nodiscard]] virtual QStringList
			entryList(const QDir& dir, const QStringList& nameFilters, QDir::Filters filters) const = 0;
			[[nodiscard]] virtual QStringList entryList(const QDir& dir, QDir::Filters filters) const final;

			[[nodiscard]] virtual std::optional<QDir> cd(const QDir& dir, const QString& subDir) const = 0;

			static std::shared_ptr<FileSystem> create();
	};

	using FileSystemPtr = std::shared_ptr<FileSystem>;
}
#endif //SAYONARA_PLAYER_FILESYSTEM_H
