/* ReloadThread.h */

/* Copyright (C) 2011-2019 Lucio Carreras
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


/*
 * ReloadThread.h
 *
 *  Created on: Jun 19, 2011
 *      Author: Lucio Carreras
 */

#ifndef RELOADTHREAD_H_
#define RELOADTHREAD_H_

#include "Utils/Library/LibraryNamespaces.h"
#include "Utils/Pimpl.h"

#include <QThread>
#include <QDir>

namespace Library
{
	class ReloadThread :
			public QThread
	{
		Q_OBJECT
		PIMPL(ReloadThread)

	signals:
		void sig_reloading_library(const QString& message, int progress);
		void sig_new_block_saved();

	public:
		explicit ReloadThread(QObject* parent);
		~ReloadThread();

		void pause();
		void goon();
		void stop();
		bool is_running() const;
		void set_quality(ReloadQuality quality);
		void set_library(LibraryId lib_id, const QString& library_path);

	protected:
		virtual void run() override;

	private:
		bool			get_and_save_all_files(const QHash<QString, MetaData>& v_md_map);
		QStringList		get_files_recursive(QDir base_dir);
		QStringList		process_sub_files(const QDir& dir, const QStringList& sub_files);
		void			store_metadata_block(const MetaDataList& v_md);
	};
}

#endif /* RELOADTHREAD_H_ */
