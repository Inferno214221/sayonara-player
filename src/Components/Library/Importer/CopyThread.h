/* CopyThread.h */

/* Copyright (C) 2011-2020 Michael Lugmair (Lucio Carreras)
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

#include "Utils/Pimpl.h"

namespace Util
{
	class FileSystem;
}

namespace Library
{
	class ImportCache;
	class CopyThread :
		public QThread
	{
		Q_OBJECT
		PIMPL(CopyThread)

		signals:
			void sigProgress(int progress);

		public:
			enum class Mode :
				uint8_t
			{
				Copy = 0,
				Rollback
			};

			CopyThread(const QString& targetDirectory,
			           const std::shared_ptr<ImportCache>& cache,
			           const std::shared_ptr<Util::FileSystem>& fileSystem,
			           QObject* parent = nullptr);

			virtual ~CopyThread();

			void cancel();
			bool wasCancelled() const;

			MetaDataList copiedMetadata() const;
			int copiedFileCount() const;

			void setMode(CopyThread::Mode mode);

		protected:
			void run() override;

		private:
			void copy();
			void rollback();
			void emitPercent();
	};
}

#endif
