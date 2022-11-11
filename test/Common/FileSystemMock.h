/* FileSystemMock.h */
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

#ifndef SAYONARA_PLAYER_FILESYSTEMMOCK_H
#define SAYONARA_PLAYER_FILESYSTEMMOCK_H

#include "Utils/FileSystem.h"

#include <QMap>

namespace Test
{
	class FileSystemMock :
		public Util::FileSystem
	{
		public:
			explicit FileSystemMock(const QMap<QString, QStringList>& fileStructure);

			~FileSystemMock() noexcept override = default;

			[[nodiscard]] bool
			isDir(const QString& filename) override;

			[[nodiscard]] bool isFile(const QString& filename) override;

			bool createDirectories(const QString& path) override;

			[[nodiscard]] bool exists(const QString& filename) override;

			bool writeFile(const QByteArray& /*data*/, const QString& filename) override;

			[[nodiscard]] QMap<QString, QStringList> allFiles() const { return m_fileStructure; }

			bool copyFile(const QString& sourceFile, const QString& targetFile) override;
			void deleteFiles(const QStringList& files) override;

		private:
			void createFileStructure(const QMap<QString, QStringList>& dirFilesMap);

			QMap<QString, QStringList> m_fileStructure;
	};

	QStringList flattenFileSystemStructure(const QMap<QString, QStringList>& dirFilesMap);
} // Test

#endif //SAYONARA_PLAYER_FILESYSTEMMOCK_H
