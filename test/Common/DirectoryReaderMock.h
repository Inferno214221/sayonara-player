/* Nix.h */
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

#ifndef SAYONARA_PLAYER_NIX_H
#define SAYONARA_PLAYER_NIX_H

#include "Utils/Pimpl.h"

#include "Utils/DirectoryReader.h"
#include "Utils/ArchiveExtractor.h"

#include <QMap>

namespace Test
{
	class FileSystemMock;
	class DirectoryReaderMock :
		public Util::DirectoryReader
	{
		public:
			explicit DirectoryReaderMock(std::shared_ptr<FileSystemMock> fileSystem);

			~DirectoryReaderMock() noexcept override = default;

			QStringList scanFilesInDirectory(const QDir& d, const QStringList& /*f*/) override;

			QStringList scanFilesRecursively(const QDir& d, const QStringList& /*f*/) override;

			MetaDataList scanMetadata(const QStringList& /*files*/) override;

		private:
			std::shared_ptr<FileSystemMock> m_fileSystem;

	};

	class ArchiveExtractorMock :
		public Util::ArchiveExtractor
	{
		public:
			ArchiveExtractorMock(QMap<QString, QStringList> result, QStringList supportedExtensions,
			                     std::shared_ptr<FileSystemMock> fileSystem);

			~ArchiveExtractorMock() noexcept override = default;

			QStringList extractArchive(const QString& filename, const QString& targetDir) override;

			[[nodiscard]] QStringList supportedArchives() const override;

		private:
			QMap<QString, QStringList> m_result;
			QStringList m_supportedExtensions;
			std::shared_ptr<FileSystemMock> m_fileSystem;
	};

} // Test

#endif //SAYONARA_PLAYER_NIX_H
