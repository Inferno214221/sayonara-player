/* ReloadThread.h */

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


/*
 * ReloadThread.h
 *
 *  Created on: Jun 19, 2011
 *      Author: Michael Lugmair (Lucio Carreras)
 */

#ifndef RELOADTHREAD_H_
#define RELOADTHREAD_H_

#include "Utils/Library/LibraryNamespaces.h"
#include "Utils/Pimpl.h"

#include <QThread>
#include <QHash>

class QDir;
class MetaData;
namespace DB
{
	class LibraryDatabase;
}

namespace Tagging
{
	class TagReader;
}

namespace Library
{
	class ReloadThreadFileScanner;
	class ReloadThread :
		public QThread
	{
		Q_OBJECT
		PIMPL(ReloadThread)

		signals:
			void sigReloadingLibrary(const QString& message, int progress);
			void sigNewBlockSaved();

		public:
			ReloadThread(ReloadThreadFileScanner* fileScanner, const std::shared_ptr<Tagging::TagReader>& tagReader,
			             QObject* parent);
			~ReloadThread() override;

			void stop();
			void setQuality(ReloadQuality quality);
			void setLibrary(LibraryId id, const QString& libraryPath);

		protected:
			void run() override;

		private:
			void cleanupLibrary(const MetaDataList& orphanedTracks, DB::LibraryDatabase* libraryDatabase);
			bool reloadLibrary(const QHash<QString, MetaData>& pathTrackMap, DB::LibraryDatabase* libraryDatabase);
	};
}

#endif /* RELOADTHREAD_H_ */
