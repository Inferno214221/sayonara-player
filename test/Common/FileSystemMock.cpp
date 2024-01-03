/* FileSystemMock.cpp */
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

#include "FileSystemMock.h"
#include "Utils/FileUtils.h"
#include "Utils/Algorithm.h"

#include <QStringList>
#include <QDir>

namespace
{
	bool filenameMatchesExtension(const QString& filename, QString nameFilter)
	{
		nameFilter = nameFilter.toLower();
		if(nameFilter.startsWith("*."))
		{
			nameFilter.remove(0, 2);
		}

		if(nameFilter.isEmpty())
		{
			return false;
		}

		return filename.toLower().endsWith(nameFilter);
	}

	QStringList filterFiles(const QStringList& files, const QStringList& nameFilters)
	{
		auto result = QStringList {};

		for(const auto& filename: files)
		{
			const auto isMatch = Util::Algorithm::contains(nameFilters, [&filename](const auto nameFilter) {
				return filenameMatchesExtension(filename, nameFilter);
			});

			if(isMatch || nameFilters.isEmpty())
			{
				result << filename;
			}
		}

		return result;
	}

	QStringList filterDirs(const QDir& baseDir, const QStringList& allDirs, const Util::FileSystem* fileSystem)
	{
		auto result = QStringList {};

		for(const auto& dir: allDirs)
		{
			auto maybeUpDir = fileSystem->cd({dir}, "..");
			if(maybeUpDir.has_value())
			{
				const auto x = maybeUpDir.value().absolutePath();
				if(maybeUpDir->absolutePath() == baseDir.absolutePath())
				{
					result << QDir(dir).dirName();
				}
			}
		}

		return result;
	}
}

namespace Test
{
	FileSystemMock::FileSystemMock(const QMap<QString, QStringList>& fileStructure) :
		m_fileStructure {fileStructure}
	{
		createFileStructure(m_fileStructure);
	}

	bool FileSystemMock::isDir(const QString& filename)
	{
		return m_fileStructure.keys().contains(Util::File::cleanFilename(filename));
	}

	bool FileSystemMock::isFile(const QString& filename)
	{
		return flattenFileSystemStructure(m_fileStructure).contains(Util::File::cleanFilename(filename));
	}

	bool FileSystemMock::createDirectories(const QString& path)
	{
		const auto root = QDir::rootPath();
		auto d = path;
		while(d != root)
		{
			if(!m_fileStructure.contains(d))
			{
				m_fileStructure.insert(d, {});
			}

			d = Util::File::getParentDirectory(d);
		}

		if(!m_fileStructure.contains(root))
		{
			m_fileStructure.insert(root, {});
		}

		return true;
	}

	bool FileSystemMock::exists(const QString& filename)
	{
		return isDir(filename) || isFile(filename);
	}

	bool FileSystemMock::writeFile(const QByteArray& content, const QString& filename)
	{
		auto [d, f] = Util::File::splitFilename(Util::File::cleanFilename(filename));
		if(m_fileStructure.contains(d))
		{
			m_fileStructure[d].push_back(f);
			m_fileStructure[d].removeDuplicates();
			m_content[filename] = QString {content};

			return true;
		}

		return false;
	}

	QString FileSystemMock::readFileIntoString(const QString& filename)
	{
		return exists(filename) ? m_content[filename] : QString {};
	}

	void FileSystemMock::createFileStructure(const QMap<QString, QStringList>& dirFilesMap)
	{
		for(const auto& dir: dirFilesMap.keys())
		{
			createDirectories(dir);
		}

		for(const auto& dir: dirFilesMap.keys())
		{
			const auto files = dirFilesMap[dir];
			for(const auto& file: files)
			{
				writeFile({}, QDir {dir}.absoluteFilePath(file));
			}
		}
	}

	bool FileSystemMock::copyFile(const QString& sourceFile, const QString& targetFile)
	{
		if(!isFile(sourceFile))
		{
			return false;
		}

		if(const auto parentDir = Util::File::getParentDirectory(targetFile); createDirectories(parentDir))
		{
			const auto data = readFileIntoString(sourceFile);
			writeFile(data.toLocal8Bit(), Util::File::cleanFilename(targetFile));
			return true;
		}

		return false;
	}

	void FileSystemMock::deleteFiles(const QStringList& files)
	{
		for(auto file: files)
		{
			file = Util::File::cleanFilename(file);
			const auto keys = m_fileStructure.keys();
			for(const auto& key: keys)
			{
				if((key == file) || (key.startsWith(file + "/")))
				{
					m_fileStructure.remove(key);
				}
			}

			const auto [dir, filename] = Util::File::splitFilename(file);
			if(auto values = m_fileStructure[dir]; values.contains(filename))
			{
				values.removeAll(filename);
				m_fileStructure[dir] = values;
			}

			m_content.remove(file);
		}
	}

	QStringList
	FileSystemMock::entryList(const QDir& dir, const QStringList& nameFilters, const QDir::Filters filters) const
	{
		auto filtered = QStringList {};

		if(filters & QDir::Files)
		{
			filtered << filterFiles(m_fileStructure[dir.absolutePath()], nameFilters);
		}

		if(filters & QDir::Dirs)
		{
			filtered << filterDirs(dir, m_fileStructure.keys(), this);
		}

		return filtered;
	}

	std::optional<QDir> FileSystemMock::cd(const QDir& dir, const QString& subDir) const
	{
		const auto success = m_fileStructure.contains(dir.absolutePath()) &&
		                     m_fileStructure.contains(QDir::cleanPath(dir.absoluteFilePath(subDir)));

		return success
		       ? std::optional {QDir::cleanPath(dir.absoluteFilePath(subDir))}
		       : std::nullopt;
	}

	QStringList flattenFileSystemStructure(const QMap<QString, QStringList>& dirFilesMap)
	{
		auto result = QStringList {};

		for(const auto& dir: dirFilesMap.keys())
		{
			result << dir;
			const auto files = dirFilesMap[dir];
			for(const auto& file: files)
			{
				result << Util::File::cleanFilename(QDir {dir}.absoluteFilePath(file));
			}
		}

		result.removeDuplicates();
		result.sort();

		return result;
	}
} // Test
