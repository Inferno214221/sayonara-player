/* LibraryImporter.h */

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

#ifndef LIBRARYIMPORTER_H
#define LIBRARYIMPORTER_H

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
			NoValidTracks,
			CachingFinished,
			Importing,
			Imported
		};

	signals:
		void sigMetadataCached(const MetaDataList& tracks);
		void sigStatusChanged(Importer::ImportStatus);
		void sigProgress(int percent);
		void sigCachedFilesChanged();
		void sigTargetDirectoryChanged(const QString& targetDir);
		void sigTriggered();

	public:
		bool isRunning() const;
		void importFiles(const QStringList& files, const QString& targetDir);
		void acceptImport(const QString& targetDir);
		bool cancelImport();
		void reset();
		int cachedFileCount() const;

		Importer::ImportStatus status() const;

	private slots:
		void cachingThreadFinished();
		void copyThreadFinished();
		void emitStatus(Importer::ImportStatus status);
		void metadataChanged();
	};
}

#endif // LIBRARYIMPORTER_H
