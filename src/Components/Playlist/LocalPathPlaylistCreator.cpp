/* PlaylistCreator.cpp */
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

#include "LocalPathPlaylistCreator.h"
#include "LocalPathProcessor.h"

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

namespace Playlist
{
	class LocalPathPlaylistCreatorImpl :
		public LocalPathPlaylistCreator
	{
		Q_OBJECT

		public:
			explicit LocalPathPlaylistCreatorImpl(PlaylistCreator* playlistCreator) :
				m_playlistCreator {playlistCreator} {}

			~LocalPathPlaylistCreatorImpl() override = default;

			int createPlaylists(const QStringList& paths, const QString& name, bool temporary) override
			{
				const auto splittedPaths = splitPathlist(paths);
				m_playlistCount = splittedPaths.standardPaths.count() + splittedPaths.playlistFiles.count();

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

				m_firstIndex = createdPlaylists.isEmpty()
				               ? -1
				               : createdPlaylists[0];

				return m_firstIndex;
			}

		private: // NOLINT(readability-redundant-access-specifiers)
			int createSinglePlaylist(const QStringList& paths, const QString& name, const bool temporary)
			{
				const auto index = m_playlistCreator->createPlaylist(MetaDataList {}, name, temporary);

				auto* playlistGenerator =
					new LocalPathProcessor(m_playlistCreator->playlist(index));

				connect(playlistGenerator, &LocalPathProcessor::sigFinished, this, [&]() {
					if((--m_playlistCount) == 0)
					{
						emit sigAllPlaylistsCreated(m_firstIndex);
					}
				});

				connect(playlistGenerator, &LocalPathProcessor::sigFinished,
				        playlistGenerator, &QObject::deleteLater);

				playlistGenerator->addPaths(paths);

				return index;
			}

			PlaylistCreator* m_playlistCreator;
			std::atomic<int> m_playlistCount {0};
			int m_firstIndex {-1};

	};

	LocalPathPlaylistCreator* LocalPathPlaylistCreator::create(PlaylistCreator* playlistCreator)
	{
		return new LocalPathPlaylistCreatorImpl(playlistCreator);
	}

	QString filesystemPlaylistName()
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
}

#include "LocalPathPlaylistCreator.moc"