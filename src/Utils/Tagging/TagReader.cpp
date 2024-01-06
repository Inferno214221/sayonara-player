/* TagReader.cpp */
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

#include "TagReader.h"

#include "Utils/MetaData/MetaData.h"
#include "Utils/Tagging/Tagging.h"
#include "Utils/Tagging/TaggingCover.h"
#include "Utils/Tagging/TaggingLyrics.h"

#include <QString>

namespace
{
	class TagReaderImpl :
		public Tagging::TagReader
	{
		public:
			~TagReaderImpl() override = default;

			std::optional<MetaData> readMetadata(const QString& filepath) override
			{
				auto track = MetaData {filepath};
				const auto success = Tagging::Utils::getMetaDataOfFile(track, Tagging::Quality::Quality);
				return success ? std::optional {track} : std::nullopt;
			}

			[[nodiscard]] bool isCoverSupported(const QString& filepath) const override
			{
				return Tagging::isCoverSupported(filepath);
			}

			[[nodiscard]] std::optional<QString> extractLyrics(const QString& filepath) const override
			{
				auto lyricsData = QString {};
				const auto success = Tagging::extractLyrics(filepath, lyricsData);
				return success
				       ? std::optional {lyricsData}
				       : std::nullopt;
			}

			[[nodiscard]] bool isLyricsSupported(const QString& filepath) const override
			{
				return Tagging::isLyricsSupported(filepath);
			}
	};
}

namespace Tagging
{
	TagReaderPtr TagReader::create()
	{
		return std::make_shared<TagReaderImpl>();
	}

} // Tagging
