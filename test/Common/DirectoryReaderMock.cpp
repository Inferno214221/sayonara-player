/* Nix.cpp */
/*
 * Copyright (C) 2011-2024 Michael Lugmair
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

#include "DirectoryReaderMock.h"
#include "FileSystemMock.h"

#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Algorithm.h"
#include "Utils/FileUtils.h"

#include <QDir>
#include <utility>

namespace Test
{
	DirectoryReaderMock::DirectoryReaderMock(std::shared_ptr<FileSystemMock> fileSystem) :
		m_fileSystem {std::move(fileSystem)} {}

	QStringList DirectoryReaderMock::scanFilesInDirectory(const QDir& d, const QStringList&)
	{
		const auto allFiles = m_fileSystem->allFiles();
		return allFiles[d.absolutePath()];
	}

	QStringList DirectoryReaderMock::scanFilesRecursively(const QDir& d, const QStringList&)
	{
		const auto allFiles = m_fileSystem->allFiles();
		auto result = QStringList();
		for(const auto& dir: allFiles.keys())
		{
			if(dir.contains(d.absolutePath()))
			{
				auto files = QStringList {};
				Util::Algorithm::transform(allFiles[dir], files, [&](const auto& file) {
					return dir + "/" + file;
				});

				result << files;
			}
		}

		result.removeDuplicates();

		return result;
	}

	MetaDataList DirectoryReaderMock::scanMetadata(const QStringList&) { return {}; }

	ArchiveExtractorMock::ArchiveExtractorMock(QMap<QString, QStringList> result, QStringList supportedExtensions,
	                                           std::shared_ptr<FileSystemMock> fileSystem) :
		m_result(std::move(result)),
		m_supportedExtensions(std::move(supportedExtensions)),
		m_fileSystem {std::move(fileSystem)} {}

	QStringList ArchiveExtractorMock::extractArchive(const QString& filename, const QString& targetDir)
	{
		const auto [dir, f] = Util::File::splitFilename(filename);
		auto files = QStringList {};

		for(const auto& file: m_result[f])
		{
			files << targetDir + "/" + file;
			m_fileSystem->writeFile({}, targetDir + "/" + file);
		}

		return files;
	}

	QStringList ArchiveExtractorMock::supportedArchives() const
	{
		return m_supportedExtensions;
	}
} // Test
