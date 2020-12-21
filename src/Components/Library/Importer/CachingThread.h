/* ImportFolderThread.h */

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

#ifndef IMPORTFOLDERTHREAD_H
#define IMPORTFOLDERTHREAD_H

#include <QThread>

#include "ImportCache.h"
#include "Utils/Pimpl.h"

namespace Library
{
	class ImportCache;
	/**
	 * @brief The CachingThread class
	 * @ingroup Library
	 */
	class CachingThread :
			public QThread
	{
		Q_OBJECT
		PIMPL(CachingThread)

		signals:
			void			sigCachedFilesChanged();

		public:
			explicit CachingThread(const QStringList& fileList, const QString& libraryPath, QObject* parent=nullptr);
			~CachingThread() override;

			Library::ImportCachePtr	cache() const;
			void			cancel();
			bool			isCancelled() const;
			QStringList		temporaryFiles() const;
			int				cachedFileCount() const;
			int				soundfileCount() const;


		private:
			void run() override;

			void scanDirectory(const QString& dir);
			bool scanRarArchive(const QString& rarFile);
			bool scanZipArchive(const QString& zipFile);
			bool scanTgzArchive(const QString& tgz);
			void addFile(const QString& filename, const QString& relativeDir=QString());

			QString createTempDirectory();
			bool scanArchive
			(
				const QString& tempDirectory,
				const QString& binary,
				const QStringList& args,
				const QList<int>& successCodes=QList<int>{0}
			);

		private slots:
			void metadataChanged();
	};
}

#endif // IMPORTFOLDERTHREAD_H
