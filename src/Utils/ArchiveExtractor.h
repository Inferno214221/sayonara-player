/* ArchiveExtractor.h */
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

#ifndef SAYONARA_PLAYER_ARCHIVEEXTRACTOR_H
#define SAYONARA_PLAYER_ARCHIVEEXTRACTOR_H

#include <memory>

class QString;
class QStringList;
namespace Util
{
	class ArchiveExtractor
	{
		public:
			ArchiveExtractor();
			virtual ~ArchiveExtractor() noexcept;

			ArchiveExtractor(const ArchiveExtractor& other) = delete;
			ArchiveExtractor(ArchiveExtractor&& other) = delete;
			ArchiveExtractor& operator=(const ArchiveExtractor& other) = delete;
			ArchiveExtractor& operator=(ArchiveExtractor&& other) = delete;

			virtual QStringList extractArchive(const QString& filename, const QString& targetDir) = 0;
			[[nodiscard]] virtual QStringList supportedArchives() const = 0;
			[[nodiscard]] bool isSupportedArchive(const QString& filename) const;

			static std::shared_ptr<ArchiveExtractor> create();
	};

	using ArchiveExtractorPtr = std::shared_ptr<ArchiveExtractor>;
}

#endif //SAYONARA_PLAYER_ARCHIVEEXTRACTOR_H
