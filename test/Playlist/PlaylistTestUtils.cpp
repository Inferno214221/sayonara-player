/* PlaylistTestUtils.cpp
 *
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

#include "PlaylistTestUtils.h"

#include "test/Common/TestTracks.h"

#include "Utils/MetaData/MetaDataList.h"
#include "Utils/FileUtils.h"

#include <QString>

namespace
{
	void createFileStructure(const QString& basePath)
	{
		auto success = Util::File::createDirectories(QString("%1/path/to/somewhere/else").arg(basePath));
		success &= Util::File::createDirectories(QString("%1/path/to/another/dir").arg(basePath));

		if(!success)
		{
			throw std::runtime_error {"Could not create directories"};
		}
	}
}

namespace Test::Playlist
{
	MetaDataList createTrackList(int min, int max)
	{
		MetaDataList tracks;
		for(int i = min; i < max; i++)
		{
			MetaData track;
			const auto p = QString("https://www.bla.com/path/to/%1.mp3").arg(i);

			track.setId(i);
			track.setFilepath(p);
			track.setDurationMs(i * 10000);

			tracks << track;
		}

		return tracks;
	}

	PathTrackMap createTrackFiles(const QString& basePath)
	{
		PathTrackMap result;

		createFileStructure(basePath);

		QStringList names;
		names << QString("%1/path/mp3test.mp3").arg(basePath);
		names << QString("%1/path/to/mp3test.mp3").arg(basePath);
		names << QString("%1/path/to/somewhere/mp3test.mp3").arg(basePath);
		names << QString("%1/path/to/somewhere/else/mp3test.mp3").arg(basePath);
		names << QString("%1/path/to/another/mp3test.mp3").arg(basePath);
		names << QString("%1/path/to/another/dir/mp3test.mp3").arg(basePath);

		const auto source = ":/test/mp3test.mp3";

		for(int i = 0; i < names.size(); i++)
		{
			auto track = Test::createTrack(i,
			                               QString("Title%1").arg(i + 1),
			                               QString("Artist%1").arg(i + 1),
			                               QString("Album%1").arg(i + 1));
			const auto& filepath = names[i];

			track.setFilepath(filepath);
			track.setDurationMs((i + 1) * 100'000);

			auto[dir, filename] = Util::File::splitFilename(filepath);
			Util::File::copyFile(source, dir, filename);

			result << std::make_pair(names[i], track);
		}

		return result;
	}
}