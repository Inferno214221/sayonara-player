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

#include "Utils/FileUtils.h"
#include "Utils/Utils.h"
#include "Utils/Algorithm.h"
#include "Utils/Tagging/Tagging.h"
#include "Utils/Parser/PlaylistParser.h"
#include "Utils/MetaData/MetaDataList.h"

#include <QDirIterator>
#include <QStringList>
#include <QDir>

#include <unordered_map>

namespace Algorithm = Util::Algorithm;

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

		for(auto& track : tracks)
		{
			const auto filepath = track.filepath();
			auto pair = std::make_pair(filepath, std::move(track));
			result.insert(std::move(pair));
		}

		return result;
	}

	MetaDataList getTrackListWithTrackMap(const QStringList& soundFiles, const TrackMap& trackMap)
	{
		MetaDataList tracks;
		for(const auto& soundFile : soundFiles)
		{
			MetaData track;
			const auto contains = (trackMap.find(soundFile) != trackMap.end());

			if(contains)
			{
				tracks << trackMap.at(soundFile);
			}

			else
			{
				MetaData track {soundFile};
				if(!Tagging::Utils::getMetaDataOfFile(track))
				{
					track.setTitle(track.filepath());
				}

				track.setExtern(true);

				tracks << std::move(track);
			}
		}

		return tracks;
	}
}

struct DirectoryReader::Private
{
	QStringList nameFilters;

	Private(const QStringList& filter) :
		nameFilters(filter) {}
};

DirectoryReader::DirectoryReader(const QStringList& filter)
{
	m = Pimpl::make<Private>(filter);
}

DirectoryReader::DirectoryReader() :
	DirectoryReader(QStringList()) {}

DirectoryReader::~DirectoryReader() = default;

void DirectoryReader::setFilter(const QStringList& filter)
{
	m->nameFilters = filter;
}

void DirectoryReader::setFilter(const QString& filter)
{
	m->nameFilters.clear();
	m->nameFilters << filter;
}

void DirectoryReader::scanFiles(const QDir& baseDir, QStringList& files) const
{
	auto fileList = baseDir.entryList
		(
			m->nameFilters,
			QDir::Filters(QDir::Files | QDir::NoDotAndDotDot)
		);

	for(const QString& filename : fileList)
	{
		files << baseDir.absoluteFilePath(filename);
	}
}


MetaDataList DirectoryReader::scanMetadata(const QStringList& fileList)
{
	// fetch sound and playlist files
	const auto filter = QStringList()
		<< Util::soundfileExtensions()
		<< Util::playlistExtensions();

	setFilter(filter);

	QStringList soundFiles, playlistFiles;
	for(const auto& filename : fileList)
	{
		if(!Util::File::exists(filename))
		{
			continue;
		}

		if(Util::File::isDir(filename))
		{
			QDir dir(filename);
			dir.cd(filename);

			QStringList files;
			scanFilesRecursive(dir, files);
			for(const auto& file : Algorithm::AsConst(files))
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
	auto tracks = getTrackListWithTrackMap(soundFiles, filepathTrackMap);

	for(const auto& playlistFile : playlistFiles)
	{
		tracks << PlaylistParser::parsePlaylist(playlistFile);
	}

	return tracks;
}

void DirectoryReader::scanFilesRecursive(const QDir& originalDirectory, QStringList& files) const
{
	auto dir = originalDirectory;
	const auto dirNames = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
	const auto fileList = dir.entryList(m->nameFilters, QDir::Files);

	for(const auto& dirName : dirNames)
	{
		if(!dirName.isEmpty() && dir.cd(dirName))
		{
			scanFilesRecursive(dir, files);
			dir.cdUp();
		}
	}

	Util::Algorithm::transform(fileList, files, [&dir](const auto& filename) {
		return dir.absoluteFilePath(filename);
	});
}
