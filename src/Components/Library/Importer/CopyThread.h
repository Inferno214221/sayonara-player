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

#include "ImportCache.h"
#include "Utils/Pimpl.h"

namespace Library
{
	class ImportCache;

	/**
	 * @brief The CopyThread class
	 * @ingroup Library
	 */
	class CopyThread :
			public QThread
	{
		Q_OBJECT
		PIMPL(CopyThread)

		signals:
			void sigProgress(int);

		public:
			enum class Mode : uint8_t
			{
				Copy=0,
				Rollback
			};

			CopyThread(const QString& targetDirectory, ImportCachePtr cache, QObject* parent=nullptr);
			virtual ~CopyThread();

			void cancel();
			bool wasCancelled() const;

			MetaDataList copiedMetadata() const;
			int copiedFileCount() const;

			void setMode(CopyThread::Mode mode);

		private:
			void clear();
			void run();

			/**
			 * @brief Copies tracks to file system.
			 * Example: i want to import /home/user/dir\n
			 * my music library is in /home/user/Music\n
			 * i will type "chosen" into entry field\n
			 * i expect a directory /home/user/Music/chosen/dir in my music library
			 */
			void copy();
			void rollback();
			void emitPercent();
	};
}

#endif
