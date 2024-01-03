/* LibraryImporter.h */

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

#ifndef LIBRARYIMPORTER_H
#define LIBRARYIMPORTER_H

#include "Utils/Pimpl.h"
#include <QObject>

namespace Util
{
	class FileSystem;
}

namespace Tagging
{
	class TagReader;
}

namespace DB
{
	class LibraryDatabase;
}

class MetaDataList;
namespace Library
{
	class Info;
	class Importer :
		public QObject
	{
		Q_OBJECT
		PIMPL(Importer)

		public:
			Importer(DB::LibraryDatabase* libraryDatabase, std::shared_ptr<Util::FileSystem> fileSystem,
			         std::shared_ptr<Tagging::TagReader> tagReader, QObject* parent);
			~Importer() override;

			enum class ImportStatus :
				uint8_t
			{
				Cancelled,
				Rollback,
				Caching,
				NoTracks,
				NoValidTracks,
				CachingFinished,
				Importing,
				Imported
			};

		signals:
			void sigStatusChanged(Library::Importer::ImportStatus status);
			void sigProgress(int percent);
			void sigCachedFilesChanged();
			void sigTargetDirectoryChanged(const QString& targetDir);

		public:
			void import(const QString& libraryPath, const QStringList& files, const QString& targetDir);
			void cancelImport();
			void copy(const QString& targetDir);
			[[nodiscard]] Importer::ImportStatus status() const;
			[[nodiscard]] MetaDataList cachedTracks() const;

			void reset();

		private slots:
			void cachingProcessorFinished();
			void copyProcessorFinished();

		private: // NOLINT(readability-redundant-access-specifiers)
			void startCaching(const QStringList& files, const QString& libraryPath);
			void rollback();
			void emitStatus(Importer::ImportStatus status);
			void storeTracksInLibrary(const MetaDataList& tracks, int copiedFiles);
	};
}

#endif // LIBRARYIMPORTER_H
