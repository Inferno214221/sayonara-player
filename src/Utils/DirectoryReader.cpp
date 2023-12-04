/* DirectoryReader.cpp */

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

#include "DirectoryReader.h"

#include "Database/Connector.h"
#include "Database/LibraryDatabase.h"

#include "Utils/Algorithm.h"
#include "Utils/FileUtils.h"
#include "Utils/FileSystem.h"
#include "Utils/Tagging/Tagging.h"
#include "Utils/Parser/PlaylistParser.h"
#include "Utils/MetaData/MetaDataList.h"

#include <QStringList>
#include <QDir>

#include <map>

namespace
{
	using TrackMap = std::map<QString, MetaData>;

	TrackMap createLibraryTrackMap(const QStringList& soundFiles)
	{
		TrackMap result;

		auto* db = DB::Connector::instance();
		auto* libraryDatabase = db->libraryDatabase(-1, 0);

		MetaDataList tracks;
		libraryDatabase->getMultipleTracksByPath(soundFiles, tracks);

		for(auto& track: tracks)
		{
			auto filepath = track.filepath();
			result.emplace(std::move(filepath), std::move(track));
		}

		return result;
	}

	MetaDataList getTrackListWithTrackMap(const QStringList& soundFiles, TrackMap trackMap)
	{
		MetaDataList tracks;
		for(const auto& soundFile: soundFiles)
		{
			const auto contains = (trackMap.find(soundFile) != trackMap.end());
			if(contains)
			{
				tracks << std::move(trackMap.at(soundFile));
			}

			else
			{
				auto track = MetaData {soundFile};
				const auto hasTags = Tagging::Utils::getMetaDataOfFile(track);
				if(!hasTags)
				{
					const auto title = Util::File::getFilenameOfPath(track.filepath());
					track.setTitle(title);
				}

				track.setExtern(true);

				tracks << std::move(track);
			}
		}

		return tracks;
	}

	MetaDataList getTracksFromPlaylistFile(const QString& playlistFile)
	{
		const auto tracks = PlaylistParser::parsePlaylist(playlistFile, false);

		auto paths = QStringList {};
		Util::Algorithm::transform(tracks, paths, [](const auto& track) {
			return track.filepath();
		});

		auto filepathTrackMap = createLibraryTrackMap(paths);
		return getTrackListWithTrackMap(paths, std::move(filepathTrackMap));
	}

	QStringList
	scanFilesInDirectory(const QDir& dir, const Util::FileSystemPtr& fileSystem, const QStringList& nameFilter = {})
	{
		QStringList result;

		const auto fileList = fileSystem->entryList(dir, nameFilter, QDir::Filters(QDir::Files | QDir::NoDotAndDotDot));
		for(const auto& filename: fileList)
		{
			result << dir.absoluteFilePath(filename);
		}

		return result;
	}

	// NOLINTNEXTLINE(misc-no-recursion)
	QStringList
	scanFilesRecursively(const QDir& dir, const Util::FileSystemPtr& fileSystem, const QStringList& nameFilter = {})
	{
		const auto b = dir.absolutePath();
		if(!fileSystem->exists(dir.absolutePath()))
		{
			return {};
		}

		const auto dirNames = fileSystem->entryList(dir, QDir::Dirs | QDir::NoDotAndDotDot);

		QStringList result;
		for(const auto& dirName: dirNames)
		{
			if(!dirName.isEmpty())
			{
				if(const auto maybeSubDir = fileSystem->cd(dir, dirName); maybeSubDir.has_value())
				{
					const auto subDir = maybeSubDir->absolutePath();
					result << scanFilesRecursively(subDir, fileSystem, nameFilter);
				}
			}
		}

		result << scanFilesInDirectory(dir, fileSystem, nameFilter);

		return result;
	}

	MetaDataList scanMetadata(const QStringList& fileList, const Util::FileSystemPtr& fileSystem)
	{
		QStringList soundFiles, playlistFiles;
		for(const auto& filename: fileList)
		{
			if(!fileSystem->exists(filename))
			{
				continue;
			}

			if(fileSystem->isDir(filename))
			{
				const auto files = scanFilesRecursively({filename}, fileSystem);
				for(const auto& file: files)
				{
					if(Util::File::isSoundFile(file))
					{
						soundFiles << Util::File::cleanFilename(file);
					}
				}
			}

			else if(Util::File::isSoundFile(filename))
			{
				soundFiles << Util::File::cleanFilename(filename);
			}

			else if(Util::File::isPlaylistFile(filename))
			{
				playlistFiles << Util::File::cleanFilename(filename);
			}
		}

		auto filepathTrackMap = createLibraryTrackMap(soundFiles);
		auto tracks = getTrackListWithTrackMap(soundFiles, std::move(filepathTrackMap));

		for(const auto& playlistFile: playlistFiles)
		{
			tracks << getTracksFromPlaylistFile(playlistFile);
		}

		return tracks;
	}

	class DirectoryReaderImpl :
		public Util::DirectoryReader
	{
		public:
			DirectoryReaderImpl(const Util::FileSystemPtr& fileSystem) :
				m_fileSystem {fileSystem} {}

			~DirectoryReaderImpl() noexcept override = default;

			QStringList scanFilesInDirectory(const QDir& baseDir, const QStringList& nameFilters) override
			{
				return ::scanFilesInDirectory(baseDir, m_fileSystem, nameFilters);
			}

			QStringList scanFilesRecursively(const QDir& baseDir, const QStringList& nameFilters) override
			{
				return ::scanFilesRecursively(baseDir, m_fileSystem, nameFilters);
			}

			MetaDataList scanMetadata(const QStringList& files) override
			{
				return ::scanMetadata(files, m_fileSystem);
			}

		private:
			Util::FileSystemPtr m_fileSystem;
	};
}

namespace Util
{
	DirectoryReader::DirectoryReader() = default;
	DirectoryReader::~DirectoryReader() noexcept = default;

	std::shared_ptr<DirectoryReader> DirectoryReader::create(const FileSystemPtr& fileSystem)
	{
		return std::make_shared<DirectoryReaderImpl>(fileSystem);
	}
}