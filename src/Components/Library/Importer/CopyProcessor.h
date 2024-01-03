/* CopyThread.h */

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

#ifndef IMPORT_COPY_THREAD_H
#define IMPORT_COPY_THREAD_H

#include <QThread>

#include <memory>

namespace Util
{
	class FileSystem;
}

class MetaDataList;
namespace Library
{
	class ImportCache;
	class CopyProcessor :
		public QObject
	{
		Q_OBJECT

		signals:
			void sigProgress(int progress);
			void sigFinished();

		public:
			static CopyProcessor* create(const QString& targetDirectory,
			                             const std::shared_ptr<ImportCache>& cache,
			                             const std::shared_ptr<Util::FileSystem>& fileSystem);

			virtual void copy() = 0;
			virtual void cancel() = 0;
			virtual void rollback() = 0;
			[[nodiscard]] virtual bool wasCancelled() const = 0;
			[[nodiscard]] virtual MetaDataList copiedMetadata() const = 0;
			[[nodiscard]] virtual int copiedFileCount() const = 0;

		protected:
			void emitProgress(int progress);
	};
}

#endif
