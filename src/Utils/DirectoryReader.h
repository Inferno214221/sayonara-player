/* DirectoryReader.h */

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

#ifndef SAYONARA_DIRECTORY_READER
#define SAYONARA_DIRECTORY_READER

#include <QStringList>

#include <memory>

class QDir;
class MetaDataList;
namespace Util
{
	class FileSystem;
	class DirectoryReader
	{
		public:
			DirectoryReader();
			virtual ~DirectoryReader() noexcept;

			[[nodiscard]] virtual QStringList
			scanFilesInDirectory(const QDir& baseDir, const QStringList& nameFilters = QStringList()) = 0;

			[[nodiscard]] virtual QStringList
			scanFilesRecursively(const QDir& baseDirOrig, const QStringList& nameFilters = QStringList()) = 0;

			[[nodiscard]] virtual MetaDataList scanMetadata(const QStringList& fileList) = 0;

			static std::shared_ptr<DirectoryReader> create(const std::shared_ptr<FileSystem>& fileSystem);
	};

	using DirectoryReaderPtr = std::shared_ptr<DirectoryReader>;
}

#endif // SAYONARA_DIRECTORY_READER
