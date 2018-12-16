/* LibraryImporter.h */

/* Copyright (C) 2011-2017  Lucio Carreras
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

#include "Utils/Settings/SayonaraClass.h"
#include "Utils/Pimpl.h"

#include <QObject>

class LocalLibrary;

namespace Library
{
	/**
	 * @brief The LibraryImporter class
	 * @ingroup Library
	 */
	class Importer :
			public QObject
	{
		Q_OBJECT
		PIMPL(Importer)

	public:
		explicit Importer(LocalLibrary* library);
		~Importer();

		enum class ImportStatus : uint8_t
		{
			Cancelled,
			Rollback,
			Caching,
			NoTracks,
			WaitForUser,
			Importing,
			Imported
		};

	signals:
		void sig_got_metadata(const MetaDataList&);
		void sig_status_changed(Importer::ImportStatus);
		void sig_got_library_dirs(const QStringList& library_dirs);
		void sig_progress(int percent);
		void sig_triggered();
		void sig_target_dir_changed(const QString& target_dir);


	public:
		void import_files(const QStringList& files, const QString& target_dir);
		void accept_import(const QString& target_dir);
		void cancel_import();

		Importer::ImportStatus status() const;


	private slots:
		void caching_thread_finished();
		void copy_thread_finished();
		void emit_status(Importer::ImportStatus status);
		void metadata_changed(const MetaDataList& old_md, const MetaDataList& new_md);

	};
}

#endif // LIBRARYIMPORTER_H
