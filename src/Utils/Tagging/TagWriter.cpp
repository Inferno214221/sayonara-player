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

#include "TagWriter.h"
#include "Tagging.h"
#include "TaggingCover.h"
#include "TaggingLyrics.h"

#include "Utils/MetaData/MetaData.h"

namespace
{
	class TagWriterImpl :
		public Tagging::TagWriter
	{
		public:
			~TagWriterImpl() override = default;

			bool writeMetaData(const QString& filepath, const MetaData& track) override
			{
				auto trackCopy = track;
				trackCopy.setFilepath(filepath);

				return updateMetaData(trackCopy);
			}

			bool updateMetaData(const MetaData& track) override
			{
				return Tagging::Utils::setMetaDataOfFile(track);
			}

			bool writeCover(const QString& filepath, const QPixmap& cover) override
			{
				return Tagging::writeCover(filepath, cover);
			}

			bool writeLyrics(const MetaData& track, const QString& lyricsData) override
			{
				return Tagging::writeLyrics(track, lyricsData);
			}

	};
}

namespace Tagging
{
	TagWriterPtr TagWriter::create()
	{
		return std::make_shared<TagWriterImpl>();
	}
} // Tagging
