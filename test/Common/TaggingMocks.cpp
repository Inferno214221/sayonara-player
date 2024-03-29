/* ${CLASS_NAME}.h */
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

#include "TaggingMocks.h"

#include "Utils/MetaData/MetaData.h"
#include "Utils/FileUtils.h"

#include <QString>

namespace Test
{
	TagReaderMock::~TagReaderMock() = default;

	std::optional<MetaData> TagReaderMock::readMetadata(const QString& filepath)
	{
		if(Util::File::isSoundFile(filepath))
		{
			m_count++;

			auto track = MetaData {filepath};
			track.setTitle(filepath);
			track.setAlbum(filepath);
			track.setArtist(filepath);

			return {track};
		}

		return std::nullopt;
	}

	bool TagReaderMock::isCoverSupported(const QString& /*filepath*/) const { return false; }

	bool TagReaderMock::isLyricsSupported(const QString& /*filepath*/) const { return false; }

	std::optional<QString> TagReaderMock::extractLyrics(const QString& /*filepath*/) const { return std::nullopt; }

	TagWriterMock::~TagWriterMock() = default;

	bool TagWriterMock::writeMetaData(const QString& filepath, const MetaData& /*track*/)
	{
		return !filepath.isEmpty();
	}

	bool TagWriterMock::writeChangedMetaDataOnly(const MetaData& /*oldTrack*/, const MetaData& newTrack)
	{
		return !newTrack.filepath().isEmpty();
	}

	bool TagWriterMock::updateMetaData(const MetaData& track)
	{
		return !track.filepath().isEmpty();
	}

	bool TagWriterMock::writeCover(const QString& /*filepath*/, const QPixmap& /*cover*/)
	{
		return false;
	}

	bool TagWriterMock::writeLyrics(const QString& /*filepath*/, const QString& /*lyricsData*/)
	{
		return false;
	}
}
