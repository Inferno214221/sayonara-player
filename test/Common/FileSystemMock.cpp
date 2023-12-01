/* FileSystemMock.cpp */
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

#include "FileSystemMock.h"
#include "Utils/FileUtils.h"

#include <QStringList>
#include <QDir>

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
		auto [d, f] = Util::File::splitFilename(path);
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
		auto [d, f] = Util::File::splitFilename(filename);
		f = Util::File::cleanFilename(f);
		if(m_fileStructure.contains(d))
		{
			m_fileStructure[d].push_back(f);
		}

		else
		{
			createDirectories(d);
			m_fileStructure.insert(d, {f});
		}

		m_content[filename] = QString {content};

		return true;
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
				writeFile({}, dir + "/" + file);
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

			auto [dir, filename] = Util::File::splitFilename(file);
			dir = Util::File::cleanFilename(dir);
			if(auto values = m_fileStructure[dir]; values.contains(file))
			{
				values.removeAll(file);
				m_fileStructure[filename] = values;
				break;
			}

			m_content.remove(file);
		}
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
				result << Util::File::cleanFilename(dir + "/" + file);
			}
		}

		result.removeDuplicates();
		result.sort();

		return result;
	}
} // Test
