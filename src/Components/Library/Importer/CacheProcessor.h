/* ImportFolderThread.h */

/* Copyright (C) 2011-2024 Michael Lugmair (Lucio Carreras)
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

#ifndef IMPORTFOLDERTHREAD_H
#define IMPORTFOLDERTHREAD_H

#include "ImportCache.h"

#include <QObject>

namespace Util
{
	class ArchiveExtractor;
	class DirectoryReader;
	class FileSystem;
}

namespace Tagging
{
	class TagReader;
}

namespace Library
{
	class ImportCache;
	class CacheProcessor :
		public QObject
	{
		Q_OBJECT

		signals:
			void sigCachedFilesChanged();
			void sigFinished();

		public:
			struct CacheResult
			{
				Library::ImportCachePtr cache {nullptr};
				QStringList temporaryFiles;
			};

			~CacheProcessor() noexcept override;

			static CacheProcessor* create(const QStringList& fileList,
			                              const QString& libraryPath,
			                              const std::shared_ptr<Tagging::TagReader>& tagReader,
			                              const std::shared_ptr<Util::ArchiveExtractor>& archiveExtractor,
			                              const std::shared_ptr<Util::DirectoryReader>& directoryReader,
			                              const std::shared_ptr<Util::FileSystem>& fileSystem);

			[[nodiscard]] virtual CacheResult cacheResult() const = 0;

			virtual void cancel() = 0;
			[[nodiscard]] virtual bool wasCancelled() const = 0;

		public slots: // NOLINT(readability-redundant-access-specifiers)
			virtual void cacheFiles() = 0;

		protected:
			void emitCachedFilesChanged();
	};
}

#endif // IMPORTFOLDERTHREAD_H
