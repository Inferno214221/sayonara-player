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
#ifndef SAYONARA_PLAYER_TAGWRITER_H
#define SAYONARA_PLAYER_TAGWRITER_H

#include <memory>

class MetaData;
class QString;
class QPixmap;

namespace Tagging
{
	class TagWriter
	{
		public:
			virtual ~TagWriter() = default;

			virtual bool writeMetaData(const QString& filepath, const MetaData& track) = 0;

			virtual bool writeChangedMetaDataOnly(const MetaData& oldTrack, const MetaData& newTrack) = 0;

			virtual bool updateMetaData(const MetaData& track) = 0;

			virtual bool writeCover(const QString& filepath, const QPixmap& cover) = 0;

			virtual bool writeLyrics(const QString& filepath, const QString& lyricsData) = 0;

			static std::shared_ptr<TagWriter> create();
	};

	using TagWriterPtr = std::shared_ptr<TagWriter>;
}

#endif //SAYONARA_PLAYER_TAGWRITER_H
