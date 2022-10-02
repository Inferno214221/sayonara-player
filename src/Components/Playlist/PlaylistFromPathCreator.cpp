/* PlaylistCreator.cpp */
/*
 * Copyright (C) 2011-2021 Michael Lugmair
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

#include "PlaylistFromPathCreator.h"
#include "ExternTracksPlaylistGenerator.h"

#include "Utils/FileUtils.h"
#include "Utils/Language/Language.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Settings/Settings.h"

#include <QStringList>
#include <atomic>

struct SplittedPaths
{
	QStringList standardPaths;
	QStringList playlistFiles;
};

namespace
{
	SplittedPaths splitPathlist(const QStringList& paths)
	{
		SplittedPaths result;
		for(const auto& path: paths)
		{
			if(Util::File::isSoundFile(path) || Util::File::isDir(path))
			{
				result.standardPaths << path;
			}

			else if(Util::File::isPlaylistFile(path))
			{
				result.playlistFiles << path;
			}
		}

		return result;
	}

	QString getPureFilename(const QString& filename)
	{
		const auto result = Util::File::getFilenameOfPath(filename);
		const auto lastDot = result.lastIndexOf('.');
		return (lastDot > 0)
		       ? result.left(lastDot)
		       : result;
	}
}

struct PlaylistFromPathCreator::Private
{
	PlaylistCreator* playlistCreator;
	std::atomic<int> playlistCount {0};
	int firstIndex {-1};

	explicit Private(PlaylistCreator* playlistCreator) :
		playlistCreator {playlistCreator} {}
};

PlaylistFromPathCreator::PlaylistFromPathCreator(PlaylistCreator* playlistCreator) :
	QObject {}
{
	m = Pimpl::make<Private>(playlistCreator);
}

PlaylistFromPathCreator::~PlaylistFromPathCreator() = default;

int PlaylistFromPathCreator::createSinglePlaylist(const QStringList& paths, const QString& name, bool temporary)
{
	const auto index = m->playlistCreator->createPlaylist(MetaDataList {}, name, temporary);

	auto* playlistGenerator =
		new ExternTracksPlaylistGenerator(m->playlistCreator->playlist(index));

	connect(playlistGenerator,
	        &ExternTracksPlaylistGenerator::sigFinished,
	        this,
	        &PlaylistFromPathCreator::generatorFinished);
	connect(playlistGenerator, &ExternTracksPlaylistGenerator::sigFinished, playlistGenerator, &QObject::deleteLater);

	playlistGenerator->addPaths(paths);

	return index;
}

int PlaylistFromPathCreator::createPlaylists(const QStringList& paths, const QString& name, bool temporary)
{
	const auto splittedPaths = splitPathlist(paths);
	m->playlistCount = splittedPaths.standardPaths.count() + splittedPaths.playlistFiles.count();

	QList<int> createdPlaylists;
	if(!splittedPaths.standardPaths.isEmpty())
	{
		createdPlaylists
			<< createSinglePlaylist(splittedPaths.standardPaths, name, temporary);
	}

	for(const auto& playlistFile: splittedPaths.playlistFiles)
	{
		const auto playlistName = getPureFilename(playlistFile);
		const auto playlistFiles = QStringList() << playlistFile;

		createdPlaylists << createSinglePlaylist(playlistFiles, playlistName, true);
	}

	m->firstIndex = createdPlaylists.isEmpty()
	                ? -1
	                : createdPlaylists[0];

	return m->firstIndex;
}

void PlaylistFromPathCreator::generatorFinished()
{
	m->playlistCount--;
	if(m->playlistCount == 0)
	{
		emit sigAllPlaylistsCreated(m->firstIndex);
	}
}

QString Playlist::filesystemPlaylistName()
{
	const auto createExtraPlaylist = GetSetting(Set::PL_CreateFilesystemPlaylist);
	if(!createExtraPlaylist)
	{
		return {};
	}

	const auto specifyPlaylistName = GetSetting(Set::PL_SpecifyFileystemPlaylistName);
	const auto specialPLaylistName = GetSetting(Set::PL_FilesystemPlaylistName);
	return (specialPLaylistName.trimmed().isEmpty() || specifyPlaylistName)
	       ? Lang::get(Lang::Files)
	       : specialPLaylistName;
}