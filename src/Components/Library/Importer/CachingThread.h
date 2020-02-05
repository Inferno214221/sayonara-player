/* ImportFolderThread.h */

/* Copyright (C) 2011-2020  Lucio Carreras
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
			void			sig_progress(int);

		public:
			explicit CachingThread(const QStringList& file_list, const QString& library_path, QObject *parent=nullptr);
			~CachingThread() override;

			Library::ImportCachePtr	cache() const;
			void			cancel();
			bool			is_cancelled() const;
			QStringList		temporary_files() const;

		private:
			void run() override;

			void update_progress();
			void scan_dir(const QString& dir);
			bool scan_rar(const QString& rar);
			bool scan_zip(const QString& zip);
			bool scan_tgz(const QString& tgz);
			void add_file(const QString& filename, const QString& relative_dir=QString());


			QString create_temp_dir();
			bool scan_archive
			(
				const QString& temp_dir,
				const QString& binary,
				const QStringList& args,
				const QList<int>& success_codes=QList<int>{0}
			);

		private slots:
			void metadata_changed();
	};
}

#endif // IMPORTFOLDERTHREAD_H
