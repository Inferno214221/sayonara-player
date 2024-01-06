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

#ifndef SAYONARA_PLAYER_TAGGINGMOCKS_H
#define SAYONARA_PLAYER_TAGGINGMOCKS_H

#include "Utils/Tagging/TagReader.h"
#include "Utils/Tagging/TagWriter.h"

namespace Test
{
	class TagReaderMock :
		public Tagging::TagReader
	{
		public:
			~TagReaderMock() override;
			std::optional<MetaData> readMetadata(const QString& filepath) override;

			[[nodiscard]] int count() const { return m_count; }

			[[nodiscard]] bool isCoverSupported(const QString& filepath) const override;
			bool isLyricsSupported(const QString& filepath) const override;
			std::optional<QString> extractLyrics(const QString& filepath) const override;

		private:
			int m_count {0};
	};

	class TagWriterMock :
		public Tagging::TagWriter
	{
		public:
			~TagWriterMock() override;

			bool writeMetaData(const QString& filepath, const MetaData& track) override;
			bool updateMetaData(const MetaData& track) override;
			bool writeCover(const QString& filepath, const QPixmap& cover) override;
			bool writeLyrics(const QString& filepath, const QString& lyricsData) override;
	};
}

#endif //SAYONARA_PLAYER_TAGGINGMOCKS_H
